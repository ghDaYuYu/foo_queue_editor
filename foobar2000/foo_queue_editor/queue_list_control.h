#pragma once

#include <algorithm>
#include "pfc/array.h"
#include "libPPUI/CFlashWindow.h"
#include "libPPUI/CListControlOwnerData.h"
#include "helpers/CListControlFb2kColors.h"

#include "queue_titleformat_hook.h" 
#include "queue_helpers.h"

#include "ui_element_host.h"
#include "config.h"

#include "queue_list_control_ILO.h"

#define AUTO_FLAG UINT32_MAX

namespace dlg {

	//contains information about the state of the active playlist at the time of drop
	struct ActivePlaylistInformation {
		// pfc::infinite_size if the drop is the drop is not coming from active playlist
		GUID src = pfc::guid_null;
		t_size active_playlist_index = SIZE_MAX;
		t_size active_playlist_item_count = 0;
		pfc::bit_array_bittable active_playlist_selection_mask;
		pfc::list_t<metadb_handle_ptr> active_playlist_selected_items;
		pfc::list_t<metadb_handle_ptr> active_playlist_all_items;
		std::vector<GUID> ibom_selection_guids;

		void Reset() {
			src = pfc::guid_null;
			active_playlist_index = SIZE_MAX;
			active_playlist_item_count = 0;
			active_playlist_selection_mask.resize(0);
			active_playlist_selected_items.remove_all();
			active_playlist_all_items.remove_all();
			ibom_selection_guids.clear();
		}
	};

	typedef CListControlFb2kColors <CListControlOwnerData> CListControlOwnerColors;

	class CListControlQueue : public CListControlOwnerColors {

	public:

		CListControlQueue(ILOD_QueueSource* h, bool is_cui) : CListControlOwnerColors(h),
			m_cui(is_cui), m_hook(), m_ui_host(nullptr) {

			SetFlatStyle();
			//..

		}

		~CListControlQueue() {
			CListControlFb2kColors::Detach();
			m_flash.CleanUp();
		}

		typedef CListControlOwnerColors TParent;
		BEGIN_MSG_MAP_EX(CListControlQueue)
			MSG_WM_SETFOCUS(OnSetFocus)
			MSG_WM_KILLFOCUS(OnKillFocus)
			if (uMsg == WM_CONTEXTMENU) {
				if (m_callback.is_valid() && m_callback->is_edit_mode_enabled()) {
					SetMsgHandled(TRUE);
					lResult = ::DefWindowProc(hWnd, uMsg, wParam, lParam);

					if (IsMsgHandled()) {
						return TRUE;
					}
				}
			}
			MSG_WM_CONTEXTMENU(OnContextMenu)
			MSG_WM_DESTROY(OnDestroy)
			CHAIN_MSG_MAP(TParent)
		END_MSG_MAP()

		void SetLastDDMark() {
			m_lastDDMark = GetDropMark();
		}

		//todo: bookmark copy/paste
		void SetLastDDMarkIbom() {
			m_lastDDMark = GetItemCount();
		}

		void ClearLastDrop() {
			m_mhl_dropped.remove_all();
			m_lastDDMark = SIZE_MAX;
		}

		void SetDropNofifyPlaylistStated(ActivePlaylistInformation drop_notify_playlist_state) {
			m_dron_plst = drop_notify_playlist_state;
		}

		ActivePlaylistInformation m_dron_plst = {};

		// drop notify async fx

		//todo: rev/remove
		size_t get_next_dup_pos(metadb_handle_ptr p_handle, size_t pl_ndx, size_t trk_pos, size_t dup_delta) {

			size_t next_dup = SIZE_MAX;

			static_api_ptr_t<playlist_manager_v5> playlist_api;
			metadb_handle_list mhl;
			playlist_api->playlist_get_all_items(pl_ndx, mhl);
			for (auto i = trk_pos + 1; i < mhl.get_count(); i++) {
				if (!stricmp_utf8(mhl.get_item(i)->get_path(), p_handle->get_path())) {
					dup_delta--;
					if (!dup_delta) {
						next_dup = i;
						break;
					}
				}
			}

			return next_dup;
		}

		process_locations_notify::func_t m_drop_notify_async_fx = [&](metadb_handle_list_cref mhl_drop) {

			m_mhl_dropped = mhl_drop;

			bool from_playlist = pfc::guid_equal(m_dron_plst.src, contextmenu_item::caller_active_playlist);
			from_playlist |= pfc::guid_equal(m_dron_plst.src, contextmenu_item::caller_active_playlist_selection);

			static_api_ptr_t<playlist_manager_v5> playlist_api;

			bool bBookmarkDeco = m_dron_plst.ibom_selection_guids.size();

			std::vector <inplay_t> vmixed;
			vmixed.resize(mhl_drop.get_count());

			if (!from_playlist) {

				size_t in_library = false;
				size_t in_active_playlist = false;

				bit_array_bittable depri_act_playlist_mask(bit_array_false(), m_dron_plst.active_playlist_item_count);

				// prepare vmixed
				// dropping bookmarks makes this lower than is usual
				// route playlist/handle queueing (inc. bookmarks)  

				GUID w_ibom_GUID = pfc::guid_null;
				size_t w_handle = 0;
				for (auto p_handle : mhl_drop) {

					w_ibom_GUID = bBookmarkDeco ? m_dron_plst.ibom_selection_guids[w_handle] : w_ibom_GUID;
					size_t pl_track_pos;

					size_t guid_pl_ndx;

					//todo: rev/remove
					//wrong track pos with duplicates
					bool pl_owned = playlist_api->activeplaylist_find_item(p_handle, pl_track_pos);
					if (!pfc::guid_equal(pfc::guid_null, w_ibom_GUID)) {
						size_t guid_pl_trk_pos;
						guid_pl_ndx = playlist_api->find_playlist_by_guid(w_ibom_GUID);
						bool guid_pl_ok = playlist_api->playlist_find_item(guid_pl_ndx, p_handle, guid_pl_trk_pos);
						pl_owned = guid_pl_ok;
						pl_track_pos = guid_pl_trk_pos;
					}

					if (pl_owned) {

						size_t pl_ndx;
						if (!pfc::guid_equal(pfc::guid_null, w_ibom_GUID)) {
							pl_ndx = guid_pl_ndx;

						}
						else {
							pl_ndx = playlist_api->get_active_playlist();
						}

						//orphans 'send to queue', active pl,...
						//running !from_playlist exec path (do not use m_dron_plst.active_playlist_index)

						in_active_playlist++;

						std::map<size_t, size_t>map_dup;

						auto found_dup_it = std::find_if(vmixed.begin(), vmixed.begin()+ w_handle, [=](const inplay_t & rec) {
							return rec.trk_pos == pl_track_pos && rec.pl_ndx == pl_ndx;
							});

						inplay_t & mixed_rec = vmixed[w_handle];

						mixed_rec.trk_pos = pl_track_pos;
						mixed_rec.pl_ndx = pl_ndx;
						mixed_rec.pl_size = playlist_api->activeplaylist_get_item_count();

						if (found_dup_it != vmixed.begin() + w_handle /*range*/) {

							auto dbg_count_this_dups = std::count_if(vmixed.begin(), vmixed.begin() + w_handle, [=](const inplay_t rec) {
								return rec.trk_pos == mixed_rec.trk_pos;
								});

							auto this_dup_ndx = get_next_dup_pos(p_handle, mixed_rec.pl_ndx,
								found_dup_it->trk_pos, dbg_count_this_dups);

							mixed_rec.trk_dup = found_dup_it->trk_pos;
							//todo: rev/remove. Misc discontinuous playlist chunks bookmarks dropped... 
							mixed_rec.trk_pos = this_dup_ndx == SIZE_MAX ? found_dup_it->trk_pos : this_dup_ndx;
						}
						depri_act_playlist_mask.set(pl_track_pos, true);
					}
					else {
						in_library++;
					}
					++w_handle;
				}

				// mix ready.

				// queue handles/playlist items

				pfc::list_t<t_playback_queue_item> queue;
				playlist_api->queue_get_contents(queue);

				t_size cadd = 0;

				for (auto it = vmixed.begin(); it != vmixed.end(); it++) {

					// assign to playlist?
					if (it->pl_ndx == SIZE_MAX || it->trk_pos == SIZE_MAX) {

						// from lib (playlist ~0 or track not in this playlist)

						// advance in vmixed while from lib
						metadb_handle_list mhl_chunk;
						for (auto& it_i = it; it_i != vmixed.end() /*&& it->second.first == SIZE_MAX*/; it_i++) {

							if (it->pl_ndx != SIZE_MAX && it->trk_pos != SIZE_MAX) {
								//chunk ends, step back iterator and break
								it_i--;
								break;
							}

							size_t ndx = std::distance(vmixed.begin(), it_i);
							mhl_chunk.add_item(mhl_drop[ndx]);
						}

						//queue !in_playlist
						if (mhl_chunk.get_count()) {

							if (m_lastDDMark + cadd < queue.get_count()) {
								queue_helpers::queue_insert_items(m_lastDDMark + cadd, mhl_chunk);
							}
							else {
								queue_helpers::queue_add_items(mhl_chunk, true);
							}
							cadd += mhl_chunk.get_count();
							mhl_chunk.remove_all();
						}

						if (it == vmixed.end()) {
							//done
							break;
						}
					}
					else {

						//mixed owned by /*active*/ playlist

						metadb_handle_list mhl_chunk;
						std::vector <inplay_t*> vmixed_chunk_ref;
						size_t dbg_pl_first_pos = it->trk_pos;
						size_t pl_index = it->pl_ndx;
						size_t pl_count = it->pl_size;

						// prepare playlist mask for this chunk
						bit_array_bittable pl_mask(bit_array_false(), pl_count);

						const auto first_it_i = it;
						size_t dups_in_chunk = 0;
						size_t dbg_w = 0;
						for (auto& it_i = it; it_i != vmixed.end(); it_i++) {
							auto dbg_first = first_it_i->trk_pos;
							auto dbg_i = it_i->trk_pos;
							dbg_w++;
							if ((it->pl_ndx == SIZE_MAX || it->trk_pos == SIZE_MAX) || it->pl_ndx != first_it_i->pl_ndx) {
								//chunk ends, step back iterator and break
								it_i--;
								break;
							}

							//todo: rev/remove
							bool dupe = it_i->trk_dup != SIZE_MAX;
							if (dupe) {
								dups_in_chunk++;
							}

							size_t drop_ndx = std::distance(vmixed.begin(), it_i);
							mhl_chunk.add_item(mhl_drop[drop_ndx]);
							auto dbg_path = mhl_drop[drop_ndx]->get_path();
							vmixed_chunk_ref.emplace_back(&*it_i);
							
							//todo: rev/remove
							if (dupe) {
								auto dbg_count_this_dups = std::count_if(first_it_i, it_i, [=](const inplay_t rec) {
									return rec.trk_pos == it_i->trk_pos;
									}
								);

								//distance from first occurrence in chunk
								auto fo_it = std::find_if(first_it_i, it_i, [=](const inplay_t rec) {
									return rec.trk_pos == it_i->trk_pos;
									});
								auto delta = std::distance(fo_it, it_i);
								pl_mask.set(it_i->trk_pos + delta/*count_dups*/, true);

							}
							else {
								pl_mask.set(it_i->trk_pos, true);
							}
						}

						// Queue the playlist items by pos in the playlist

						if (mhl_chunk.get_count()) {

							queue_helpers::queue_insert_items(m_lastDDMark + cadd,
								pl_index, vmixed_chunk_ref, pl_count);

							cadd += mhl_chunk.get_count();
							mhl_chunk.remove_all();
						}

						if (it == vmixed.end()) {
							//done
							break;
						}
					}
				} //end for
			}
			else {
				queue_helpers::queue_insert_items(m_lastDDMark,
					playlist_api->get_active_playlist(),
					m_dron_plst.active_playlist_selection_mask,
					playlist_api->activeplaylist_get_item_count());

				vmixed.clear();
				m_dron_plst.Reset();
				ClearLastDrop();
			}

			return;

			if (m_lastDDMark == SIZE_MAX) {
				//drop over empty list?
				m_lastDDMark = 0;
			}

			if (m_lastDDMark != SIZE_MAX) {

				pfc::list_t<t_playback_queue_item> queue;

				playlist_api->queue_get_contents(queue);

				if (m_dron_plst.active_playlist_index != SIZE_MAX) {

					DEBUG_PRINT << "Queueing playlist items";
					// Queue playlist items by pos in the playlist
					queue_helpers::queue_insert_items(m_lastDDMark,
							m_dron_plst.active_playlist_index,
							m_dron_plst.active_playlist_selection_mask,
						m_dron_plst.active_playlist_item_count);

					m_dron_plst.active_playlist_selected_items.remove_all();
					m_dron_plst.active_playlist_index = SIZE_MAX;
				}
				else {
				
					if (m_lastDDMark < queue.get_count()) {
						queue_helpers::queue_insert_items(m_lastDDMark, m_mhl_dropped);
					}
					else {
						queue_helpers::queue_add_items(m_mhl_dropped, true);
					}
				}
			}
			ClearLastDrop();
		};

		void SetHost(ui_element_host* host) { m_ui_host = host; }
		ui_element_host* GetHost() { return m_ui_host; };
		//todo: revise shared selections
		void SetSharedSelection(metadb_handle_list_cref p_items);
		void SetSharedSelection();

		void CommandContextMenu(CPoint screen_point, unsigned cmd);

		bool cmdMenuCommon(unsigned cmd);
		bool cmdMenuHeader(unsigned base_cmd);
		bool cmdMenuListItems(unsigned base_cmd);
		bool cmdMenuNoItems(unsigned base_cmd);
		bool cmdMenuFrameStyle(unsigned base_cmd);
		bool cmdMenuPropPref(unsigned base_cmd);

		void BuildContextMenu(CPoint screen_point, CMenuHandle menu);

		bool bldMenuHeader(CMenuHandle menu);
		bool bldMenuListItems(CMenuHandle menu);
		bool bldMenuNoItems(CMenuHandle menu);
		bool bldMenuFrameStyle(CMenuHandle menu);
		bool bldMenuPrefProp(CMenuHandle menu, bool header_hit);

		void build_sub_menu_save_restore(HMENU parent_menu, UINT submenu_id, TCHAR* wmenutitle);
		void build_sub_menu_columns(HMENU parent_menu, UINT submenu_id, TCHAR* wmenutitle);

		void DeleteSelected();
		void MoveSelected(const size_t* order, size_t count);
		void SetSelectedIndices(const pfc::list_base_const_t<t_size>& p_in);

		void QueueRefresh();
		void QueueReset() { my_queue.remove_all(); }

		void SetCallback(ui_element_instance_callback_ptr p_callback) { m_callback = p_callback; }

		// list control data
		// Add queue list data, called when adding a new column
		void Add_PQI_List(pfc::list_t<t_playback_queue_item> lpqi);
		// Add a queue list item
		void Add_PQI_Item(t_playback_queue_item pqi, t_size qpos);
		// Add field column item
		void Add_PQI_List_Column(pfc::list_t<t_playback_queue_item> lpqi, t_size col_ndx);
		// Add field value to subItems (single)
		void Add_PQI_SubItem(t_playback_queue_item pqi, t_size qpos, t_size col_ndx);

		void OnDestroy() {

			m_shared_selection.release();
			SetMsgHandled(FALSE);
		}

		void OnKillFocus(CWindow wndFocus);
		void OnSetFocus(CWindow wndOld);

		void GetMetaDbHandles(metadb_handle_list_ref p_out);
		metadb_handle_list_ref GetAllMetadbs() { return m_metadbs; }

		void GetSelectedItemsMetaDbHandles(metadb_handle_list_ref p_out);

		void OnContextMenu(CWindow wnd, CPoint point);

		void ModColumns(pfc::map_t<long, ui_column_definition> old_ui_col_defs) {
			ui_element_settings* settings;
			m_ui_host->get_configuration(&settings);

			bool need_header_rebuilt = false;
			bool need_header_refresh = false;
			bool need_data_requery = false;

			for (auto w : settings->m_columns) {

				if (need_header_rebuilt && need_data_requery && need_header_refresh) {
					break;
				}

				auto was_cf = old_ui_col_defs.find(w.m_id);
				auto is_cf = cfg_ui_columns.find(w.m_id);
				if (was_cf.is_valid()) {
					// was using it
					if (is_cf.is_valid()) {
						//still exists
						if (!(was_cf->m_value.m_name.equals(is_cf->m_value.m_name) &&
							was_cf->m_value.m_alignment == is_cf->m_value.m_alignment)) {
							//header or aligment is different
							need_header_refresh = true;
						}
						//requery?
						need_data_requery |= !was_cf->m_value.m_pattern.equals(is_cf->m_value.m_pattern);
					}
					else {
						//field no longer available
						need_header_rebuilt = true;
					}
				}
				else {
					//ui not using this field_id
				}
			}

			//todo: rev

			if (need_header_rebuilt) {
				BuildColumns(true, false);
			}

			if (need_data_requery) {
				QueueReset();
				QueueRefresh();
			}
			
			if (need_header_refresh) {
				UpdateHeaderNameAlign();
			}
		}

		void ResetColumns() {
			pfc::map_t<long, ui_column_definition> old_ui_col_defs;
			old_ui_col_defs = cfg_ui_columns;
			BuildColumns(false, true);
			QueueReset();
			QueueRefresh();
		}

		void BuildColumns(bool restore, bool defaults = false) {

			DeleteColumns(bit_array_true());

			ui_element_settings* settings;
			m_ui_host->get_configuration(&settings);

			auto DPI = GetDPI();

			if (defaults) {
				//or crash OnPaint -shouldn�t be here-?
				OnColumnsChanged();

				auto reset_cols = 2;
				//auto reset_cols = 1;

				if (cfg_ui_columns.get_count() >= reset_cols) {

					settings->m_columns.remove_all();

					std::vector<size_t> vdef_keys;
					if (reset_cols == 2) {
						vdef_keys = { 1, 0 };
					}
					else {
						vdef_keys = { 1 };
					}

					for (size_t i = 0; i < vdef_keys.size(); i++) {

						auto width = 90;
						bool autoWidth = false;

						if (i == 1 || vdef_keys.size() == 1) {
							width = AUTO_FLAG;
							autoWidth = true;
						}

						auto cs = cfg_ui_columns.find(vdef_keys[i]);
						settings->m_columns.add_item(cs->m_key);
						ui_column_settings &col_def = settings->m_columns[i];
						col_def.m_autoWidth = autoWidth;
						col_def.m_order = i;
						col_def.m_column_width = width;

						AddColumn(cs->m_value.m_name,
							col_def.m_column_width,
							cs->m_value.m_alignment, true);
					}
					m_order = GetColumnOrderArray();
					this->ReloadItems(bit_array_true());
				}
				OnColumnsChanged();
				return;
			}

			RemoveInvalidColCfgs();

			t_size columnCount = settings->m_columns.get_count();

			m_order; m_order.resize(columnCount);
			for (size_t c = 0; c < settings->m_columns.get_count(); ++c) m_order[c] = (int)c;

			bool order_fault = false;

			for (int i = 0; i < columnCount; i++) {

				long field_id = settings->m_columns[i].m_id;
				PFC_ASSERT(cfg_ui_columns.exists(field_id));

				cfg_ui_iterator column_settings = cfg_ui_columns.find(field_id);
				if (!column_settings.is_valid()) {
					continue;
				}

				AddColumn(column_settings->m_value.m_name, MulDiv(static_cast<int>(400), DPI.cx, 96), column_settings->m_value.m_alignment, true);

				if (settings->m_relative_column_widths) {

					ResizeColumn(i, AUTO_FLAG);
				}

				//order
				t_uint32 tmp_order = settings->m_columns[i].m_order;
				if (tmp_order == ~0 || tmp_order >= columnCount) {
					order_fault = true;
				}
				else {
					m_order[i] = tmp_order;
				}
			}

			if (order_fault || GetColumnCount() != columnCount) {
				m_order = GetColumnOrderArray();
			}

			//apply column order
			if (m_order.size()) {
				GetHeaderCtrl().SetOrderArray(m_order.size(), &m_order[0]);
			}

			if (restore) {
				if (!settings->m_relative_column_widths) {
					RelayoutUIColumns();
				}
				
			}

			OnColumnsChanged();
		}

		//
		//

		void AddUIHeaderColumn(long field_id) {

			ui_element_settings* settings;
			m_ui_host->get_configuration(&settings);

			GetCurrentColumnLayout();

			// update current columns
			auto new_ndx = settings->m_columns.get_count();
			settings->m_columns.add_item(ui_column_settings(field_id));
			settings->m_columns[new_ndx].m_order = new_ndx;
			// mixed auto-resize
			settings->m_columns[new_ndx].m_autoWidth = settings->m_relative_column_widths;

			// add list control column
			cfg_ui_iterator column_settings = cfg_ui_columns.find(field_id);
			auto DPI = GetDPI();

			AddColumn(column_settings->m_value.m_name, MulDiv(static_cast<int>(70), DPI.cx, 96), column_settings->m_value.m_alignment, true);

			if (settings->m_relative_column_widths) {
				
				ResizeColumn(new_ndx, AUTO_FLAG);

			}
			
			m_order = GetColumnOrderArray();

			if (!settings->m_relative_column_widths) {
				RelayoutUIColumns();
			}

			m_ui_host->save_configuration();

			// Update all the items for this column

			pfc::list_t<t_playback_queue_item> queue;
			static_api_ptr_t<playlist_manager>()->queue_get_contents(queue);

			Add_PQI_List_Column(queue, GetColumnCount() - 1);

			if (settings->m_columns.get_size() == 1) {
				QueueRefresh();
			}
		}

		void RemoveUIHeaderColumn(long field_id) {

			ui_element_settings* settings;
			m_ui_host->get_configuration(&settings);

			t_size col_ndx = settings->column_index_from_id(field_id);
			if (col_ndx != SIZE_MAX) {
				settings->m_columns.remove_by_idx(col_ndx);

			}

			//note: vldata[n].cols[ndx] still holding value
			DeleteColumn(col_ndx, true);
			GetCurrentColumnLayout();

			m_ui_host->save_configuration();

		}

		//

		bool GetCurrentColumnLayout() {

			ui_element_settings* settings;
			m_ui_host->get_configuration(&settings);

			auto order = GetColumnOrderArray();

			//walk realtime cols
			for (auto w = 0; w < GetColumnCount(); w++) {

				//width and order
				auto width = GetColumnWidthF(w);
				if (!settings->m_columns[w].m_autoWidth) {
					settings->m_columns[w].m_column_width = width;
				}
				settings->m_columns[w].m_order = order[w];
			}

			return (bool)GetColumnCount();
		}

		void RemoveInvalidColCfgs() {

			PFC_ASSERT(m_ui_host != NULL);
			ui_element_settings* settings;
			m_ui_host->get_configuration(&settings);

			bit_array_bittable delMask(bit_array_false(), settings->m_columns.get_count());

			t_size columnCount = settings->m_columns.get_count();

			for (t_size i = 0; i < columnCount; i++) {

				// Check that column exists
				// Mask removed if it doesn't exist
				long id = settings->m_columns[i].m_id;

				// The column definition does not exists anymore, remove the column!
				if (!cfg_ui_columns.exists(id)) {
					delMask.set(i, true);
					continue;
				}

				cfg_ui_iterator column_settings = cfg_ui_columns.find(id);

				if (!column_settings.is_valid()) {
					delMask.set(i, true);
					continue;
				}
			}

			settings->m_columns.remove_mask(delMask);

			// If no columns are present, use the first defined column
			if (!settings->m_columns.get_count()) {
				if (cfg_ui_columns.get_count()) {
					settings->m_columns.add_item(ui_column_settings(cfg_ui_columns.first()->m_key));
				}
				else {
					//..
				}
			}
		}

		void UpdateHeaderNameAlign() {
			ui_element_settings* settings;
			m_ui_host->get_configuration(&settings);

			for (t_size i = 0; i < settings->m_columns.get_count(); i++) {
				
				auto cf = cfg_ui_columns.find(settings->m_columns[i].m_id);
				if (cf.is_valid()) {
					SetColumn(i, cf->m_value.m_name, cf->m_value.m_alignment, true);
				}
			}
		}

		void RelayoutUIColumns() {

			ui_element_settings* settings;
			m_ui_host->get_configuration(&settings);

			PFC_ASSERT(GetColumnCount() == settings->m_columns.get_count());

			// Resize all columns

			bool relative_column_sizes = settings->m_relative_column_widths;

			for (t_size i = 0; i < settings->m_columns.get_count(); i++) {
			
				// Check that the column exists
				// Mask removed if it doesn't exist
				long field_id = settings->m_columns[i].m_id;

				// Column is defined in the configuration, everything is OK
				
				// Resize column
				CRect rc;
				GetClientRect(&rc);

				int column_nominal_width = settings->m_columns[i].m_column_width;
				bool autoWidth = settings->m_columns[i].m_autoWidth;
				int column_width = autoWidth ? AUTO_FLAG : column_nominal_width;

				if (relative_column_sizes && settings->m_control_width > 0) {
					column_width = (int)(((double)column_width / (double)settings->m_control_width) * (double)rc.Width());
				}

				ResizeColumn(i, column_width);
			}
		}

		void InvalidateHeader() {

			RelayoutUIColumns();
			UpdateItemsAndHeaders(bit_array_true());

			if (cfg_show_header) {
				GetHeaderCtrl().Invalidate(true);
			}

			Invalidate(true);

		}

		virtual uint32_t QueryDragDropTypes() const override {
			return dragDrop_reorder | dragDrop_external;
		}

		virtual DWORD DragDropSourceEffects() override {
			return DROPEFFECT_MOVE | DROPEFFECT_COPY /*DRAGDROP_S_DROP */;
		}

		// Outgoing drop

		virtual pfc::com_ptr_t<IDataObject> MakeDataObject() override {

			bit_array_bittable selMask = GetSelectionMask();
			t_size selSize = selMask.size();

			metadb_handle_list mhl;
			t_size n = selMask.find_first(true, 0, selSize);
			for (n; n < selSize; n = selMask.find_next(true, n, selSize)) {

				data_t rec = my_data.get_item(n);
				metadb_handle_ptr track_bm = rec.pqi.m_handle;
				mhl.add_item(track_bm);
			}

			static_api_ptr_t<playlist_incoming_item_filter> piif;
			pfc::com_ptr_t<IDataObject> pDataObject = piif->create_dataobject_ex(mhl);

			return pDataObject;
		}

	protected:

		virtual int GroupHeaderFontWeight(int origVal) const override {

			if (cfg_show_header) {
				return origVal;
			}
			else {
				return 0;
			}
		}

		virtual void OnColumnHeaderClick(t_size index) override {
			//..
		}

	private:

		bool m_cui = false;

		ui_element_host* m_ui_host;
		ui_element_instance_callback_ptr m_callback;

		metadb_handle_list my_selection;
		ui_selection_holder::ptr m_shared_selection;

		metadb_handle_list m_metadbs;

		size_t m_lastDDMark;
		metadb_handle_list m_mhl_dropped;

		queuecontents_titleformat_hook m_hook;
		pfc::list_t<t_playback_queue_item> my_queue;

		pfc::array_staticsize_t<long> m_header_ctx_menu_field_ids;
		std::vector<int>m_order;

		CFlashWindow m_flash;
	};
}


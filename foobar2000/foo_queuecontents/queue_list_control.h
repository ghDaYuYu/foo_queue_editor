#pragma once

#include "pfc/array.h"
#include "libPPUI/CFlashWindow.h"
#include "libPPUI/CListControlOwnerData.h"
#include "helpers/CListControlFb2kColors.h"

#include "queuecontents_titleformat_hook.h" 
#include "queue_helpers.h"

#include "ui_element_host.h"
#include "config.h"

#include "queue_list_control_ILO.h"

#define AUTO_FLAG UINT32_MAX

namespace dlg {

	typedef CListControlFb2kColors <CListControlOwnerData> CListControlOwnerColors;

	class CListControlQueue : public CListControlOwnerColors {
	public:

		CListControlQueue(ILOD_QueueSource* h, bool is_cui) : CListControlOwnerColors(h),
			m_cui(is_cui), m_hook() {

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

		void ClearLastDrop() {
			m_mhl_dropped.remove_all();
			m_lastDDMark = SIZE_MAX;
		}

		process_locations_notify::func_t m_drop_notify_async_fx = [&](metadb_handle_list_cref op) {
			m_mhl_dropped = op;

			if (m_lastDDMark == SIZE_MAX) {
				//drop over empty list?
				m_lastDDMark = 0;
			}

			if (m_lastDDMark != SIZE_MAX) {

				static_api_ptr_t<playlist_manager> playlist_api;
				pfc::list_t<t_playback_queue_item> queue;
				playlist_api->queue_get_contents(queue);
				t_size cq = queue.get_count();
				if (m_lastDDMark < cq) {
					queue_helpers::queue_insert_items(m_lastDDMark, m_mhl_dropped);
				}
				else {
					queue_helpers::queue_add_items(m_mhl_dropped, true);
				}
			}
			ClearLastDrop();
		};

		void SetHost(ui_element_host* host) { m_ui_host = host; }

		//todo: revise shared selections

		void SetSharedSelection(metadb_handle_list_cref p_items);
		void SetSharedSelection();

		void AppendShowHeaderMenuItem(CMenuHandle menu, int ID);
		void ShowHideColumnHeader();

		void CommandListItemContextMenu(unsigned p_id, unsigned p_id_base, CPoint point);
		void CommandListNoItemContextMenu(unsigned p_id, unsigned p_id_base, CPoint point);
		void CommandFrameStyleContextMenu(unsigned p_id, unsigned p_id_base, CPoint point);
		void CommandHeaderContextMenu(unsigned p_id, unsigned p_id_base, CPoint point);
		void CommandContextMenu(CPoint screen_point, unsigned p_id, unsigned p_id_base);

		void AppendFrameStyleMenuItems(CMenuHandle menu, unsigned p_id_base, CPoint point);
		void BuildHeaderContextMenu(CMenuHandle menu, unsigned p_id_base, CPoint point);
		void BuildListItemContextMenu(CMenuHandle menu, unsigned p_id_base, CPoint point);
		void BuildListNoItemContextMenu(CMenuHandle menu, unsigned p_id_base, CPoint point);
		void BuildContextMenu(CPoint screen_point, CMenuHandle menu, unsigned p_id_base);

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
		}

		void OnKillFocus(CWindow wndFocus);
		void OnSetFocus(CWindow wndOld);

		void GetMetaDbHandles(metadb_handle_list_ref p_out);
		metadb_handle_list_ref GetAllMetadbs() { return m_metadbs; }

		void GetSelectedItemsMetaDbHandles(metadb_handle_list_ref p_out);

		void EditModeContextMenuGetFocusPoint(CPoint& pt);

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
						need_data_requery = !was_cf->m_value.m_pattern.equals(is_cf->m_value.m_pattern);
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

			if (need_data_requery) {
				QueueReset();
				QueueRefresh();
			}
			else {
				if (need_header_rebuilt) {
					BuildColumns(true, false);
					QueueReset();
					QueueRefresh();
				}
				else if (need_header_refresh) {
					UpdateHeaderNameAlign();
				}
			}

		}

		void BuildColumns(bool restore, bool defaults = false) {

			DeleteColumns(bit_array_true());

			ui_element_settings* settings;
			m_ui_host->get_configuration(&settings);

			auto DPI = GetDPI();

			if (defaults) {
				settings->m_columns.remove_all();
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
				endif
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

		// Drag and drop

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

		virtual void OnColumnHeaderClick(t_size index) override {
			//..
		}

	private:

		bool m_cui = false;

		ui_element_host* m_ui_host;
		ui_element_instance_callback_ptr m_callback;

		int m_focused_item;

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


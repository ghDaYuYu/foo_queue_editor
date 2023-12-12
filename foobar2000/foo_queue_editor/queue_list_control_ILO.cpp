#include "stdafx.h"
#include "queue_list_control.h"
#include "queue_list_control_ILO.h"
#include "foo_vbookmark/IBookmark.h"


namespace dlg {

	size_t ILOD_QueueSource::listGetItemCount(ctx_t) {

		return my_data.get_count();
	}

	pfc::string8 ILOD_QueueSource::listGetSubItemText(ctx_t ctx, size_t item, size_t subItem) {

		//the subItem order has already been resolved

		const auto& rec = my_data[item];
		auto text = rec.vcols.at(subItem);

		return text;
	}

	void ILOD_QueueSource::listSelChanged(ctx_t ctx) {

		CListControlQueue* plc = (CListControlQueue*)(ctx);
		plc->SetSharedSelection();
	}

	bool ILOD_QueueSource::listReorderItems(ctx_t ctx, const size_t* order, size_t count) {

		PFC_ASSERT(count == my_data.get_count());

		CListControlQueue* plc = (CListControlQueue*)(ctx);
		plc->MoveSelected(order, count);
		//todo: rev other count mismatch crash scenerios
		if (count != ctx->GetItemCount()) {
			plc->ReloadData();
			plc->ReloadItems(bit_array_true());
			plc->SelectNone();
			return false;
		}
		return true;
	}

	bool ILOD_QueueSource::listRemoveItems(ctx_t ctx, pfc::bit_array const& mask) {

		size_t oldCount = my_data.get_count();
		CListControlQueue* plc = (CListControlQueue*)(ctx);

		plc->DeleteSelected();

		return true;
	}

	void ILOD_QueueSource::listItemAction(ctx_t ctx, size_t item) {
		//..
	}

	uint32_t ILOD_QueueSource::listGetEditFlags(ctx_t ctx, size_t item, size_t subItem) {
		return listIsColumnEditable(ctx, subItem) ? 0 : InPlaceEdit::KFlagReadOnly;
	}

	bool ILOD_QueueSource::listIsColumnEditable(ctx_t ctx, size_t subItem) {
		return false;
	}

	//
	// incomming
	//

	void ILOD_QueueSource::listDragDropSourceSucceeded(ctx_t, DWORD effect) {
		DEBUG_PRINT << "...listDragDropSourceSucceeded";
		//..
	}

	pfc::com_ptr_t<IDataObject> ILOD_QueueSource::listMakeDataObject(ctx_t) {
		DEBUG_PRINT << "...listMakeDataObject";

		return nullptr;
	}

	DWORD ILOD_QueueSource::listDragDropAccept(ctx_t ctx, IDataObject* obj, bool& showDropMark) {

		DEBUG_PRINT << "...listDragDropAccept";

		static_api_ptr_t<playlist_incoming_item_filter> incoming_item_filter;

		if (incoming_item_filter->process_dropped_files_check(obj)) {
			// IDataObject convertible to metadb_handle
			showDropMark = true;
			return DROPEFFECT_COPY;
		}
		return DROPEFFECT_NONE;
	}

	DWORD ILOD_QueueSource::listDragDropSourceEffects(ctx_t) {
		DEBUG_PRINT << "...listDragDropSourceEffects";

		return DROPEFFECT_MOVE | DROPEFFECT_COPY;
	}

	uint32_t ILOD_QueueSource::listQueryDragDropTypes(ctx_t) const {
		DEBUG_PRINT << "...listQueryDragDropTypes";

		return 0;
	}

	class process_locations_notify_lambda : public process_locations_notify {
	public:
		process_locations_notify::func_t f;
		void on_completion(metadb_handle_list_cref p_items) override {
			PFC_ASSERT(f != nullptr);
			f(p_items);
		}
		void on_aborted() override {}
	};

	//todo
	bool GetActivePlaylistState(ActivePlaylistInformation &drop_notify_playlist_state) {

		pfc::list_t<metadb_handle_ptr> mhpl_selection_mngr;
		static_api_ptr_t<ui_selection_manager> selection_manager;
		selection_manager->get_selection(mhpl_selection_mngr);
		auto dbg_sm_guid = selection_manager->get_selection_type();
		
		bool from_x = false;
		bool from_library = false;
		bool from_playlist = false;
		bool from_now_playing = false;
		bool from_active_playlist_selection = false;

		auto guid_sel_type = selection_manager->get_selection_type();
		drop_notify_playlist_state.src = guid_sel_type;

		GUID guid_x = pfc::GUID_from_text("D154DEDA-7BE7-4075-809A-647DF819E574");

		GUID guid_active_sel = contextmenu_item::caller_active_playlist_selection;
	
		auto guid_dbg = ui_selection_manager::get()->get_selection_type();

		from_playlist = pfc::guid_equal(guid_sel_type, contextmenu_item::caller_active_playlist_selection);
		// 994C0D0E-319E-45F3-92FC-518616E73ADC
		from_now_playing = pfc::guid_equal(guid_sel_type, contextmenu_item::caller_now_playing);
		// D154DEDA-7BE7-4075-809A-647DF819E574
		from_x = pfc::guid_equal(guid_sel_type, guid_mistery);
		// FDA07C56-05D0-4B84-9FBD-A8BE556D474D
		from_library = pfc::guid_equal(guid_sel_type, contextmenu_item::caller_media_library_viewer);
		// 47502BA1-816D-4a3e-ADE5-A7A9860A67DB
		from_active_playlist_selection = pfc::guid_equal(guid_sel_type, contextmenu_item::caller_active_playlist_selection);

		if (from_library || from_x) {

			if (from_x) {
				//..
			}

			//todo: rev
			//pqi might get a hold of the active playlist index
			//selected positions,...

			static_api_ptr_t<playlist_manager_v5> playlist_api;
			auto active_ndx = playlist_api->get_active_playlist();
			if (active_ndx != ~0) {
				auto pl_item_count = playlist_api->activeplaylist_get_item_count();
				if (pl_item_count) {

					metadb_handle_list pmhl_allitems;
					drop_notify_playlist_state.active_playlist_index = active_ndx;
					drop_notify_playlist_state.active_playlist_item_count = pl_item_count;
					drop_notify_playlist_state.active_playlist_all_items = std::move(pmhl_allitems);
					return true;
				}
			}
		}

		if (from_playlist) {

			static_api_ptr_t<playlist_manager_v5> playlist_api;
			auto active_ndx = playlist_api->get_active_playlist();
			if (active_ndx != ~0) {

				auto csel = playlist_api->activeplaylist_get_selection_count(255);

				if (csel) {

					auto pl_item_count = playlist_api->activeplaylist_get_item_count();
					pfc::bit_array_bittable selmask(bit_array_false(), pl_item_count);
					playlist_api->activeplaylist_get_selection_mask(selmask);
					t_size item = selmask.find_first(true, 0, pl_item_count);

					if (item < pl_item_count) {
					
						metadb_handle_list pmhl_selected;
						metadb_handle_list pmhl_allitems;
						playlist_api->activeplaylist_get_selected_items(pmhl_selected);

						drop_notify_playlist_state.active_playlist_index = active_ndx;
						drop_notify_playlist_state.active_playlist_item_count = pl_item_count;
						drop_notify_playlist_state.active_playlist_selection_mask = selmask;
						drop_notify_playlist_state.active_playlist_selected_items = std::move(pmhl_selected);

						return true;
					}
				}
			}
		}
		return false;
	}

	//todo
	void ILOD_QueueSource::listOnDrop(ctx_t ctx, IDataObject* obj, CPoint pt) {

		//todo: bookmark copy/paste
		//bool b_from_ibom = CPoint(-1, -1) == pt;

		DEBUG_PRINT << "listOnDrop";

		std::vector<GUID> v_ibom_guids;

		if (ibom::IBom::isBookmarkDeco(obj)) {

			ibom::IBom* pBookmarkDataObject = static_cast<ibom::IBom*>(obj);
			if (pBookmarkDataObject) {
				try {

					LPSAFEARRAY lpSafeArray = pBookmarkDataObject->GetPlaylistGuids();

					LONG /*lBound,*/ uBound;
					//SafeArrayGetLBound(lpSafeArray, 1, &lBound);
					SafeArrayGetUBound(lpSafeArray, 1, &uBound);
					LONG count = uBound + 1;

					BSTR* raw;
					SafeArrayAccessData(lpSafeArray, (void**)&raw);

					std::vector<std::wstring> v(raw, raw + count);

					pfc::stringcvt::string_utf8_from_wide temp;
					for (const auto w : v) {
						//todo: rev bookmark guid types
						temp.convert(w.data(), w.size());
						GUID guid = pfc::GUID_from_text(temp.get_ptr());
						v_ibom_guids.emplace_back(std::move(guid));
					}

					SafeArrayUnaccessData(lpSafeArray);
				}
				catch (...) {
					//..
				}
			}
		}
		else {
			//..
		}

		CListControlQueue* plc = (CListControlQueue*)(ctx);

		//todo: bookmark copy/paste
		if (b_from_ibom) {
			plc->SetLastDDMarkIbom();
		}
		else {
			plc->SetLastDDMark();
		}

		ActivePlaylistInformation dron_playlist_state;
		if (GetActivePlaylistState(dron_playlist_state)) {
			//..
		}

		dron_playlist_state.ibom_selection_guids = std::move(v_ibom_guids);
		plc->SetDropNofifyPlaylistStated(std::move(dron_playlist_state));

		auto notify = fb2k::service_new< process_locations_notify_lambda >();
		notify->f = plc->m_drop_notify_async_fx;

		static_api_ptr_t<playlist_incoming_item_filter_v2> incoming_item_filter;
		incoming_item_filter->process_dropped_files_async(obj, NULL, ctx->m_hWnd, notify);
	}
}


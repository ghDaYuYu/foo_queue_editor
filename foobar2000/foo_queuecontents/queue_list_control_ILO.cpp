#include "stdafx.h"
#include "queue_list_control.h"
#include "queue_list_control_ILO.h"


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

	void ILOD_QueueSource::listOnDrop(ctx_t ctx, IDataObject* obj, CPoint pt) {

		DEBUG_PRINT << "listOnDrop";

		CListControlQueue* plc = (CListControlQueue*)(ctx);

		plc->SetLastDDMark();

		auto notify = fb2k::service_new< process_locations_notify_lambda >();
		notify->f = plc->m_drop_notify_async_fx;

		static_api_ptr_t<playlist_incoming_item_filter_v2> incoming_item_filter;
		incoming_item_filter->process_dropped_files_async(obj, NULL, ctx->m_hWnd, notify);
	}
}


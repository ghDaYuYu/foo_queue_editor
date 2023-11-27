#pragma once
#include <objidl.h>
#include <atltypes.h>

#include "SDK/playlist.h"
#include "libPPUI/IDataObjectUtils.h"
#include "pfc/string-lite.h"
#include "pfc/int_types.h"
#include "pfc/bit_array.h"
#include "pfc/list.h"
#include "pfc/com_ptr_t.h"

#include "libPPUI/CListControlOwnerData.h"

namespace dlg {

	struct data_t {

		t_playback_queue_item pqi;
		std::vector<pfc::string8>vcols; //subItems

		void reset() {
			pqi = t_playback_queue_item();
			vcols.clear();
		}
	};

	inline pfc::list_t<data_t> my_data;


	class CListControlQueue;

	class ILOD_QueueSource : public IListControlOwnerDataSource {

	public:

		//overrides...

		virtual uint32_t listQueryDragDropTypes(ctx_t) const override;
		virtual DWORD listDragDropAccept(ctx_t, IDataObject* obj, bool& showDropMark) override;
		virtual pfc::com_ptr_t<IDataObject> listMakeDataObject(ctx_t) override;
		virtual void listOnDrop(ctx_t, IDataObject* obj, CPoint pt) override;
		virtual DWORD listDragDropSourceEffects(ctx_t) override;
		virtual void listDragDropSourceSucceeded(ctx_t, DWORD effect) override;

		virtual size_t listGetItemCount(ctx_t) override;
		virtual pfc::string8 listGetSubItemText(ctx_t, size_t item, size_t subItem) override;
		virtual void listSelChanged(ctx_t) override;

		void listItemAction(ctx_t, size_t) override;
		bool listRemoveItems(ctx_t, pfc::bit_array const&) override;

		bool listCanReorderItems(ctx_t) override { return true; }
		bool listReorderItems(ctx_t, const size_t*, size_t) override;

		// Called prior to o typefind pass attempt, you can either deny entirely, or prepare any necessary data and allow it.
		bool listAllowTypeFind(ctx_t) override { return true; }
		// Allow type-find in o specific item/column?
		bool listAllowTypeFindHere(ctx_t ctx, size_t item, size_t subItem) { return true; };

		bool listIsColumnEditable(ctx_t, size_t) override;
		uint32_t listGetEditFlags(ctx_t ctx, size_t item, size_t subItem) override;

		virtual void listColumnHeaderClick(ctx_t, size_t subItem) override {
			//..
		}

	};
}

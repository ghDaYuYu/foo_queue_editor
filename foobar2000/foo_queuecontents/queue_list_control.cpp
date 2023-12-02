#include "stdafx.h"
#include <regex>
#include <iomanip>

#include "libPPUI/listview_helper.h"
#include "queue_helpers.h"
#include "queue_list_control.h"

namespace dlg {

	// No list item context menu
	enum {
		// Show/Hide Column Header
		// todo:
		//ID_SHOW_COLUMN_HEADER = 1,
		ID_PREFERENCES = 2,
		ID_BEGIN_CUSTOM,
	};

	// List item context menu
	enum {
		// let's not redefine ID_SHOW_COLUMN_HEADER or ID_PREFERENCES

		// ID for "Remove"
		ID_REMOVE = ID_BEGIN_CUSTOM,
		ID_PL_SEND_TO_CURRENT,
		ID_PL_ADD_TO_CURRENT,
		ID_PL_SEND_NEW,
		ID_PROPERTIES,
		// ID for "Move to Top" and 
		ID_MOVE_TOP,
		// ID for "Move to Bottom"
		ID_MOVE_BOTTOM,
		// ID for "Move to Bottom"

		// The range ID_CONTEXT_FIRST through ID_CONTEXT_LAST is reserved
		// for menu entries from menu_manager.
		// ID_CONTEXT_FIRST,
		// ID_CONTEXT_LAST = ID_CONTEXT_FIRST + 1000,
	};

	// Header item context menu
	enum {
		// let's not redefine
		/*ID_SHOW_COLUMN_HEADER*/
		// Reserved for columns
		ID_COLUMNS_FIRST = ID_BEGIN_CUSTOM,
		// Reserved for columns
		ID_COLUMNS_LAST = ID_COLUMNS_FIRST + 1000,
		ID_RELATIVE_WIDTHS = ID_COLUMNS_LAST + 2, // "Auto-scale Columns with Window Size"
		ID_BORDER_NONE = ID_COLUMNS_LAST + 3,
		ID_BORDER_GREY = ID_COLUMNS_LAST + 4,
		ID_BORDER_SUNKEN = ID_COLUMNS_LAST + 5,
		ID_RESET_HEADER,
	};

	void CListControlQueue::OnContextMenu(CWindow wnd, CPoint p_point) {
		TRACK_CALL_TEXT("CListControlQueue::OnContextMenu");

		CPoint point = GetContextMenuPoint(p_point);
		CPoint ptInvalid(-1, -1);
		if (point == ptInvalid) {
			//no items in list
		::GetCursorPos(&point);
		}
		
		if (m_callback.is_valid() && m_callback->is_edit_mode_enabled()) {
			SetMsgHandled(TRUE);
			return;
		}

		CMenu menu;
		menu.CreatePopupMenu();
		unsigned p_base_id = 0;

		CMenuHandle menuhandle(menu);
		BuildContextMenu(point, menuhandle, p_base_id);

		unsigned cmd = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON,
			point.x, point.y, 0, *this, 0);


		CommandContextMenu(point, cmd, p_base_id);

	}

	void CListControlQueue::OnKillFocus(CWindow wndFocus) {

		m_shared_selection.release();
		// Do the default action
		SetMsgHandled(FALSE);
	}

	void CListControlQueue::OnSetFocus(CWindow wndOld) {

		DEBUG_PRINT << "OnSetFocus";

		// Inform of the current selection to foobar

		SetSharedSelection();

		// Do the default action
		SetMsgHandled(FALSE);
	}

	void CListControlQueue::SetSharedSelection(metadb_handle_list_cref p_items) {

		if (p_items.g_equals(p_items, my_selection)) {
			return;
		}

		TRACK_CALL_TEXT("CListControlQueue::SetSharedSelections");

		// Only notify other components about changes in our selection,
		// if our window is the active one.

		if (::GetFocus() == m_hWnd) {

			// Acquire o ui_selection_holder that allows us to notify other components
			// of the selected tracks in our window, when it has the focus.
			m_shared_selection = static_api_ptr_t<ui_selection_manager>()->acquire();

			if (m_shared_selection.is_valid()) {

				my_selection = p_items;
				m_shared_selection->set_selection_ex(p_items, contextmenu_item::caller_undefined);
				return;
			}
		}
	}

	void CListControlQueue::SetSharedSelection() {

		metadb_handle_list mhl_sel;
		m_metadbs.get_items_mask(mhl_sel, GetSelectionMask());
		SetSharedSelection(mhl_sel);
	}

	void CListControlQueue::QueueRefresh() {

		TRACK_CALL_TEXT("CListControlQueue::QueueRefresh");

		pfc::list_t<t_playback_queue_item> queue;
		static_api_ptr_t<playlist_manager>()->queue_get_contents(queue);

		auto selmask = GetSelectionMask();
		auto m_focused_item = GetFocusItem();

		if (queue.g_equals(queue, my_queue)) {
			return;
		}

		my_queue.remove_all();
		my_queue = queue;

		//todo
		// Save focus

		m_metadbs.remove_all();
		my_data.remove_all();

		queue_helpers::extract_metadb_handles(queue, m_metadbs);

		Add_PQI_List(queue);

		// Try to restore selection
		SetSelection(bit_array_true(), selmask);

		// Try to restore focus marker
		if (m_focused_item < my_data.get_count()) {

			SetFocusItem(m_focused_item);
		}

		ReloadData();
		ReloadItems(bit_array_true());
	}

	void CListControlQueue::GetMetaDbHandles(metadb_handle_list_ref p_out) {

		TRACK_CALL_TEXT("CListControlQueue::GetMetaDbHandles");

		t_size indices_count = my_data.get_count();

		// Transform indices to o bitarray
		bit_array_bittable indices_bit_array(bit_array_true(), GetItemCount());

		metadb_handle_list tmp;
		// Retrieve the queue playback items
		m_metadbs.get_items_mask(tmp, indices_bit_array);

		// list has overloaded =, so this works 'as expected'
		p_out = tmp;
	}

	void CListControlQueue::GetSelectedItemsMetaDbHandles(metadb_handle_list_ref p_out) {
		TRACK_CALL_TEXT("CListControlQueue::GetSelectedItemsMetaDbHandles");
		pfc::list_t<t_size> selected;
		GetMetaDbHandles(p_out);
	}

	void CListControlQueue::Add_PQI_List_Column(pfc::list_t<t_playback_queue_item> lpqi, t_size col_ndx) {
		TRACK_CALL_TEXT("CListControlQueue::AddItems - column overload");
		t_size count = lpqi.get_count();

		for (t_size i = 0; i < count; i++) {
			Add_PQI_SubItem(lpqi[i], i + 1, col_ndx); // Items must be passed in index order!
		}
	}

	void CListControlQueue::Add_PQI_List(pfc::list_t<t_playback_queue_item> lpqi) {

		TRACK_CALL_TEXT("CListControlQueue::AddItems");

		std::vector<int> vorder = GetColumnOrderArray();

		for (t_size i = 0; i < lpqi.get_count(); i++) {
			Add_PQI_Item(lpqi[i], i + 1);
		}
	}

	void CListControlQueue::Add_PQI_Item(t_playback_queue_item pqi, t_size qpos) {
		TRACK_CALL_TEXT("CListControlQueue::AddItem");
		PFC_ASSERT(m_ui_host != NULL);

		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		PFC_ASSERT(GetColumnCount() == settings->m_columns.get_count());

		for (t_size col_ndx = 0; col_ndx < settings->m_columns.get_count(); col_ndx++) {
			//DEBUG_PRINT << "AddItem on row " << index << " in column (index) " << column_index;
			Add_PQI_SubItem(pqi, qpos, col_ndx); // Items must be passed in index order!
		}
	}

	void CListControlQueue::Add_PQI_SubItem(t_playback_queue_item pqi, /*1base*/t_size qpos, t_size col_ndx) {
		TRACK_CALL_TEXT("CListControlQueue::AddItem - column overload");

		PFC_ASSERT(m_ui_host != NULL);
		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		long field_id = settings->m_columns[col_ndx].m_id;
		cfg_ui_iterator column_definition = cfg_ui_columns.find(field_id);

		PFC_ASSERT(column_definition.is_valid());

		pfc::string8 itemString;

		if (!column_definition.is_valid()) {
			// The column is not valid
			itemString = "error";
		}
		else {
			// We use our own format hook for interpreting %queue_index% etc. as it doesn't work default 
			service_ptr_t<titleformat_object> format;
			m_hook.setData(qpos, pqi);
			static_api_ptr_t<titleformat_compiler>()->compile(format, column_definition->m_value.m_pattern);
			pqi.m_handle->format_title(&m_hook, itemString, format, NULL);
			format.release();
		}

		// current or new row
		// qpos is 1base

		if (qpos > my_data.get_count()) {

			my_data.add_item(data_t());
		}

		data_t & rec = my_data[qpos - 1];

		//set queue item handle

		if (!rec.pqi.m_handle.get_ptr()) {
			rec.pqi = pqi;
		}

		// current or new field

		if (col_ndx >= rec.vcols.size()) {
			rec.vcols.emplace_back();
		}

		rec.vcols[col_ndx] = itemString;

		if (qpos == 0) {
			DEBUG_PRINT << "CListViewCtrl::AddItem on row " << (qpos - 1) << " in column (index) " << col_ndx;
		}
	}


	void CListControlQueue::BuildContextMenu(CPoint screen_point, CMenuHandle menu, unsigned p_id_base) {

		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		CRect headerRect;
		//if (settings->m_show_column_header) {
			GetHeaderCtrl().GetClientRect(headerRect);
			GetHeaderCtrl().ClientToScreen(headerRect);
		//}

		if (headerRect.PtInRect(screen_point) || !settings->m_columns.get_count()) {
			BuildHeaderContextMenu(menu, p_id_base, screen_point);
		}
		else {
			CPoint pt(screen_point);
			LVHITTESTINFO hitTestInfo;
			ScreenToClient(&pt);
			hitTestInfo.pt = pt;

			auto ht = AccItemHitTest(pt);
			if (ht != SIZE_MAX) {
				BuildListItemContextMenu(menu, p_id_base, pt);
			}
			else {
				BuildListNoItemContextMenu(menu, p_id_base, screen_point);
			}
		}
	}

	//todo
	void CListControlQueue::AppendShowHeaderMenuItem(CMenuHandle menu, int ID) {

		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		int header_checked = settings->m_show_column_header;
		menu.AppendMenu(header_checked ? MF_CHECKED /*| MF_DISABLED | MF_GRAYED*/: 0, ID, _T("Show column header"));
	}

	void CListControlQueue::BuildListItemContextMenu(CMenuHandle menu, unsigned p_id_base, CPoint point) {

		//AppendShowHeaderMenuItem(menu, ID_SHOW_COLUMN_HEADER + p_id_base);
		AppendFrameStyleMenuItems(menu, p_id_base, point);

		static_api_ptr_t<playlist_manager_v5> playlist_api;
		bool active_fault = playlist_api->get_active_playlist() == SIZE_MAX;
		active_fault |= playlist_api->playlist_lock_is_present(playlist_api->get_active_playlist());

		if (menu.GetMenuItemCount()) {
			menu.AppendMenu(MF_SEPARATOR);
		}
		menu.AppendMenu(MF_STRING, ID_REMOVE + p_id_base, _T("&Remove"));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING | (active_fault ? MF_DISABLED | MF_GRAYED : 0), ID_PL_SEND_TO_CURRENT + p_id_base, _T("&Send to Current Playlist"));
		menu.AppendMenu(MF_STRING | (active_fault ? MF_DISABLED | MF_GRAYED : 0), ID_PL_ADD_TO_CURRENT + p_id_base, _T("&Add to Current Playlist"));
		menu.AppendMenu(MF_STRING, ID_PL_SEND_NEW + p_id_base, _T("Send to& New Playlist"));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_MOVE_TOP + p_id_base, _T("Move to &top"));
		menu.AppendMenu(MF_STRING, ID_MOVE_BOTTOM + p_id_base, _T("Move to &bottom"));
		menu.AppendMenuW(MF_SEPARATOR);
		menu.AppendMenuW(MF_STRING, ID_PREFERENCES + p_id_base, _T("Queue Editor &preferences..."));
		menu.AppendMenuW(MF_SEPARATOR);
		menu.AppendMenuW(MF_STRING, ID_PROPERTIES + p_id_base, _T("Properties"));
	}

	void CListControlQueue::BuildListNoItemContextMenu(CMenuHandle menu, unsigned id_base, CPoint point) {

		//AppendShowHeaderMenuItem(menu, ID_SHOW_COLUMN_HEADER + id_base);
		AppendFrameStyleMenuItems(menu, id_base, point);
		if (menu.GetMenuItemCount()) {
			menu.AppendMenuW(MF_SEPARATOR);
		}
		menu.AppendMenuW(MF_STRING, ID_PREFERENCES + id_base, _T("Queue Editor preferences..."));
	}

	void CListControlQueue::AppendFrameStyleMenuItems(CMenuHandle menu, unsigned p_id_base, CPoint point) {

		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		// Add Frame Style sub menu if using CUI
		if (!m_ui_host->is_dui()) {
			CMenu frame_style_submenu;
			frame_style_submenu.CreatePopupMenu();
			menu.AppendMenuW(MF_STRING | MF_POPUP, frame_style_submenu.m_hMenu, _T("Frame style"));

			// None
			UINT non_checked = MFT_RADIOCHECK | (settings->m_border == 0 ? MF_CHECKED : MF_UNCHECKED);
			frame_style_submenu.AppendMenuW(non_checked, ID_BORDER_NONE + p_id_base, _T("None"));

			// Grey
			UINT grey_checked = MFT_RADIOCHECK | (settings->m_border == WS_EX_STATICEDGE ? MF_CHECKED : MF_UNCHECKED);
			frame_style_submenu.AppendMenuW(grey_checked, ID_BORDER_GREY + p_id_base, _T("Grey"));

			// Sunken
			UINT sunken_checked = MFT_RADIOCHECK | (settings->m_border == WS_EX_CLIENTEDGE ? MF_CHECKED : MF_UNCHECKED);
			frame_style_submenu.AppendMenuW(sunken_checked, ID_BORDER_SUNKEN + p_id_base, _T("Sunken"));

		}
	}

	void CListControlQueue::CommandFrameStyleContextMenu(unsigned p_id, unsigned p_id_base, CPoint point) {

		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		auto ori_val = settings->m_border;

		if (p_id == ID_BORDER_NONE + p_id_base) {
			settings->m_border = 0;
		}
		else if (p_id == ID_BORDER_GREY + p_id_base) {
			settings->m_border = WS_EX_STATICEDGE;
		}
		else if (p_id == ID_BORDER_SUNKEN + p_id_base) {
			settings->m_border = WS_EX_CLIENTEDGE;
		}

		if (ori_val != settings->m_border) {
			m_ui_host->save_configuration();
			// Refresh border
			m_ui_host->RefreshVisuals();
		}
	}

	void CListControlQueue::CommandHeaderContextMenu(unsigned cmd, unsigned id_base, CPoint point) {

		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		if (false /*cmd == ID_SHOW_COLUMN_HEADER + id_base*/) {
			ShowHideColumnHeader();
		}
		else if (cmd == ID_RESET_HEADER) {
			ResetColumns();
		}
		else if (cmd == ID_PREFERENCES + id_base) {
			static_api_ptr_t<ui_control>()->show_preferences(guid_preferences);
		}
		else if (cmd == ID_RELATIVE_WIDTHS + id_base) {

			settings->m_relative_column_widths = !settings->m_relative_column_widths;

			// refresh layout...
			for (int i = 0; i < GetColumnCount(); i++) {
				if (settings->m_relative_column_widths) {
					if (i == GetColumnCount() - 1) {
						//set auto width
						ResizeColumn(i, AUTO_FLAG, false);
					}
					else {
						ResizeColumn(i, AUTO_FLAG, false);
					}
				}
				else {
					RelayoutUIColumns();
					GetCurrentColumnLayout();
					SendMessage(WM_SIZE, 0, MAKELPARAM(-1, -1));
					break;
				}
			}

			// Save configuration

			m_ui_host->save_configuration();
		}
		else if (cmd >= ID_COLUMNS_FIRST + id_base && cmd <= ID_COLUMNS_LAST + id_base) {

			long column_id = m_header_ctx_menu_field_ids[cmd - ID_COLUMNS_FIRST - id_base];
			PFC_ASSERT(cfg_ui_columns.exists(column_id));


			if (settings->column_exists(column_id)) {
				RemoveUIHeaderColumn(column_id);
			}
			else {
				AddUIHeaderColumn(column_id);
			}
		}
		else {
			if (!m_ui_host->is_dui()) {
				CommandFrameStyleContextMenu(cmd, id_base, point);
			}
		}
	}

	void CListControlQueue::BuildHeaderContextMenu(CMenuHandle menu, unsigned id_base, CPoint point) {

		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		// Show Header and Frame Style menu options //

		//AppendShowHeaderMenuItem(menu, ID_SHOW_COLUMN_HEADER + id_base);
		AppendFrameStyleMenuItems(menu, id_base, point);
		if (menu.GetMenuItemCount()) {
			menu.AppendMenu(MF_SEPARATOR);
		}

		// Defined fields and checked state //

		// Since iterating a map has unpredictable order
		// we save the ids corresponding to context menu items
		t_size c = cfg_ui_columns.get_count();
		m_header_ctx_menu_field_ids = pfc::array_staticsize_t<long>(c);

		int ndx = 0;

		for (cfg_ui_iterator iter = cfg_ui_columns.first(); iter.is_valid(); iter++) {

			if (!ndx) {
				menu.AppendMenuW(MF_GRAYED | MF_DISABLED, 1, _T("Columns"));
			}

			m_header_ctx_menu_field_ids[ndx] = iter->m_key;

			pfc::string8 field_name(iter->m_value.m_name);

			//check m_columns
			UINT checked = settings->column_exists(iter->m_key) ? MF_CHECKED : MF_UNCHECKED;
			menu.AppendMenuW(checked, ID_COLUMNS_FIRST + ndx + id_base, pfc::stringcvt::string_os_from_utf8(field_name));
			ndx++;
		}
		menu.AppendMenuW(MF_SEPARATOR);
		menu.AppendMenuW(MF_STRING, ID_RESET_HEADER + id_base, _T("Reset header"));
		menu.AppendMenuW(MF_SEPARATOR);

		// Preferences //

		menu.AppendMenuW(MF_STRING, ID_PREFERENCES + id_base, _T("Queue Editor preferences..."));
		menu.AppendMenuW(MF_SEPARATOR);

		// Dynamic column widths //

		bool header_enabled = settings->m_show_column_header;
		UINT relative_checked = settings->m_relative_column_widths ? MF_CHECKED : MF_UNCHECKED;
		menu.AppendMenuW(relative_checked /*| (header_enabled ? 0 : MF_DISABLED | MF_GRAYED)*/, ID_RELATIVE_WIDTHS + id_base, _T("Auto-scale Columns with Window Size"));
	}

	void CListControlQueue::ShowHideColumnHeader() {

		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		settings->m_show_column_header = !settings->m_show_column_header;

		SendMessage(GetParent(), WM_SIZE, -1, -1);

		m_ui_host->save_configuration();
	}

	void CListControlQueue::CommandListNoItemContextMenu(unsigned cmd, unsigned id_base, CPoint point) {

		if (false/* cmd == ID_SHOW_COLUMN_HEADER + id_base*/) {
			ShowHideColumnHeader();
		}
		else if (cmd == ID_PREFERENCES + id_base) {
			static_api_ptr_t<ui_control>()->show_preferences(guid_preferences);
		}
		else {
			if (!m_ui_host->is_dui()) {
				CommandFrameStyleContextMenu(cmd, id_base, point);
			}
		}
	}

	void GetQueueMetaSelection(metadb_handle_list_ref &out, CListControlQueue* l) {
		static_api_ptr_t<playlist_manager_v5> playlist_api;
		pfc::list_t<t_playback_queue_item> queue;
		playlist_api->queue_get_contents(queue);
		pfc::list_t<t_playback_queue_item> queue_sel_items;
		queue.get_items_mask(queue_sel_items, l->GetSelectionMask());

		for (auto w : queue_sel_items) {
			out.add_item(w.m_handle);
		}

		queue.remove_all();
		queue_sel_items.remove_all();
	}

	void CListControlQueue::CommandListItemContextMenu(unsigned cmd, unsigned id_base, CPoint point) {

		if (false/*cmd == ID_SHOW_COLUMN_HEADER + id_base*/) {

			ShowHideColumnHeader();
		}
		else if (cmd == ID_REMOVE + id_base) {

			DeleteSelected();
		}
		else if (cmd == ID_PREFERENCES + id_base) {

			static_api_ptr_t<ui_control>()->show_preferences(guid_preferences);
		}
		else if (	cmd == ID_PL_SEND_TO_CURRENT ||
							cmd == ID_PL_ADD_TO_CURRENT ||
							cmd == ID_PL_SEND_NEW) {

			static_api_ptr_t<playlist_manager_v5> playlist_api;
			metadb_handle_list mhl;

			GetQueueMetaSelection(mhl, this);

			switch (cmd) {
			case ID_PL_SEND_TO_CURRENT:
				playlist_api->activeplaylist_clear();
				playlist_api->activeplaylist_add_items(mhl, bit_array_true());
				break;
			case ID_PL_ADD_TO_CURRENT:
				playlist_api->activeplaylist_add_items(mhl, bit_array_true());
				break;
			case ID_PL_SEND_NEW:
				auto ndx = playlist_api->create_playlist_autoname();
				playlist_api->playlist_add_items(ndx, mhl, bit_array_true());
				break;
			}

			mhl.remove_all();
		}
		else if (cmd == ID_PROPERTIES) {
			GUID guid_ctx = pfc::guid_null;
			menu_helpers::name_to_guid_table menu_table;
			bool bf = menu_table.search("Properties", 10, guid_ctx);
			metadb_handle_list mhl;
			GetQueueMetaSelection(mhl, this);

			bool rrs = menu_helpers::run_command_context(guid_ctx, pfc::guid_null, mhl);
		}
		else if (cmd == ID_MOVE_TOP + id_base ||
				cmd == ID_MOVE_BOTTOM + id_base) {

			if (!GetSelectedCount()) {
				return;
			}

			pfc::list_t<t_size> selection;

			// Reordering result of the drag operation
			pfc::list_t<t_size> ordering;

			// Selection of the dragged items after reorder
			pfc::list_t<t_size> newSelection;

			auto cmask = GetItemCount();
			auto selMask = GetSelectionMask();

			auto n = selMask.find_first(true, 0, cmask);
			for (n; n < cmask; n = selMask.find_next(true, n, cmask)) {
				selection.add_item(n);
			}

			t_size new_index = 0;

			if (cmd == ID_MOVE_TOP + id_base) {
				new_index = 0;
				queue_helpers::queue_move_items_reordering(new_index, selection, newSelection, ordering);
			}
			else if (cmd == ID_MOVE_BOTTOM + id_base) {
				new_index = GetItemCount();
				queue_helpers::queue_move_items_reordering(new_index, selection, newSelection, ordering);
			}

			// When the refresh occurs in the listbox it tries to hold these selections
			SetSelectedIndices(newSelection);

			// Update the queue and let the new order propagate through queue events
			queue_helpers::queue_reorder(ordering);
		}
		else if (!m_ui_host->is_dui()) {
			//todo: check cmd before
			CommandFrameStyleContextMenu(cmd, id_base, point);
		}
	}

	void CListControlQueue::CommandContextMenu(CPoint screen_point, unsigned p_id, unsigned p_id_base) {

		ui_element_settings* settings;
		m_ui_host->get_configuration(&settings);

		CRect headerRect;

		//if (settings->m_show_column_header) {
			GetHeaderCtrl().GetClientRect(headerRect);
			GetHeaderCtrl().ClientToScreen(headerRect);
		//}

		if (headerRect.PtInRect(screen_point) || !settings->m_columns.get_count()) {

			// header command

			CommandHeaderContextMenu(p_id, p_id_base, screen_point);
		}
		else {

			// row area command

			CPoint pt(screen_point);
			ScreenToClient(&pt);

			// Test if we clicked an item

			auto ht = AccItemHitTest(pt);

			if (ht != ~0) {
				CommandListItemContextMenu(p_id, p_id_base, pt);
			}
			else {
				CommandListNoItemContextMenu(p_id, p_id_base, screen_point);
			}
		}
	}

	// Move items from the queue.
	// The move operation will propagate through queue notifications

	void CListControlQueue::MoveSelected(const size_t* order, size_t count) {

		TRACK_CALL_TEXT("CListControlBookmark::MoveSelected");

		pfc::reorder_t(my_data, order, my_data.get_count());
		pfc::reorder_t(my_queue, order, my_queue.get_count());
		queue_helpers::queue_reorder(order);
	}

	// Deletes selected items from the queue.
	// The deletion operation will propagate through queue notifications

	void CListControlQueue::DeleteSelected() {

		TRACK_CALL_TEXT("CListControlQueue::DeleteSelected");

		bit_array_bittable selMask = GetSelectionMask();
		UINT i = 0;

		// Add all selected items to this array
		int selectedCount = 0;

		auto firstSelected = GetFirstSelected();
		int focus_before_delete = GetFocusItem();
		size_t pos = firstSelected;

		while (pos < GetItemCount()) {
			// Calculate focused item after delete
			if (pos < focus_before_delete && focus_before_delete != -1) {
				focus_before_delete--;
			}
			//selMask.set(pos, true);
			selectedCount++;
			pos = selMask.find_next(true, pos, GetItemCount());
		}

		if (focus_before_delete == -1) {
			focus_before_delete = GetItemCount() - 1;
		}

		SelectNone();

		// Set focus rectangle
		int lastIndexAfterRemoval = GetItemCount() - selectedCount - 1;
		m_focused_item = min(lastIndexAfterRemoval, focus_before_delete);
		SetFocusItem(m_focused_item);

		DEBUG_PRINT << "CListControlQueue::DeleteSelected: SetItemState(focused) successful. Focus index was: " << m_focused_item;

		my_data.remove_mask(selMask);
		my_queue.remove_mask(selMask);

		queue_helpers::queue_remove_mask(selMask);
	}

	void CListControlQueue::SetSelectedIndices(const pfc::list_base_const_t<t_size>& p_in) {

		TRACK_CALL_TEXT("CListControlQueue::SetSelectedIndices");
		
		t_size itemCount = GetItemCount();
		t_size indicesCount = p_in.get_count();

		static_api_ptr_t<playlist_manager> playlist_api;
		pfc::list_t<t_playback_queue_item> queue;
		metadb_handle_list selected_handles;

		playlist_api->queue_get_contents(queue);

		// Unselect everything
		SelectNone();

		bit_array_bittable newSel(bit_array_false(), GetItemCount());

		for (t_size i = 0; i < indicesCount; i++) {

			t_size index = p_in[i];

			if (index >= 0 && index < itemCount) {

				newSel.set(index, true);
				selected_handles.add_item(queue[index].m_handle);

			}
		}

		SetSelection(bit_array_true(), newSel);

		// Inform other components of the selection
		SetSharedSelection(selected_handles);
	}
}

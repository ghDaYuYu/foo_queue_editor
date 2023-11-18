#include "stdafx.h"
#include "ui_element_base.h"

void ui_element_base::InvalidateWnd() {
	TRACK_CALL_TEXT("ui_element_base::InvalidateWnd");
	// Since listview control fills the whole space we might just as
	// well invalidate that instead of the container window
	::InvalidateRect(m_listview, NULL, TRUE);
}

BOOL ui_element_base::OnInitDialog(CWindow, LPARAM, HWND wnd /*= NULL*/) {
	TRACK_CALL_TEXT("ui_element_base::OnInitDialog");
	inited_successfully = false;
	CRect rect;
	if(wnd == NULL) {
		rect.bottom = rect.top = rect.left = rect.right = 0;
	} else {
		GetClientRect(wnd, &rect);
	}

#ifdef _DEBUG
	console::formatter() << "OnInitDialog. Column count is " << m_settings.m_columns.get_count();
#endif

	// wnd parameter 'overrides' get_wnd if it is set
	HWND parent = (wnd != NULL) ? wnd : get_wnd();

	int showColumnHeader = m_settings.m_show_column_header ? 0 : LVS_NOCOLUMNHEADER;

	// Let's create the listview controller "manually"
	m_listview.Create(parent, rect, CListViewCtrl::GetWndClassName(),
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT
		| showColumnHeader, WS_EX_WINDOWEDGE); 

	m_listview.SetExtendedListViewStyle( LVS_EX_FULLROWSELECT | 
		LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE  );

	t_size columnCount = m_settings.m_columns.get_count();
	bool atleast_one_valid_column = false;
	for(int i = columnCount-1; i >= 0 ; i--) {		
		long column_id = m_settings.m_columns[i].m_id;
		PFC_ASSERT(cfg_ui_columns.exists(column_id));

		// The column definition does not exists anymore, remove the column!
		if(!cfg_ui_columns.exists(column_id)) {
			m_settings.m_columns.remove_by_idx(i);
			continue;
		}

		pfc::map_t<long, ui_column_definition>::const_iterator column_settings = cfg_ui_columns.find(column_id);
		m_listview.AddColumnHelper(pfc::stringcvt::string_os_from_utf8(column_settings->m_value.m_name), 0, column_settings->m_value.m_alignment);
		atleast_one_valid_column = true;
	}

	// If no columns are present, we force the ui element use the first defined column
	if(!atleast_one_valid_column && (cfg_ui_columns.get_count() > 0)) {
		m_settings.m_columns.add_item(ui_column_settings(cfg_ui_columns.first()->m_key));
		m_listview.AddColumn(pfc::stringcvt::string_os_from_utf8(cfg_ui_columns.first()->m_value.m_name), 0);
		save_configuration();
	}
	

	m_listview.SetHost(this);

	//SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, WS_EX_STATICEDGE); // "Grey", DUI default
	//SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, 0); // No border"
	//SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, WS_EX_CLIENTEDGE); // "Sunken"
	SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, m_settings.m_border);

	// Update is needed to refresh border, see Remarks from http://msdn.microsoft.com/en-us/library/aa931583.aspx
	SetWindowPos(get_wnd(), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	
	// Call OnSize manually to tune column widths etc.
	OnSize(0, CSize(-1, -1));
	
	// Update fonts and colors
	RefreshVisuals();
	
	window_manager::AddWindow(this);

	// Update UI
	Refresh();

	// Relayout UI columns so if we have default config values
	// we will convert them to some proper values
	m_listview.RelayoutUIColumns();
	inited_successfully = m_listview.UpdateSettingsWidths();
	DEBUG_PRINT << "ui_element_base::OnInitDialog. Inited successfully?" << inited_successfully;

	// Save initial configuration which is read in the constructor (before this method!)
	save_configuration();

	//SetMsgHandled(FALSE);
	return TRUE;
}

void ui_element_base::OnSize(UINT nType, CSize size) {	
	TRACK_CALL_TEXT("ui_element_base::OnSize");
	CRect rect;
	
	NoRedrawScope tmp(m_listview.m_hWnd);

	if(size.cx < 0 || size.cy < 0) {
		m_listview.GetParent().GetClientRect(rect);
	} else {
		rect.left = 0;
		rect.right = size.cx;
		rect.top = 0;
		rect.bottom = size.cy;
	}

	// Resize control
	m_listview.MoveWindow(rect);

	// Resize columns
	m_listview.RelayoutUIColumns();

	DEBUG_PRINT << "ui_element_base::OnSize. Inited successfully?" << inited_successfully;
	if(!inited_successfully) {
		inited_successfully = m_listview.UpdateSettingsWidths();
		DEBUG_PRINT << "...trying again, OK?" << inited_successfully;
		save_configuration();
	}
	
#ifdef _DEBUG
	console::formatter() << "size cx" << size.cx << " cy " << size.cy;
#endif

}

BOOL ui_element_base::OnEraseBkgnd(CDCHandle dc) {
	// We don't want to draw background since the listview
	// fills the whole space anyway
	return TRUE;
}


void ui_element_base::OnFinalMessage(HWND hWnd){
	TRACK_CALL_TEXT("ui_element_base::OnFinalMessage");
#ifdef _DEBUG
	console::formatter() << "OnFinalMessage ui_element.";
#endif
	// WE DO *NOT* detach the list view
	// as some messages might still arrive after this one (OnFocus
	// at least), and if they use m_listview it might cause problems!

	/*
	OnFinalMessage ui_element.
	window_manager: Unregistering Window
	current size of registered windows: 2
	window_manager: Unregistered Window
	current size of registered windows: 1
	OnKillFocus
	OnSetFocus
	OnKillFocus
	*/

	// // Detach list view ctrl
	// m_listview.Detach();
	// Unregister queue updates
	window_manager::RemoveWindow(this);
}

void ui_element_base::on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook) {
	TRACK_CALL_TEXT("ui_element_base::on_changed_sorted");
	DEBUG_PRINT << "ui_element_base::on_changed_sorted";

	metadb_handle_list_cref metadbs = m_listview.GetAllMetadbs();
	t_size count = metadbs.get_count();
	
	for(t_size i = 0; i < count; i++) {
		metadb_handle_ptr item = metadbs[i];		
		t_size index = metadb_handle_list_helper::bsearch_by_pointer(p_items_sorted, item);
		if(index != pfc::infinite_size) {
			DEBUG_PRINT << "Found changed metadb handle, triggering repaint";
			// We found metadb that has changed, trigger repaint
			Refresh();
			break;
		}
	}
}

// window_manager calls this whenever queue changes
void ui_element_base::Refresh() {
	TRACK_CALL_TEXT("ui_element_base::Refresh");
	{
	NoRedrawScope tmp(m_listview.m_hWnd);
	m_listview.QueueRefresh();
	
	// Update size (might change because of appearance/disappearance of scrollbars)
	OnSize(0, CSize(-1, -1));
	}

	InvalidateWnd();
}

void ui_element_base::ColumnsChanged() {
	TRACK_CALL_TEXT("ui_element_base::ColumnsChanged");
	m_listview.RelayoutUIColumns();
}

void ui_element_base::RefreshVisuals() {
	TRACK_CALL_TEXT("ui_element_base::RefreshVisuals");
	
	// component border
	SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, m_settings.m_border);

	// Update is needed to refresh border, see Remarks from http://msdn.microsoft.com/en-us/library/aa931583.aspx
	SetWindowPos(get_wnd(), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}



void ui_element_base::get_configuration(ui_element_settings** configuration) {
	// we just return (already parsed) m_settings
	// this works because config is not changed outside of this control
	*configuration = &m_settings;
}

bool ui_element_base::is_popup() {
	TRACK_CALL_TEXT("ui_element_base::is_popup");
	// hack: UI element is hosted in a popup 
	// <-> parent of parent of the ui element is main window
	HWND wnd = get_wnd();
	PFC_ASSERT( wnd != NULL );
	PFC_ASSERT( ::IsWindow(wnd) );
	
	HWND parent = ::GetParent(wnd);
	PFC_ASSERT( parent != NULL );
	PFC_ASSERT( ::IsWindow(parent) );

	HWND parentOfParent = ::GetParent(parent);
	PFC_ASSERT( parentOfParent != NULL );
	PFC_ASSERT( ::IsWindow(parentOfParent) );

#ifdef _DEBUG
	char wndBuf[64];
	char parentBuf[64];
	char parparentBuf[64];
	char mainBuf[64];
	sprintf(wndBuf, "%p", wnd);
	sprintf(parentBuf, "%p", parent);
	sprintf(parparentBuf, "%p", parentOfParent);
	sprintf(mainBuf, "%p", core_api::get_main_window());
	DEBUG_PRINT << "ui_element_base::is_popup(): wnd:" << wndBuf << ". parent: " << parentBuf << ". parent of parent: " << parparentBuf << ". main: " << mainBuf;
#endif

	return parentOfParent == core_api::get_main_window();
}

void ui_element_base::close() {
	TRACK_CALL_TEXT("ui_element_base::close");
	if(!is_popup()) {
		DEBUG_PRINT << "Close() rejected since ui element is not hosted in a popup.";
		return;
	}

	HWND wnd = get_wnd();
	PFC_ASSERT( wnd != NULL );
	PFC_ASSERT( ::IsWindow(wnd) );
	
	HWND parent = ::GetParent(wnd);
	PFC_ASSERT( parent != NULL );
	PFC_ASSERT( ::IsWindow(parent) );

	BOOL ret = ::PostMessage(parent, WM_CLOSE, (WPARAM) 0, (LPARAM) 0);
	PFC_ASSERT( ret );
}
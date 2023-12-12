#include "stdafx.h"
#include "ui_element_base.h"

#include "style_manager_cui.h"

void ui_element_base::InvalidateWnd() {

	TRACK_CALL_TEXT("ui_element_base::InvalidateWnd");

	m_guiList.UpdateItemsAndHeaders(bit_array_true());
	m_guiList.Invalidate(true);

}

void ui_element_base::toggleHeader(HWND parent) {

	//window_manager::EnableUpdates(false);

	CWindow cWndParent(parent);
	CRect client; cWndParent.GetClientRect(client);

	CRect rcList;
	m_guiList.GetClientRect(&rcList);
	auto hwndCurrrent = cWndParent.GetDlgItem(IDC_QUEUELIST);

	CRect rcHeader;
	WINDOWPOS wPos = {};
	HDLAYOUT layout = { &rcHeader, &wPos };
	bool getlayout = m_guiList.GetHeaderCtrl().Layout(&layout);

	bool hadHeader = getlayout && wPos.cy > 0;

	if (hadHeader == cfg_show_header) {
		//nothing to do
		return;
	}

	//::EnableWindow(m_guiList.m_hWnd, false);

	m_guiList.DestroyWindow();

	HWND newwnd = CreateWindowEx(WS_EX_CLIENTEDGE, _T("listbox"),
		_T("queue listbox"),
		WS_CHILD | WS_VISIBLE | LBS_NOTIFY,
		client.left, client.top, client.right, client.bottom,
		cWndParent, 0,
		core_api::get_my_instance(), 0);

	on_style_change(false);

	cWndParent.SetFont(m_guiList.GetFont());

	m_guiList.CreateInDialog(cWndParent, IDC_QUEUELIST, newwnd);

	::SetWindowLongPtr(m_guiList.m_hWnd, GWL_EXSTYLE, 0);

	ClearDark();
	m_dark.AddDialogWithControls(parent);

	CRect rc_header;
	
	if (cfg_show_header) {
		m_guiList.InitializeHeaderCtrl(HDS_DRAGDROP | HDS_BUTTONS);
	}
	else {
		m_guiList.InitializeHeaderCtrl(HDS_HIDDEN);
	}
	
	m_guiList.SetPlaylistStyle();
	m_guiList.SetWantReturn(true);

	setBorderStyle();

	// todo: CUI test not working?
	// Update is needed to refresh border, see Remarks from http://msdn.microsoft.com/en-us/library/aa931583.aspx
	SetWindowPos(get_wnd(), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	on_style_change(true);
	InvalidateWnd();
}

BOOL ui_element_base::OnInitDialog(CWindow, LPARAM, HWND wnd /*= NULL*/) {
	TRACK_CALL_TEXT("ui_element_base::OnInitDialog");

	if (!is_dui()) {
		m_cust_stylemanager = new CuiStyleManager();
		m_cust_stylemanager->setChangeHandler([&](bool repaint) { this->on_style_change(repaint); });
	}

	inited_successfully = false;

	// wnd parameter 'overrides' get_wnd if it is set
	HWND parent = (wnd != NULL) ? wnd : get_wnd();

	m_guiList.CreateInDialog(parent, IDC_QUEUELIST);
	::SetWindowLongPtr(m_guiList.m_hWnd, GWL_EXSTYLE, 0);

	if (cfg_show_header) {
		m_guiList.InitializeHeaderCtrl(HDS_DRAGDROP | HDS_BUTTONS);
	}
	else {
		m_guiList.InitializeHeaderCtrl(HDS_HIDDEN);
	}

	m_dark.AddDialogWithControls(parent);

	m_guiList.SetPlaylistStyle();
	m_guiList.SetWantReturn(true);

	m_guiList.SetHost(this);
	m_guiList.BuildColumns(true);

	setBorderStyle();

	// Update is needed to refresh border, see Remarks from http://msdn.microsoft.com/en-us/library/aa931583.aspx
	SetWindowPos(get_wnd(), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	on_style_change(true);

	InvalidateWnd();

	window_manager::AddWindow(this);

	inited_successfully = m_guiList.GetColumnCount();

	DEBUG_PRINT << "ui_element_base::OnInitDialog. Inited successfully? " << inited_successfully;

	// Save initial configuration which is read in the constructor (before this method!)
	save_configuration();

	return TRUE;
}

void ui_element_base::OnSize(UINT nType, CSize size) {

	CRect rect;

	NoRedrawScopeEx tmp(m_guiList.m_hWnd);

	if(size.cx < 0 || size.cy < 0) {
		m_guiList/*.GetParent()*/.GetClientRect(rect);
	} else {
		rect.left = 0;
		rect.right = size.cx;
		rect.top = 0;
		rect.bottom = size.cy;
	}

	m_guiList.MoveWindow(rect);

}

BOOL ui_element_base::OnEraseBkgnd(CDCHandle dc) {
	// We don't want to draw background since the listview
	// fills the whole space anyway
	return TRUE;
}

void ui_element_base::OnFinalMessage(HWND hWnd){
	TRACK_CALL_TEXT("ui_element_base::OnFinalMessage");
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

	// Detach list view ctrl
	m_guiList.Detach();

	// Unregister queue updates
	window_manager::RemoveWindow(this);
}

void ui_element_base::on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook) {
	TRACK_CALL_TEXT("ui_element_base::on_changed_sorted");
	DEBUG_PRINT << "ui_element_base::on_changed_sorted";

	metadb_handle_list_cref metadbs = m_guiList.GetAllMetadbs();
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
		NoRedrawScopeEx tmp(m_guiList.m_hWnd);
		m_guiList.QueueRefresh();
	}
}

void ui_element_base::PrefColumnsChanged(bool reset) {
	TRACK_CALL_TEXT("ui_element_base::ColumnsChanged");

	//todo: rev params, column/field changes
	m_guiList.BuildColumns(!reset, reset);
	m_guiList.QueueReset();
	m_guiList.QueueRefresh();
	m_guiList.GetHeaderCtrl().Invalidate();
	InvalidateWnd();
}

void ui_element_base::PrefColumnsChanged(pfc::map_t<long, ui_column_definition> old_ui_col_defs) {
	TRACK_CALL_TEXT("ui_element_base::ColumnsChanged");

	m_guiList.ModColumns(old_ui_col_defs);
}

// list control to m_columns
void ui_element_base::GetLayout() {
	m_guiList.GetCurrentColumnLayout();
}

void ui_element_base::HideHeader() {
	toggleHeader(get_wnd());
}

void ui_element_base::RefreshVisuals() {
	TRACK_CALL_TEXT("ui_element_base::RefreshVisuals");

	setBorderStyle();

	// Update is needed to refresh border, see Remarks from http://msdn.microsoft.com/en-us/library/aa931583.aspx
	SetWindowPos(get_wnd(), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

// host to dlg
void ui_element_base::set_configuration() {
	//..
}

//use internally to write changes to settings (headers changes...)
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

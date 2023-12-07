#include "stdafx.h"
#include "default_ui_hacks.h"
#include "config.h"
#include "style_manager_dui.h"
#include "dui_element.h"

dui_element::dui_element(ui_element_config::ptr config,ui_element_instance_callback_ptr p_callback) :
		m_callback(p_callback), m_config(config) {

	m_cust_stylemanager = new DuiStyleManager(m_callback);
	m_cust_stylemanager->setChangeHandler([&](bool) { this->on_style_change(); });

	TRACK_CALL_TEXT("dui_element::dui_element");

	// load current settings to m_settings
	ui_element_config_parser parser(m_config);
	parser >> m_settings;

	m_guiList.SetCallback(p_callback);
}

void dui_element::edit_mode_context_menu_build(const POINT & p_point,bool p_fromkeyboard,HMENU p_menu,unsigned p_id_base) { 
	TRACK_CALL_TEXT("dui_element::edit_mode_context_menu_build");
	CMenuHandle menu(p_menu);

	m_guiList.BuildContextMenu(p_point, menu, p_id_base);
}

void dui_element::edit_mode_context_menu_command(const POINT & p_point,bool p_fromkeyboard,unsigned p_id,unsigned p_id_base) {
	TRACK_CALL_TEXT("dui_element::edit_mode_context_menu_command");

	m_guiList.CommandContextMenu(p_point, p_id, p_id_base);
}

bool dui_element::edit_mode_context_menu_get_focus_point(POINT & p_point) {
	TRACK_CALL_TEXT("dui_element::edit_mode_context_menu_get_focus_point");

	p_point = m_guiList.GetContextMenuPoint(p_point);
	CPoint ptInvalid(-1, -1);
	if (CPoint(p_point) == ptInvalid) {
		//no items in list
		::GetCursorPos(&p_point);
	}
	return true;
}

void dui_element::save_configuration() {
	TRACK_CALL_TEXT("dui_element::save_configuration");
	ui_element_config_builder config_builder;
	config_builder << m_settings;
	m_config = config_builder.finish(g_get_guid());

}

void dui_element::initialize_window(HWND parent) {
	TRACK_CALL_TEXT("dui_element::initialize_window");
	Create(parent);
}

HWND dui_element::get_wnd() {
	return *this;
}

void dui_element::HideHeader() {

	toggleHeader(m_hWnd);
	return;
}

void dui_element::RefreshVisuals() {
	//todo: remove
	return;
}

void dui_element::notify(const GUID & p_what, t_size p_param1, const void * p_param2, t_size p_param2size) {
	TRACK_CALL_TEXT("dui_element::notify");

	if (p_what == ui_element_notify_colors_changed || p_what == ui_element_notify_font_changed) {

		m_cust_stylemanager->onChange();
	}
}

void dui_element::OnFinalMessage(HWND hWnd){
	TRACK_CALL_TEXT("dui_element::OnFinalMessage");
	ui_element_base::OnFinalMessage(hWnd);

	// Let the base class(es) handle this message...
	SetMsgHandled(FALSE);
}

bool dui_element::is_dui(){
	return true;
}

static service_factory_single_t<ui_element_myimpl> g_ui_element_myimpl_factory;

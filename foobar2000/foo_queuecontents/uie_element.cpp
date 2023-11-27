#include "stdafx.h"
#include "uie_element.h"

const GUID uie_element::extension_guid = uie_guid;

HWND uie_element::get_wnd() {

	return m_guiList.GetParent();
}

HWND uie_element::get_wnd() const {

	return m_guiList.GetParent();
}

bool uie_element::is_dui() {
	return false;
}

void uie_element::RefreshVisuals() {
	TRACK_CALL_TEXT("uie_element::RefreshVisuals");
	ui_element_base::RefreshVisuals();
	console::formatter() << "Refresh visuals";
	columns_ui::colours::helper vis = columns_ui::colours::helper::helper(uie_colours_client_guid);
	
	columns_ui::fonts::helper fonts = columns_ui::fonts::helper::helper(uie_font_client_guid);

	m_guiList.SetFont(fonts.get_font());

	//todo: rev
	/*m_listview.SetColors(vis.get_colour(columns_ui::colours::colour_background),
		vis.get_colour(columns_ui::colours::colour_selection_background),
		vis.get_colour(columns_ui::colours::colour_text),
		vis.get_colour(columns_ui::colours::colour_active_item_frame),
		vis.get_colour(columns_ui::colours::colour_active_item_frame),
		vis.get_colour(columns_ui::colours::colour_selection_text));*/

	InvalidateWnd();
}

void uie_element::save_configuration() {
	// In columns ui we cannot force saving of settings...So we do nothing!
	// CUI calls get_config when its ready to save the settings
}

LRESULT uie_element::on_message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TRACK_CALL_TEXT("uie_element::on_message");
	BOOL bRet = TRUE;
	LRESULT lResult = 0;
	switch(uMsg)
	{
	case WM_CREATE:
		bRet = OnInitDialog(NULL, NULL, hWnd);
		PFC_ASSERT(hWnd == get_wnd());
		break;
	case WM_GETMINMAXINFO:
		break;
	case WM_SIZE:
		OnSize((UINT)wParam, CSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		break;
	case WM_ERASEBKGND:
		bRet = OnEraseBkgnd((HDC)wParam);
		break;
	case WM_DESTROY:
		OnFinalMessage(hWnd);
		// Do Default action, too
		bRet = FALSE;
		break;
	default:
		// We didn't handle the message
		bRet = FALSE;
	}

	// We do "CHAIN_MSG_MAP_MEMBER" manually here
	if(!bRet) {
		//DEBUG_PRINT << "uie_element did not know what to do with the message " << pfc::format_hex(uMsg)
		//	<< ". Passing it to m_listview";
		if(uMsg == WM_NOTIFY) {
			//DEBUG_PRINT << "uMsg was WM_NOTIFY, lParam->code:" << ((LPNMHDR)lParam)->code;
		}

		//todo: rev
		//bRet = m_guiList.ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult);

		//if(bRet) {
		//	DEBUG_PRINT << "m_listview handled the message";
		//} else {
		//	DEBUG_PRINT << "m_listview did NOT handle the message";
		//}
	}	

	if(!bRet) {
		if(uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) {
			DEBUG_PRINT << "Message is WM_KEYDOWN or WM_SYSKEYDOWN, Checking if foobar wants it.";
			if(get_host()->get_keyboard_shortcuts_enabled() && static_api_ptr_t<keyboard_shortcut_manager>()->on_keydown_auto(wParam)) {
				return 0;
			}
		}
		//DEBUG_PRINT << "Final try to handle the message: DefWindowProc";
		lResult = uDefWindowProc(hWnd, uMsg, wParam, lParam);		
	}

	return lResult;
}

ui_extension::window_factory<uie_element> blah;
columns_ui::colours::client::factory<queuecontents_uie_colours_client> blah2;
columns_ui::fonts::client::factory<queuecontents_uie_fonts_client> blah3;
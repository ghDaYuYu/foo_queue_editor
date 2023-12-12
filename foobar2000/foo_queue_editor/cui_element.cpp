#include "stdafx.h"
#include "cui_element.h"

const GUID cui_element::extension_guid = cui_guid;

HWND cui_element::get_wnd() {

	return m_guiList.GetParent();
}

HWND cui_element::get_wnd() const {

	return m_guiList.GetParent();
}

bool cui_element::is_dui() {
	return false;
}

void cui_element::HideHeader() {

	toggleHeader(get_wnd());
}

void cui_element::RefreshVisuals() {
	TRACK_CALL_TEXT("cui_element::RefreshVisuals");
	InvalidateWnd();
}

void cui_element::save_configuration() {
	// In columns ui we cannot force saving of settings...So we do nothing!
	// CUI calls get_config when its ready to save the settings
}

LRESULT cui_element::on_message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TRACK_CALL_TEXT("cui_element::on_message");
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
		//DEBUG_PRINT << "cui_element did not know what to do with the message " << pfc::format_hex(uMsg)
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

ui_extension::window_factory<cui_element> blah;
columns_ui::colours::client::factory<queue_editor_cui_colours_client> blah2;
columns_ui::fonts::client::factory<queue_editor_cui_fonts_client> blah3;
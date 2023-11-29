#pragma once

#include <helpers/atl-misc.h>
#include <helpers/DarkMode.h>

#include "ui_element_configuration.h"
#include "ui_element_host.h"
#include "window_manager.h"
#include "resource.h"

#include "queue_list_control.h"
#include "queue_helpers.h"

class ui_element_base : public ui_element_host,
	public metadb_io_callback_dynamic_impl_base, private dlg::ILOD_QueueSource {

public:

	ui_element_base() : m_guiList(this, false) {}
	// what to do when window_manager requests refresh
	virtual void Refresh();
	virtual void GetLayout();
	// when columns might have been changed
	virtual void PrefColumnsChanged(bool reset = false);
	virtual void PrefColumnsChanged(pfc::map_t<long, ui_column_definition> modded_ui_col_defs);
	
	// persist UI element configuration
	virtual void set_configuration();
	virtual void get_configuration(ui_element_settings** configuration);
	virtual HWND get_wnd() = 0;
	virtual void InvalidateWnd();

	virtual bool is_popup();
	virtual void close();

	virtual bool is_dui() = 0; // True with DUI, False with CUI

	// metadb_io_callback_dynamic
	virtual void on_changed_sorted(metadb_handle_list_cref p_items_sorted, bool p_fromhook);

protected:
	// Added one extra parameter, wnd, for determining size of the list view
	// It is also used for the parent of the list view
	// If wnd is NULL get_wnd() is used instead

	virtual BOOL OnInitDialog(CWindow, LPARAM, HWND wnd = NULL);
	virtual void OnSize(UINT nType, CSize size);
	virtual BOOL OnEraseBkgnd(CDCHandle);
	
	// Implementors: call listview's SetColors and SetFont.
	// Base implementation updates component border.
	virtual void RefreshVisuals();
	// when window dies
	virtual void OnFinalMessage(HWND /*hWnd*/);

	//todo: style manager
	void setBorderStyle() {
		if (is_dui()) {
			::SetWindowLongPtr(/*get_wnd()*/m_guiList, GWL_EXSTYLE, 0);
		}
		else {
			::SetWindowLongPtr(/*get_wnd()*/m_guiList, GWL_EXSTYLE, m_settings.m_border);
		}
	}

	dlg::CListControlQueue m_guiList;

	ui_element_settings m_settings;

	bool inited_successfully;

private:

	fb2k::CDarkModeHooks m_dark;
};

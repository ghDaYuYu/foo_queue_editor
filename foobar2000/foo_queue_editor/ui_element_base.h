#pragma once

#include <helpers/atl-misc.h>
#include <helpers/DarkMode.h>

#include "ui_element_configuration.h"
#include "ui_element_host.h"

#include "resource.h"

#include "queue_list_control.h"
#include "queue_helpers.h"

#include "style_manager.h"

class ui_element_base : public ui_element_host,
	public metadb_io_callback_dynamic_impl_base, private dlg::ILOD_QueueSource {

public:

	ui_element_base() : m_guiList(this, false) {
		//..
	}

	~ui_element_base() {
		if (m_cust_stylemanager) {
			delete m_cust_stylemanager;
		}
	}

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
	virtual void HideHeader();
	
	virtual void RefreshVisuals();
	// when window dies
	virtual void OnFinalMessage(HWND /*hWnd*/);

	//todo: style manager
	void toggleHeader(HWND parent);

	void on_style_change(bool repaint) {

		if (m_guiList.IsHeaderEnabled()) {
			const LOGFONT lfh = m_cust_stylemanager->getTitleFont();
			HFONT hOldFontHeader = m_guiList.GetHeaderCtrl().GetFont();
			HFONT hFontHeader = CreateFontIndirect(&lfh);
			m_guiList.SetHeaderFont(hFontHeader);
			//todo: rev already managed (CFont)?
			if (hOldFontHeader != hFontHeader) {
				//todo: leak? rev crash show/hide header...
				//::DeleteObject(hOldFontHeader);
			}
		}

		const LOGFONT lf = m_cust_stylemanager->getListFont();
		HFONT hOldFont = m_guiList.GetFont();
		HFONT hFont = CreateFontIndirect(&lf);
		m_guiList.SetFont(hFont);

		if (hOldFont != hFont) {
			//todo: leak or managed CFontHandle?
			//::DeleteObject(hOldFont);
		}

		if (repaint) {

			m_guiList.UpdateItemsAndHeaders(bit_array_true());
			m_guiList.Invalidate(true);
		}
	}

	void setBorderStyle() {

		if (is_dui()) {
			::SetWindowLongPtr(/*get_wnd()*/m_guiList, GWL_EXSTYLE, 0);
		}
		else {
			::SetWindowLongPtr(get_wnd(), GWL_EXSTYLE, m_settings.m_border);
		}

		// todo: rev. Update is needed to refresh border, see Remarks from http://msdn.microsoft.com/en-us/library/aa931583.aspx
		SetWindowPos(get_wnd(), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	void ClearDark() { m_dark.clear(); }

	dlg::CListControlQueue m_guiList;

	ui_element_settings m_settings;

	bool inited_successfully;

	StyleManager* m_cust_stylemanager = nullptr;

private:

	fb2k::CDarkModeHooks m_dark;
};

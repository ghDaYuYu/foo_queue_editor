#pragma once
#include "config.h"
#include "queuecontents_lock.h"
#include "PropertyGrid.h"
#include "HPropertyHelpers.h"
#include "window_manager.h"
#include "CPropertyEditWindowWithAutomaticSave.h"
#include "CustomPropertyGrid.h"
#include "queuecontents_lock.h"

#include "guids.h"
#include "helpers\atl-misc.h"

#define NAME_COLUMN_TEXT "Name"
#define PATTERN_COLUMN_TEXT "Pattern"
#define ALIGNMENT_COLUMN_TEXT "Alignment"

#define ALIGNMENT_LEFT "Left"
#define ALIGNMENT_CENTER "Center"
#define ALIGNMENT_RIGHT "Right"

class CMyPreferences : public CDialogImpl<CMyPreferences>, 
	public preferences_page_instance {
public:
	//Constructor - invoked by preferences_page_impl helpers - don't do Create() in here, preferences_page_impl does this for us
	CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}
	~CMyPreferences();
	//Note that we don't bother doing anything regarding destruction of our class.
	//The host ensures that our dialog is destroyed first, then the last reference to our preferences_page_instance object is released, causing our object to be deleted.


	//dialog resource ID
	enum {IDD = IDD_PREFERENCES};
	// preferences_page_instance methods (not all of them - get_wnd() is supplied by preferences_page_impl helpers)
	t_uint32 get_state();
	void apply();
	void reset();

	//WTL message map
	BEGIN_MSG_MAP(CMyPreferences)
		MSG_WM_INITDIALOG(OnInitDialog)		
		COMMAND_HANDLER_EX(IDC_UI_FORMAT, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER_EX(IDC_PLAYLIST_ENABLED, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER_EX(IDC_PLAYLIST_NAME, EN_CHANGE, OnEditChange)		
		COMMAND_HANDLER(IDC_PLAYLIST_ENABLED, BN_CLICKED, OnBnClickedPlaylistEnabled)
		COMMAND_HANDLER(IDC_BUTTON_ADD_COLUMN, BN_CLICKED, OnBnClickedButtonAddColumn)
		COMMAND_HANDLER(IDC_BUTTON_REMOVE_COLUMN, BN_CLICKED, OnBnClickedButtonRemoveColumn)
		COMMAND_HANDLER(IDC_BUTTON_SYNTAX_HELP, BN_CLICKED, OnBnClickedButtonSyntaxHelp)
		//todo NOTIFY_CODE_HANDLER(PIN_ITEMCHANGED, OnUIColumnChanged);
		REFLECT_NOTIFICATIONS()			
	END_MSG_MAP()
private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnEditChange(UINT, int, CWindow);
	bool HasChanged();
	void OnChanged();
	LRESULT OnUIColumnChanged(int idCtrl, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnColumnGridClick(int idCtrl, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

	void UpdateColumnDefinitionsFromCfg(const pfc::map_t<long, ui_column_definition>& config = cfg_ui_columns);

	const preferences_page_callback::ptr m_callback;
	CCustomPropertyGrid m_grid;
	LPCTSTR m_alignment_values[4];
	int m_alignment_cfg_values[4];
	bool m_columns_dirty;
	pfc::ptr_list m_guids;
public:
	LRESULT OnBnClickedPlaylistEnabled(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonAddColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonRemoveColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonSyntaxHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};


class preferences_page_myimpl : public preferences_page_impl<CMyPreferences> {
	// preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
public:
	const char * get_name() {
		return COMPONENT_NAME_HC;
	}
	
	GUID get_guid() {
		return guid_queuecontents_preferences;
	}
	
	GUID get_parent_guid() {
		return guid_tools;
	}

	bool get_help_url (pfc::string_base &p_out) {
		p_out = "http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Components/Queue_Contents_Editor_(foo_queuecontents)";
		return true;
	}
};

#include "stdafx.h"
#include "preferences.h"

CMyPreferences::~CMyPreferences() {
	m_guids.free_all();
}

BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM) {

	m_alignment_values[0] = _T(ALIGNMENT_LEFT);
	m_alignment_values[1] = _T(ALIGNMENT_CENTER);
	m_alignment_values[2] = _T(ALIGNMENT_RIGHT);
	m_alignment_values[3] = NULL;
	
	// matching values of above
	m_alignment_cfg_values[0] = COLUMN_ALIGNMENT_LEFT;
	m_alignment_cfg_values[1] = COLUMN_ALIGNMENT_CENTER;
	m_alignment_cfg_values[2] = COLUMN_ALIGNMENT_RIGHT;
	m_alignment_cfg_values[3] = -1;
	
	m_grid.SubclassWindow(GetDlgItem(IDC_LIST_COLUMN_FORMATS));

	m_grid.InsertColumn(0, _T(NAME_COLUMN_TEXT), LVCFMT_LEFT, 165, 0);
	m_grid.InsertColumn(1, _T(PATTERN_COLUMN_TEXT), LVCFMT_LEFT, 235, 0);
	m_grid.InsertColumn(2, _T(ALIGNMENT_COLUMN_TEXT), LVCFMT_LEFT, 60, 0);

	m_grid.SetExtendedGridStyle(PGS_EX_SINGLECLICKEDIT);

	


	UpdateColumnDefinitionsFromCfg();

	
	m_columns_dirty = false;

	CheckDlgButton(IDC_PLAYLIST_ENABLED, cfg_playlist_enabled ? BST_CHECKED : BST_UNCHECKED);
	uSetDlgItemText(*this, IDC_PLAYLIST_NAME, cfg_playlist_name);
	// Enable/Disable playlist name control
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(cfg_playlist_enabled);
	return TRUE;
}

void CMyPreferences::UpdateColumnDefinitionsFromCfg(const pfc::map_t<long, ui_column_definition>& config /*= cfg_ui_columns*/) {
	
	if(m_grid.GetColumnCount() > 0) {
		// DeleteAllItems does not work (probably bug
		// in PropertyGrid) so we use this little hack
		while(m_grid.GetItemCount() > 0)
			m_grid.DeleteItem(0);

		m_guids.free_all();
	}
	
	int i = 0;
	for(pfc::map_t<long, ui_column_definition>::const_iterator iter = config.first();
		iter.is_valid(); iter++) {
		HPROPERTY name = PropCreateSimpleWithAutosave(_T(""), pfc::stringcvt::string_os_from_utf8(iter->m_value.m_name));
		HPROPERTY pattern = PropCreateSimpleWithAutosave(_T(""), pfc::stringcvt::string_os_from_utf8(iter->m_value.m_pattern));

		// We find the item to select...
		t_size list_value_index;
		for(list_value_index = 0; m_alignment_cfg_values[list_value_index] != -1; list_value_index++) {
			DEBUG_PRINT << "list_value_index: " << list_value_index;
			if(m_alignment_cfg_values[list_value_index] == iter->m_value.m_alignment) {
				DEBUG_PRINT << "Alignment cfg index: " << list_value_index;
				break;
			}
		}


		HPROPERTY alignment = PropCreateList(_T(""), m_alignment_values,  list_value_index);
#ifdef _DEBUG
		console::formatter() << "Reading column definitions from configuration:";
		console::formatter() << NAME_COLUMN_TEXT << ": "<< iter->m_value.m_name;
		console::formatter() << PATTERN_COLUMN_TEXT  << ": "<< iter->m_value.m_pattern;
		console::formatter() << ALIGNMENT_COLUMN_TEXT  << ": "<< iter->m_value.m_alignment;
		console::formatter() << "ID: " << iter->m_key;
#endif
		m_grid.InsertItem(i, name);
		m_grid.SetSubItem(i, 1, pattern);
		m_grid.SetSubItem(i, 2, alignment);
		long id = iter->m_key;

		m_grid.SetItemData(name, id);
#ifdef _DEBUG			
		PFC_ASSERT(((long)m_grid.GetItemData(m_grid.GetProperty(i,0))) == id);
#endif
		i++;
	}
}


LRESULT CMyPreferences::OnUIColumnChanged(int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMPROPERTYITEM pnpi  = reinterpret_cast<LPNMPROPERTYITEM>( pnmh );
	ATLTRACE(_T("OnUIColumnChanged - Ctrl: %d\n"), idCtrl); idCtrl;
	m_columns_dirty = true;
#ifdef _DEBUG
	console::formatter() << "Column definitions changed: dirty!";
#endif

	OnChanged();
	if( pnpi->prop==NULL ) return 0;	

	return 0;
}

void CMyPreferences::OnEditChange(UINT, int, CWindow) {
	bool playlist_enabled = (IsDlgButtonChecked(IDC_PLAYLIST_ENABLED) == BST_CHECKED);
	// Enable/Disable playlist name control
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(playlist_enabled);

	OnChanged();
}

t_uint32 CMyPreferences::get_state() {
	t_uint32 state = preferences_state::resettable;
	if (HasChanged()) state |= preferences_state::changed;
	return state;
}

void CMyPreferences::reset() {
	CheckDlgButton(IDC_PLAYLIST_ENABLED, default_cfg_playlist_enabled ? BST_CHECKED : BST_UNCHECKED);
	uSetDlgItemText(*this, IDC_PLAYLIST_NAME, default_cfg_playlist_name);
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(default_cfg_playlist_enabled);
	
	pfc::map_t<long, ui_column_definition> column_list;
	map_utils::fill_map_with_values(column_list, default_cfg_ui_columns);	
	
	UpdateColumnDefinitionsFromCfg(column_list);
	

	m_columns_dirty = true;
	OnChanged();
}

void CMyPreferences::apply() {
	
	pfc::string8 playlist_name;
	cfg_playlist_enabled = IsDlgButtonChecked(IDC_PLAYLIST_ENABLED) == BST_CHECKED;
	playlist_name = uGetDlgItemText(m_hWnd, IDC_PLAYLIST_NAME);

	// Rename current queue playlist if necessary
	if(cfg_playlist_enabled && playlist_name != cfg_playlist_name){
		static_api_ptr_t<playlist_manager> playlist_api;
		t_size plIndex = queuecontents_lock::find_playlist();
		if(plIndex != pfc::infinite_size) {
			playlist_api->playlist_rename(plIndex, playlist_name.get_ptr(), playlist_name.get_length());
		}
	}

	if(cfg_playlist_enabled) {
		queuecontents_lock::install_lock();
	} else {
		queuecontents_lock::uninstall_lock();
	}

	if(m_columns_dirty) {
		// apply column settings
		t_size column_count = m_grid.GetItemCount();
		cfg_ui_columns.remove_all();
		for(t_size i = 0; i < column_count; i++) {
			ui_column_definition definition;

			HPROPERTY prop = m_grid.GetProperty(i, 0);
			HPropertyHelpers::GetDisplayValue(prop, definition.m_name);			

			prop = m_grid.GetProperty(i, 1);
			HPropertyHelpers::GetDisplayValue(prop, definition.m_pattern);						
			
			CComVariant value;			
			prop = m_grid.GetProperty(i, 2);
			m_grid.GetItemValue(prop, &value);			
			definition.m_alignment = m_alignment_cfg_values[value.intVal];

			long id = (long)m_grid.GetItemData(m_grid.GetProperty(i, 0));
	
#ifdef _DEBUG
			console::formatter() << "Updating cfg column definition with ID " <<  ((long)m_grid.GetItemData(m_grid.GetProperty(i, 0)));
#endif
			cfg_ui_columns.set(id, definition);
		}

		window_manager::UIColumnsChanged();
		window_manager::GlobalRefresh();
		
	}

	cfg_playlist_name = playlist_name;

	m_columns_dirty = false;
	OnChanged(); //our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
}

bool CMyPreferences::HasChanged() {
	pfc::string8 playlist_name;
	pfc::string8 ui_format;
	// make sure we have normalized booleans for direct comparison with == and !=
	bool playlist_enabled = (IsDlgButtonChecked(IDC_PLAYLIST_ENABLED) == BST_CHECKED) != 0;	
	bool playlist_enabled_cfg = cfg_playlist_enabled != 0;
	playlist_name = uGetDlgItemText(m_hWnd, IDC_PLAYLIST_NAME);
	
	
	//returns whether our dialog content is different from the current configuration (whether the apply button should be enabled or not)
	return (playlist_enabled != playlist_enabled_cfg )
		|| (playlist_name != cfg_playlist_name)
		|| m_columns_dirty;
}
void CMyPreferences::OnChanged() {
	//tell the host that our state has changed to enable/disable the apply button appropriately.
	m_callback->on_state_changed();
}


static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;


LRESULT CMyPreferences::OnBnClickedPlaylistEnabled(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Enable/Disable playlist name control
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(IsDlgButtonChecked(IDC_PLAYLIST_ENABLED) == BST_CHECKED);
	OnChanged();
	return 0;
}


LRESULT CMyPreferences::OnBnClickedButtonAddColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	HPROPERTY name = PropCreateSimple(_T(""), _T(NAME_COLUMN_TEXT));
	HPROPERTY pattern = PropCreateSimple(_T(""), _T(PATTERN_COLUMN_TEXT));
	HPROPERTY alignment = PropCreateList(_T(""), m_alignment_values, COLUMN_ALIGNMENT_LEFT);
	
	// Generate new id so it's 1 larger than any of the used columns.
	long cfg_max_id = LONG_MAX;
	int current_count = m_grid.GetItemCount();
	if(cfg_ui_columns.query_nearest_ptr<true, true, long>(cfg_max_id) == NULL) {
		cfg_max_id = -1;
	}

	for(int i = 0; i < current_count; i++) {
		long id = (long)m_grid.GetItemData(m_grid.GetProperty(i,0));
		if(id > cfg_max_id)
			cfg_max_id = id;
	}

	
	long new_id = max(cfg_max_id, 0) + 1;

	int i = m_grid.InsertItem(-1, name);
	m_grid.SetSubItem(i, 1, pattern);
	m_grid.SetSubItem(i, 2, alignment);
	m_grid.SelectItem(i);
	m_grid.SetItemData(name, new_id);

#ifdef _DEBUG
	console::formatter() << "New column definition with ID " << ((long)m_grid.GetItemData(m_grid.GetProperty(i,0))) << " added";
	console::formatter() << "Column definition added: dirty!";
#endif

	m_columns_dirty = true;
	OnChanged();
	return 0;
}


LRESULT CMyPreferences::OnBnClickedButtonRemoveColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int index = m_grid.GetSelectedIndex();
	if(index >= 0) 
		m_grid.DeleteItem(index);

	m_columns_dirty = true;
	OnChanged();

	return 0;
}


LRESULT CMyPreferences::OnBnClickedButtonSyntaxHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShellExecute(NULL, _T("open"), _T(TITLEFORMAT_WIKIPAGEURL),
                NULL, NULL, SW_SHOWNORMAL);
	
	return 0;
}

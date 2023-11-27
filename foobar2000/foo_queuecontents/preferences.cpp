#include "stdafx.h"
#include <algorithm>
#include "yesno_dialog.h"
#include "preferences.h"

using namespace cfg_var_legacy;

CMyPreferences::~CMyPreferences() {

	m_static_header.Detach();
}

BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM) {

	HWND wndStHeader = uGetDlgItem(IDC_STATIC_HEADER);
	m_static_header.SubclassWindow(wndStHeader);
	m_static_header.PaintHeader();

	m_field_list.CreateInDialog(*this, IDC_FIELD_LIST);
	m_field_list.InitializeHeaderCtrl(HDS_FLAT);

	m_field_list.AddColumn(NAME_COLUMN_TEXT, 165);
	m_field_list.AddColumn(PATTERN_COLUMN_TEXT, 235);
	m_field_list.AddColumn(ALIGNMENT_COLUMN_TEXT, 60);

	m_dark.AddDialogWithControls(m_hWnd);

	m_columns_dirty = false;

	CheckDlgButton(IDC_PLAYLIST_ENABLED, cfg_playlist_enabled ? BST_CHECKED : BST_UNCHECKED);
	uSetDlgItemText(*this, IDC_PLAYLIST_NAME, cfg_playlist_name);

	//todo
	const pfc::string8 tt_obs = "Note:\r\n%queue_index%, %queue_total%, " \
		"%list_index% and %list_total% work here. Use %playlist_name% for the playlist of the queued item.";
	uSetDlgItemText(m_hWnd, IDC_TT_OBS, tt_obs);

	// Enable/Disable playlist name control
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(cfg_playlist_enabled);

	return TRUE;
}

void CMyPreferences::OnContextMenu(CWindow wnd, CPoint point) {

	if (wnd == m_field_list) {

		pfc::bit_array_bittable selmask = m_field_list.GetSelectionMask();

		size_t curr_alignment = SIZE_MAX;

		size_t csel = m_field_list.GetSelectedCount();
		size_t isel = selmask.find_first(true, 0, data.size());

		bool bsingle_sel = (csel == 1);

		if (bsingle_sel) {
			const field_t* entry = &data.at(isel);
			alignment = entry->alignment;
		}
		else {
			return;
		}

		CMenu menu;
		WIN32_OP(menu.CreatePopupMenu());
		menu.AppendMenu(MF_STRING | (bsingle_sel && alignment == HDF_LEFT ? MF_CHECKED : 0), (UINT_PTR)HDF_LEFT + 1, TEXT(ALIGNMENT_LEFT"\tL"));
		menu.AppendMenu(MF_STRING | (bsingle_sel && alignment == HDF_CENTER ? MF_CHECKED : 0), (UINT_PTR)HDF_CENTER +1 , TEXT(ALIGNMENT_CENTER"\tC"));
		menu.AppendMenu(MF_STRING | (bsingle_sel && alignment == HDF_RIGHT ? MF_CHECKED : 0), (UINT_PTR)HDF_RIGHT + 1, TEXT(ALIGNMENT_RIGHT"\tR"));

		CMenuDescriptionMap receiver(*this);
		//todo
		receiver.Set(HDF_LEFT, "");
		receiver.Set(HDF_CENTER, "");
		receiver.Set(HDF_RIGHT, "");

		int cmd = menu.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, receiver);

		if (cmd) {

			--cmd;

			//todo
			listSetEditField(nullptr, isel, /*aligment subitem*/2, std::to_string(cmd).c_str());
			m_field_list.ReloadItem(isel);
		}
	}

	return;
}

void CMyPreferences::UpdateColumnDefinitionsFromCfg(const pfc::map_t<long, ui_column_definition>& config /*= cfg_ui_columns*/) {
	//..
}

void CMyPreferences::OnEditChange(UINT, int, CWindow) {
	bool playlist_enabled = (IsDlgButtonChecked(IDC_PLAYLIST_ENABLED) == BST_CHECKED);
	// Enable/Disable playlist name control
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(playlist_enabled);

	OnChanged();
}

t_uint32 CMyPreferences::get_state() {
	t_uint32 state = preferences_state::resettable | preferences_state::dark_mode_supported;
	if (HasChanged()) state |= preferences_state::changed;
	return state;
}

void CMyPreferences::reset() {

	CYesNoApiDialog yndlg;
	if (!yndlg.query(m_hWnd, { "Reset", "Reset Queue Editor settings?" })) {
		return;
	}

	//todo
	m_columns_dirty = false;

	size_t old_size = data.size();
	cfg_ui_columns.remove_all(); //(1)
	map_utils::fill_map_with_values(cfg_ui_columns, default_cfg_ui_columns);
	init_data(); //(2)

	//refresh data/scroll bar
	size_t new_size = data.size();
	if (old_size > new_size) {
		bit_array_bittable delMask(bit_array_false(), old_size);
		for (auto n = new_size; n < old_size; n++) {
			delMask.set(n, true);
		}
		m_field_list.OnItemsRemoved(delMask, old_size);
	}
	else if (new_size > old_size) {
		m_field_list.OnItemsInserted(0, new_size - old_size, false);
	}
	else {
		m_field_list.ReloadData();
		m_field_list.ReloadItems(bit_array_true());
	}

	CheckDlgButton(IDC_PLAYLIST_ENABLED, default_cfg_playlist_enabled ? BST_CHECKED : BST_UNCHECKED);
	uSetDlgItemText(*this, IDC_PLAYLIST_NAME, default_cfg_playlist_name);
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(default_cfg_playlist_enabled);
	
	pfc::map_t<long, ui_column_definition> column_list;
	map_utils::fill_map_with_values(column_list, default_cfg_ui_columns);	

	window_manager::UIColumnsChanged(true);
	window_manager::GlobalRefresh();

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

	if (m_columns_dirty) {

		// apply column settings

		long max_id = 0;
		if (data.size()) {
			max_id = std::max_element(data.begin(),
			data.end(),
			[](const auto& a, const auto& b) {
				return a.id < b.id;
			})->id;
		}

		for (auto rec_it = data.begin(); rec_it != data.end(); rec_it++) {
			//todo: also if rec_it->id == LONG_MAX 
			auto &iter = cfg_ui_columns.find(rec_it->id);
			if (iter.is_valid()) {
				//update
				if (!(iter->m_value.m_name.equals(rec_it->name) &&
					iter->m_value.m_pattern.equals(rec_it->pattern) &&
					iter->m_value.m_alignment == rec_it->alignment)) 
				{
					iter->m_value.m_name = rec_it->name;
					iter->m_value.m_pattern = rec_it->pattern;
					iter->m_value.m_alignment = rec_it->alignment;
				}
				continue;
			}
			else {
				//add
				ui_column_definition cfg_def;
				cfg_def.m_name = rec_it->name;
				cfg_def.m_pattern = rec_it->pattern;
				cfg_def.m_alignment = rec_it->alignment;

				cfg_ui_columns.set(++max_id, cfg_def);

				continue;
			}
		}

		//del key/button...
		for (auto d : m_user_removals) {
			if (d != LONG_MAX) {
				auto& iter = cfg_ui_columns.find(d);
				if (iter.is_valid()) {
					cfg_ui_columns.remove(iter);
				}
			}
		}
		
		m_user_removals.clear();

		window_manager::SaveUILayout();
		window_manager::UIColumnsChanged(false);
		window_manager::GlobalRefresh();
	}

	cfg_playlist_name = playlist_name;
	m_columns_dirty = false;

	OnChanged();
}

bool CMyPreferences::HasChanged() {

	m_columns_dirty = false;

	pfc::string8 playlist_name;
	pfc::string8 ui_format;

	//todo
	// make sure we have normalized booleans for direct comparison with == and !=
	bool playlist_enabled = (IsDlgButtonChecked(IDC_PLAYLIST_ENABLED) == BST_CHECKED) != 0;
	bool playlist_enabled_cfg = cfg_playlist_enabled != 0;

	playlist_name = uGetDlgItemText(m_hWnd, IDC_PLAYLIST_NAME);
	
	bool bfields_changed = data.size() != cfg_ui_columns.get_count();

	m_columns_dirty = cfg_ui_columns.get_count() != data.size();

	if (!m_columns_dirty) {

		int c = 0;
		for (auto w : cfg_ui_columns) {

			field_t rec = { w.m_key, w.m_value.m_name, w.m_value.m_pattern, w.m_value.m_alignment };
			auto dbg = data.at(c);
			if (!(rec == data.at(c))) {
				m_columns_dirty = true;
				break;
			}
			c++;
		}
	}

	//returns whether our dialog content is different from the current configuration
	//(whether the apply button should be enabled or not)
	return (playlist_enabled != playlist_enabled_cfg )
		|| (playlist_name != cfg_playlist_name)
		|| m_columns_dirty;
}

void CMyPreferences::OnChanged() {
	//refresh the apply button state
	m_callback->on_state_changed();
}

LRESULT CMyPreferences::OnBnClickedPlaylistEnabled(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// Enable/Disable playlist name control
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(IsDlgButtonChecked(IDC_PLAYLIST_ENABLED) == BST_CHECKED);
	OnChanged();
	return 0;
}

LRESULT CMyPreferences::OnBnClickedButtonAddColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	field_t new_rec{ LONG_MAX, "new field", "", 0};

	data.emplace_back(new_rec);
	m_field_list.ReloadItem(data.size() - 1);
	m_field_list.OnItemsInserted(data.size() - 1, 1, true);


	m_columns_dirty = true;
	OnChanged();
	return 0;
}

LRESULT CMyPreferences::OnBnClickedButtonRemoveColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (listRemoveItems(nullptr, m_field_list.GetSelectionMask())) {
		m_columns_dirty = true;
		OnChanged();
	}

	return 0;
}

LRESULT CMyPreferences::OnBnClickedButtonSyntaxHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ShellExecute(NULL, _T("open"), _T(TITLEFORMAT_WIKIPAGEURL),
			NULL, NULL, SW_SHOWNORMAL);
	
	return 0;
}

static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;

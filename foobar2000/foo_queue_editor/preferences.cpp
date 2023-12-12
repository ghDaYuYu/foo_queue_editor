#include "stdafx.h"
#include <algorithm>

#include "queue_lock.h"
#include "yesno_dialog.h"
#include "preferences.h"

using namespace cfg_var_legacy;

CMyPreferences::~CMyPreferences() {

	m_static_header.Detach();
}

void CMyPreferences::init_syntax_link() {
	pfc::stringcvt::string_wide_from_utf8 wtext(TITLEFORMAT_WIKIPAGEURL);
	m_lnk_syntax_help.SetHyperLink((LPCTSTR)const_cast<wchar_t*>(wtext.get_ptr()));
	m_lnk_syntax_help.SetLabel(_T("Syntax help"));
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

	uSetDlgItemText(m_hWnd, IDC_TT_OBS, TT_INFO_TEXT);

	m_lnk_syntax_help.SubclassWindow(GetDlgItem(IDC_STATIC_HELP_SYNTAX));
	init_syntax_link();

	m_dark.AddDialogWithControls(m_hWnd);

	m_disable_on_change = true;

	CheckDlgButton(IDC_HEADER_ENABLED, cfg_show_header ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_LOAD_INIT, cfg_load_init ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_SAVE_QUIT, cfg_save_quit ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_PLAYLIST_ENABLED, cfg_playlist_enabled ? BST_CHECKED : BST_UNCHECKED);
	uSetDlgItemText(*this, IDC_PLAYLIST_NAME, cfg_playlist_name);

	m_disable_on_change = false;

	// Enable/Disable playlist name control
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(cfg_playlist_enabled);

	return TRUE;
}

void AddAlignMenuItems(CMenu* menu, bool single_sel, size_t curr_alignment) {

	menu->AppendMenu(MF_STRING | (single_sel && curr_alignment == HDF_LEFT ? MF_CHECKED : 0), (UINT_PTR)HDF_LEFT + 1, TEXT(ALIGNMENT_LEFT"\tL"));
	menu->AppendMenu(MF_STRING | (single_sel && curr_alignment == HDF_CENTER ? MF_CHECKED : 0), (UINT_PTR)HDF_CENTER + 1, TEXT(ALIGNMENT_CENTER"\tC"));
	menu->AppendMenu(MF_STRING | (single_sel && curr_alignment == HDF_RIGHT ? MF_CHECKED : 0), (UINT_PTR)HDF_RIGHT + 1, TEXT(ALIGNMENT_RIGHT"\tR"));

}

void CMyPreferences::OnContextMenu(CWindow wnd, CPoint point) {

	if (wnd == m_field_list) {

		pfc::bit_array_bittable selmask = m_field_list.GetSelectionMask();

		size_t csel = m_field_list.GetSelectedCount();
		size_t isel = selmask.find_first(true, 0, data.size());

		bool bsingle_sel = (csel == 1);

		size_t curr_alignment = SIZE_MAX;
		if (bsingle_sel) curr_alignment = data.at(isel).alignment;

		enum {
			ID_ADD = 10,
			ID_DELETE,
		};

		CMenu menu;
		WIN32_OP(menu.CreatePopupMenu());

		menu.AppendMenu(MF_STRING,ID_ADD + 1, TEXT("Add new field"));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING | (!csel ? MF_GRAYED | MF_DISABLED : (bsingle_sel && curr_alignment == HDF_LEFT ? MF_CHECKED : 0)), (UINT_PTR)HDF_LEFT + 1, TEXT(ALIGNMENT_LEFT"\tL"));
		menu.AppendMenu(MF_STRING | (!csel ? MF_GRAYED | MF_DISABLED : (bsingle_sel && curr_alignment == HDF_CENTER ? MF_CHECKED : 0)), (UINT_PTR)HDF_CENTER + 1, TEXT(ALIGNMENT_CENTER"\tC"));
		menu.AppendMenu(MF_STRING | (!csel ? MF_GRAYED | MF_DISABLED : (bsingle_sel && curr_alignment == HDF_RIGHT ? MF_CHECKED : 0)), (UINT_PTR)HDF_RIGHT + 1, TEXT(ALIGNMENT_RIGHT"\tR"));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING | (!csel ? MF_GRAYED | MF_DISABLED : 0), ID_DELETE + 1, TEXT("Remove"));

		CMenuDescriptionMap receiver(*this);

		int cmd = menu.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, receiver);

		if (cmd) {

			--cmd;

			switch (cmd) {
			case HDF_LEFT:
			[[walkthrought]]
			case HDF_RIGHT:
			[[walkthrought]]
			case HDF_CENTER: {
			[[walkthrought]]
				size_t n = selmask.find_first(true, 0, selmask.size());
				for (n; n < selmask.size(); n = selmask.find_next(true, n, selmask.size())) {
					data.at(n).alignment = cmd;
				}
				m_field_list.ReloadItems(selmask);
				OnChanged();
				break;
			}
			case ID_DELETE: 
				listRemoveItems(nullptr, selmask);
				break;
			case ID_ADD:
				add_new_item();
				break;
			default:
				;
			}
		}
	}

	//todo: Invalidate/Check changes

	return;
}

size_t CMyPreferences::ShowAlignContextMenu(size_t item, CPoint point) {

	auto isel = m_field_list.GetSingleSel();
	const field_t* entry = &data.at(isel);
	size_t curr_alignment = entry->alignment;

	CMenu menu;
	WIN32_OP(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | (curr_alignment == HDF_LEFT ? MF_CHECKED : 0), (UINT_PTR)HDF_LEFT + 1, TEXT(ALIGNMENT_LEFT"\tL"));
	menu.AppendMenu(MF_STRING | (curr_alignment == HDF_CENTER ? MF_CHECKED : 0), (UINT_PTR)HDF_CENTER + 1, TEXT(ALIGNMENT_CENTER"\tC"));
	menu.AppendMenu(MF_STRING | (curr_alignment == HDF_RIGHT ? MF_CHECKED : 0), (UINT_PTR)HDF_RIGHT + 1, TEXT(ALIGNMENT_RIGHT"\tR"));

	CMenuDescriptionMap receiver(*this);
	return menu.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, receiver);
}

void CMyPreferences::UpdateColumnDefinitionsFromCfg(const pfc::map_t<long, ui_column_definition>& config /*= cfg_ui_columns*/) {
	//..
}

void CMyPreferences::OnEditChange(UINT, int, CWindow) {

	if (m_disable_on_change) {
		return;
	}

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
	if (!yndlg.query(m_hWnd, { "Reset", "Reset Queue Editor settings?" }, false, false)) {
		return;
	}

	m_disable_on_change = true;

	CheckDlgButton(IDC_HEADER_ENABLED, default_cfg_show_header ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(IDC_LOAD_INIT, default_cfg_load_init ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_SAVE_QUIT, default_cfg_save_quit ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(IDC_PLAYLIST_ENABLED, default_cfg_playlist_enabled ? BST_CHECKED : BST_UNCHECKED);
	uSetDlgItemText(*this, IDC_PLAYLIST_NAME, default_cfg_playlist_name);
	GetDlgItem(IDC_PLAYLIST_NAME).EnableWindow(default_cfg_playlist_enabled);

	m_disable_on_change = false;

	size_t old_size = data.size();
	pfc::map_t<long, ui_column_definition> tmp_column_map;
	map_utils::fill_map_with_values(tmp_column_map, default_cfg_ui_columns);

	init_data(tmp_column_map);

	//update scroll bar
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

	OnChanged();
}

void CMyPreferences::apply() {

	bool bSavedUILayout = false;

	bool old_show_header = cfg_show_header;
	cfg_show_header = IsDlgButtonChecked(IDC_HEADER_ENABLED) == BST_CHECKED;

	if (old_show_header != cfg_show_header) {
		bSavedUILayout = true;
		window_manager::SaveUILayout();
		window_manager::HideUIHeader();
	}

	cfg_load_init = IsDlgButtonChecked(IDC_LOAD_INIT) == BST_CHECKED;
	cfg_save_quit = IsDlgButtonChecked(IDC_SAVE_QUIT) == BST_CHECKED;

	pfc::string8 playlist_name;
	cfg_playlist_enabled = IsDlgButtonChecked(IDC_PLAYLIST_ENABLED) == BST_CHECKED;
	playlist_name = uGetDlgItemText(m_hWnd, IDC_PLAYLIST_NAME);

	// Rename current queue playlist if necessary
	if(cfg_playlist_enabled && playlist_name != cfg_playlist_name){
		static_api_ptr_t<playlist_manager> playlist_api;
		t_size plIndex = queue_lock::find_playlist();
		if(plIndex != pfc::infinite_size) {
			playlist_api->playlist_rename(plIndex, playlist_name.get_ptr(), playlist_name.get_length());
		}
	}

	if(cfg_playlist_enabled) {
		queue_lock::install_lock();
	} else {
		queue_lock::uninstall_lock();
	}

	if (m_columns_dirty) {

		// apply column settings

		long max_id = 0;

		if (data.size()) {
			max_id = std::max_element(data.begin(),
			data.end(),
			[](const auto& a, const auto& b) {
				return (a.id < b.id) && (!(a.id == LONG_MAX || b.id == LONG_MAX));
			})->id;
		}

		pfc::map_t<long, ui_column_definition> old_ui_col_defs;
		old_ui_col_defs = cfg_ui_columns;

		pfc::map_t<long, ui_column_definition>& target_defs = cfg_ui_columns;

		for (auto rec_it = data.begin(); rec_it != data.end(); rec_it++) {

			auto &iter = target_defs.find(rec_it->id);

			if (iter.is_valid()) {
				PFC_ASSERT(iter->m_key != LONG_MAX);
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

				if (rec_it == data.begin()) {
					max_id = 0;
					target_defs.set(max_id, cfg_def);
				}
				else {
					target_defs.set(++max_id, cfg_def);
				}
				PFC_ASSERT(max_id != LONG_MAX);
				rec_it->id = max_id;
				continue;
			}
		}

		for (auto d : m_user_removals) {

			auto& iter = target_defs.find(d);
			if (iter.is_valid()) {
				target_defs.remove(iter);
			}
		}

		m_user_removals.clear();

		if (target_defs.get_count() != data.size()) {
			for (auto wt : target_defs) {
				auto found_it = std::find_if(data.begin(), data.end(), [=](const field_t rec) {
					return rec.id == wt.m_key;
					});
				if (found_it == data.end()) {
					target_defs.remove(wt.m_key);
				}
			}
		}

		if (!bSavedUILayout) {
			window_manager::SaveUILayout();
		}

		window_manager::UIColumnsChanged(old_ui_col_defs);
		//window_manager::GlobalRefresh();
	}

	cfg_playlist_name = playlist_name;

	OnChanged();
}

bool CMyPreferences::HasChanged() {

	m_columns_dirty = false;

	bool header_enabled = (IsDlgButtonChecked(IDC_HEADER_ENABLED) == BST_CHECKED) != 0;
	bool bshow_header_changed = header_enabled != cfg_show_header;

	bool bload_init_enabled = (IsDlgButtonChecked(IDC_LOAD_INIT) == BST_CHECKED) != 0;
	bool bload_init_changed = bload_init_enabled != cfg_load_init;
	bool bsave_quit_enabled = (IsDlgButtonChecked(IDC_SAVE_QUIT) == BST_CHECKED) != 0;
	bool bsave_quit_changed = bsave_quit_enabled != cfg_save_quit;

	pfc::string8 playlist_name;
	pfc::string8 ui_format;

	//todo
	// make sure we have normalized booleans for direct comparison with == and !=
	bool playlist_enabled = (IsDlgButtonChecked(IDC_PLAYLIST_ENABLED) == BST_CHECKED) != 0;
	bool playlist_enabled_cfg = cfg_playlist_enabled != 0;

	playlist_name = uGetDlgItemText(m_hWnd, IDC_PLAYLIST_NAME);

	m_columns_dirty = cfg_ui_columns.get_count() != data.size();

	if (!m_columns_dirty) {

		for (auto w : data) {

			auto cf = cfg_ui_columns.find(w.id);
			if (!cf.is_valid()) {
			}
			else {
				field_t rec = { cf->m_key, cf->m_value.m_name, cf->m_value.m_pattern, cf->m_value.m_alignment };

				if (!(rec == w)) {
					m_columns_dirty = true;
					break;
				}
			}
		}
	}

	//returns whether our dialog content is different from the current configuration
	//(whether the apply button should be enabled or not)
	return (playlist_enabled != playlist_enabled_cfg )
		|| (playlist_name != cfg_playlist_name)
		|| m_columns_dirty || bshow_header_changed
		|| bsave_quit_changed || bload_init_changed;
}

void CMyPreferences::OnChanged() {
	//refresh the apply button state
	m_callback->on_state_changed();
}

LRESULT CMyPreferences::OnBnClickedEnabled(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	OnChanged();
	return 0;
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
	add_new_item();
	return 0;
}


static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;

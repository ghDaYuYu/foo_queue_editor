#pragma once
#include "libPPUI\CListControlOwnerData.h"
#include "libPPUI\CDialogResizeHelper.h"
#include <helpers\DarkMode.h>
#include "helpers\atl-misc.h"
#include "config.h"
#include "guids.h"
#include "queuecontents_lock.h"

#include "window_manager.h"

#include "queuecontents_lock.h"
#include "header_static.h"

#define NAME_COLUMN_TEXT "Name"
#define PATTERN_COLUMN_TEXT "Pattern"
#define ALIGNMENT_COLUMN_TEXT "Alignment"

#define ALIGNMENT_LEFT "Left"
#define ALIGNMENT_CENTER "Center"
#define ALIGNMENT_RIGHT "Right"

struct field_t {
	long id = 0;
	pfc::string8 name;
	pfc::string8 pattern;
	t_int alignment = 0;
};

inline bool operator ==(const field_t& a, const field_t& b) {
	return  a.id == b.id && 
			a.name.equals(b.name) &&
			a.pattern.equals(b.pattern) &&
			a.alignment == b.alignment;
}

std::vector<field_t>data;

void init_data() {

	data.clear();

	//todo: remove cfg_next_id
	auto curr_max_id = cfg_next_id.get_value();
	t_int32 next_max_id = 0;
	auto c = cfg_ui_columns.get_count();
	for (auto w : cfg_ui_columns) {
		field_t rec = { w.m_key, w.m_value.m_name, w.m_value.m_pattern, w.m_value.m_alignment };
		data.emplace_back(rec);
		if (w.m_key > curr_max_id) {
			next_max_id = w.m_key + 1;
		}
	}
	if (next_max_id) {
		cfg_next_id = next_max_id;
	}
}

class CFieldList : public CListControlOwnerData {

public:

	CFieldList(IListControlOwnerDataSource* h)
		: CListControlOwnerData(h) {
		
		init_data();
	}

	~CFieldList() {
		//..
	}

	BEGIN_MSG_MAP_EX(CFieldList)
		CHAIN_MSG_MAP(CListControlOwnerData)
	END_MSG_MAP()

private:

	std::vector<long>m_user_removals;

private:

	fb2k::CDarkModeHooks m_dark;
};

//snapLeft, snapTop, snapRight, snapBottom
const CDialogResizeHelper::Param rz_params[] = {
	{IDC_STATIC_HEADER, 0,0,1,0},
	{IDC_BUTTON_ADD_COLUMN, 1,0,1,0},
	{IDC_BUTTON_REMOVE_COLUMN, 1,0,1,0},
	{IDC_BUTTON_SYNTAX_HELP, 1,0,1,0},
	{IDC_FIELD_LIST, 0,0,1,0},
	{IDC_STATIC_OBS, 0,0,1,0},
	{IDC_STATIC_PL_OBS, 0,0,1,0},
	{IDC_PLAYLIST_NAME, 0,0,1,0},
};

class CMyPreferences : public CDialogImpl<CMyPreferences>,
	public preferences_page_instance, private IListControlOwnerDataSource {

public:

	CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback), m_field_list(this), m_resize_helper(rz_params) {}
	~CMyPreferences();

	//dialog resource ID
	enum {IDD = IDD_PREFERENCES};

	// preferences_page_instance methods
	t_uint32 get_state();
	void apply();
	void reset();

	//WTL message map
	BEGIN_MSG_MAP(CMyPreferences)
		CHAIN_MSG_MAP_MEMBER(m_resize_helper)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		COMMAND_HANDLER_EX(IDC_UI_FORMAT, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER_EX(IDC_PLAYLIST_ENABLED, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER_EX(IDC_PLAYLIST_NAME, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_PLAYLIST_ENABLED, BN_CLICKED, OnBnClickedPlaylistEnabled)
		COMMAND_HANDLER(IDC_BUTTON_ADD_COLUMN, BN_CLICKED, OnBnClickedButtonAddColumn)
		COMMAND_HANDLER(IDC_BUTTON_REMOVE_COLUMN, BN_CLICKED, OnBnClickedButtonRemoveColumn)
		COMMAND_HANDLER(IDC_BUTTON_SYNTAX_HELP, BN_CLICKED, OnBnClickedButtonSyntaxHelp)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

public:
	LRESULT OnBnClickedPlaylistEnabled(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonAddColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonRemoveColumn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButtonSyntaxHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:

	BOOL OnInitDialog(CWindow, LPARAM);
	void OnEditChange(UINT, int, CWindow);
	bool HasChanged();
	void OnChanged();

	void UpdateColumnDefinitionsFromCfg(const pfc::map_t<long, ui_column_definition>& config = cfg_ui_columns);

	// IListControlOwnerDataSource 

	// get Item Count

	size_t listGetItemCount(ctx_t ctx) override {

		return data.size();
	}

	// get subitems

	pfc::string8 listGetSubItemText(ctx_t ctx, size_t item, size_t subItem) override {

		pfc::string8 buffer;
		if (item < data.size()) {

			const field_t* entry = &data.at(item);

			if (subItem == 0) {
				buffer = entry->name;
			}
			else if (subItem == 1) {
				buffer = entry->pattern;
			}
			else if (subItem == 2) {
				const char* text =
					(entry->alignment > 0 ?
						(entry->alignment > 1 ?
							ALIGNMENT_CENTER
							: ALIGNMENT_RIGHT) :
						ALIGNMENT_LEFT);

				buffer = pfc::string8(text);
			}
		}
		return buffer;
	}

	// can reorder

	bool listCanReorderItems(ctx_t) override {
		return false;
	}

	// reorder

	bool listReorderItems(ctx_t ctx, const size_t* order, size_t count) override {
		return false;
	}

	// remove

	bool listRemoveItems(ctx_t ctx, pfc::bit_array const& mask) override {

		size_t deleted = 0;
		auto n = mask.find_first(true, 0, data.size());

		data.erase(
			std::remove_if(std::begin(data), std::end(data), [&](field_t& elem)
				{
					if (n < data.size() && (&elem - &data[0] == n))
					{
						n = mask.find_next(true, n, data.size());
						deleted++;
						m_user_removals.emplace_back(elem.id);
						return true;
					}
					return false;
				}),
			std::end(data)
		);

		m_field_list.OnItemsRemoved(mask, deleted);

		return deleted;
	}

	// item action

	void listItemAction(ctx_t, size_t item) override {
		//..
	}

	void listSubItemClicked(ctx_t, size_t item, size_t subItem) override {

		const field_t* entry = &data.at(item);

		if (subItem == 0 || subItem == 1) {
			m_field_list.TableEdit_Start(item, subItem);
		}
		if (subItem == 2) {
			CPoint p;
			::GetCursorPos(&p);
			LPARAM lparam = MAKELPARAM(p.x, p.y);
			OnContextMenu((HWND)m_field_list, p);
		}
	}

	// set edited text

	void listSetEditField(ctx_t ctx, size_t item, size_t subItem, const char* val) override {

		field_t& data_rec = data.at(item);
		long datalong = data_rec.id;
		
		//ui_column_definition& dbg_ndx = cfg_ui_columns[datalong];

		t_size column_count = cfg_ui_columns.get_count();
		pfc::array_staticsize_t<long> m_header_ctx_menu_field_ids = pfc::array_staticsize_t<long>(column_count);
		
		long field_id = LONG_MAX;
		cfg_ui_iterator iter;

		int i = 0;
		for (cfg_ui_iterator iter = cfg_ui_columns.first(); iter.is_valid(); iter++) {

			m_header_ctx_menu_field_ids[i] = iter->m_key;
			if (i == datalong) {
				field_id = iter->m_key;
				break;
			}
			i++;
		}

		//note: still LONG_MAX field_id in new entries 

		if (subItem == 0) {
			data_rec.name = val;
		}
		else if (subItem == 1) {
			data_rec.pattern = val;
		}
		else if (subItem == 2) {
			data_rec.alignment = atoi(val);
		}

		OnChanged();
	}

	// can edit

	bool listIsColumnEditable(ctx_t, size_t subItem) override {
		return !subItem || subItem == 1;
	}

	// focus

	void listFocusChanged(ctx_t) {
		//..
	}

	//..end IListControlOwnerDataSource

	void OnContextMenu(CWindow wnd, CPoint point);

protected:

	std::vector<long> m_user_removals;

private:

	const preferences_page_callback::ptr m_callback;

	CFieldList m_field_list;
	bool m_columns_dirty;

	fb2k::CDarkModeHooks m_dark;
	HeaderStatic m_static_header;
	CDialogResizeHelper m_resize_helper;
};


class preferences_page_myimpl : public preferences_page_impl<CMyPreferences> {

public:
	const char * get_name() {
		return COMPONENT_NAME_H;
	}
	
	GUID get_guid() {
		return guid_preferences;
	}
	
	GUID get_parent_guid() {
		return guid_tools;
	}

	bool get_help_url (pfc::string_base &p_out) {
		p_out = "http://wiki.hydrogenaudio.org/index.php?title=Foobar2000:Components/Queue_Editor_(foo_queue_editor)";
		return true;
	}
};

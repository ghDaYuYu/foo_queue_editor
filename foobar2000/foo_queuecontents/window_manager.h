#pragma once
#include <list>
#include "config.h"

class window_manager_window {
public:

	virtual void Refresh() = 0;
	virtual void GetLayout() {}
	virtual void PrefColumnsChanged(bool reset = false) {}
	virtual void PrefColumnsChanged(pfc::map_t<long, ui_column_definition> old_ui_col_defs) {}
	virtual void RefreshVisuals() {}
	virtual void HideHeader() {}
};

class window_manager
{
private:
	static std::list<window_manager_window*> m_window_list;
	static critical_section m_critical_section;
	static bool hasInit;
	static bool updates_enabled;

	static void GetListLock();
	static void ReleaseListLock();

public:
	static void EnableUpdates(bool enable);
	static void AddWindow(window_manager_window* wnd);
	static void RemoveWindow(window_manager_window* wnd);
	static std::list<window_manager_window*> GetWindowList();
	static void GlobalRefresh();
	static void UIColumnsChanged(bool reset);
	static void UIColumnsChanged(pfc::map_t<long, ui_column_definition> modded_ui_col_defs);
	static void SaveUILayout();
	static void VisualsChanged();
	static void HideUIHeader();
};

class global_refresh_callback : public main_thread_callback
{
public:
	void callback_run() {
		window_manager::GlobalRefresh();
	}
};

class NoRefreshScope {
public:
	NoRefreshScope(bool scopeEnabled = true);
	~NoRefreshScope();
	void EnableScope(bool scopeEnabled = true);
private:
	bool bScopeEnabled;
};

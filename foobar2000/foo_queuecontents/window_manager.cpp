#include "stdafx.h"
#include "window_manager.h"
#include "queuecontents_lock.h"

bool window_manager::updates_enabled = true;
std::list<window_manager_window*> window_manager::m_window_list;
critical_section window_manager::m_critical_section;

void window_manager::EnableUpdates(bool enable = true) {
	updates_enabled = enable;

	if(updates_enabled) {
		GlobalRefresh();
	}
}

void window_manager::GetListLock()
{
	m_critical_section.enter();
}

void window_manager::ReleaseListLock()
{
	m_critical_section.leave();
}

void window_manager::AddWindow(window_manager_window* wnd)
{
#ifdef _DEBUG
	console::formatter() << "window_manager: Registering Window";
	console::formatter() << "current size of registered windows: " << m_window_list.size();
#endif
	GetListLock();
	m_window_list.push_back(wnd);
	ReleaseListLock();
#ifdef _DEBUG
	console::formatter() << "window_manager: Registered Window";
	console::formatter() << "current size of registered windows: " << m_window_list.size();
#endif
}

void window_manager::RemoveWindow(window_manager_window* wnd)
{
#ifdef _DEBUG
	console::formatter() << "window_manager: Unregistering Window";
	console::formatter() << "current size of registered windows: " << m_window_list.size();
#endif
	m_critical_section.enter();
	m_window_list.remove(wnd);
	m_critical_section.leave();
#ifdef _DEBUG
	console::formatter() << "window_manager: Unregistered Window";
	console::formatter() << "current size of registered windows: " << m_window_list.size();
#endif
}


std::list<window_manager_window*> window_manager::GetWindowList()
{
	TRACK_CALL_TEXT("window_manager::GetWindowList");
	std::list<window_manager_window*> ret;
	GetListLock();
	ret = m_window_list;
	ReleaseListLock();
	
	return ret;
}

void window_manager::GlobalRefresh()
{
	TRACK_CALL_TEXT("window_manager::GlobalRefresh");
	if(!updates_enabled)
		return;

	std::list<window_manager_window*> windowList = GetWindowList();
	std::list<window_manager_window*>::iterator Iter;

	//console::formatter() << "Executing global redraw";
	updates_enabled = false; // To ensure that GlobalRefresh() is not called again
	for (Iter = windowList.begin(); Iter != windowList.end(); Iter++)
	{
		(*Iter)->Refresh();
	}
	updates_enabled = true;
}

void window_manager::SaveUILayout() {
	TRACK_CALL_TEXT("window_manager::SaveUILayout");

	std::list<window_manager_window*> windowList = GetWindowList();
	std::list<window_manager_window*>::iterator Iter;

	for (Iter = windowList.begin(); Iter != windowList.end(); Iter++)
	{
		(*Iter)->GetLayout();
	}
}

void window_manager::UIColumnsChanged(bool reset) {
	TRACK_CALL_TEXT("window_manager::UIColumnsChanged");
	if(!updates_enabled)
		return;

	std::list<window_manager_window*> windowList = GetWindowList();
	std::list<window_manager_window*>::iterator Iter;

	//console::formatter() << "Executing global redraw";

	for (Iter = windowList.begin(); Iter != windowList.end(); Iter++)
	{
		(*Iter)->PrefColumnsChanged(reset);
	}
}

void window_manager::UIColumnsChanged(pfc::map_t<long, ui_column_definition> old_ui_col_defs) {
	TRACK_CALL_TEXT("window_manager::UIColumnsChanged");

	if (!updates_enabled)
		return;

	std::list<window_manager_window*> windowList = GetWindowList();
	std::list<window_manager_window*>::iterator Iter;

	for (Iter = windowList.begin(); Iter != windowList.end(); Iter++)
	{
		(*Iter)->PrefColumnsChanged(old_ui_col_defs);
	}
}

void window_manager::VisualsChanged() {
	TRACK_CALL_TEXT("window_manager::VisualsChanged");
	if(!updates_enabled)
		return;

	std::list<window_manager_window*> windowList = GetWindowList();
	std::list<window_manager_window*>::iterator Iter;

	//console::formatter() << "Executing global redraw";

	for (Iter = windowList.begin(); Iter != windowList.end(); Iter++)
	{
		(*Iter)->RefreshVisuals();
	}
}

NoRefreshScope::NoRefreshScope(bool scopeEnabled /*= true*/) : bScopeEnabled(scopeEnabled) {
	if(scopeEnabled) {
		window_manager::EnableUpdates(false);
	}
}

NoRefreshScope::~NoRefreshScope() {
	if(bScopeEnabled) {
		window_manager::EnableUpdates(true);
	}
}

void NoRefreshScope::EnableScope(bool scopeEnabled /*= true*/) {
	bScopeEnabled = scopeEnabled;
}

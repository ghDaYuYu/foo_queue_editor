#pragma once

#include "stdafx.h"
#include "window_manager.h"

// class that responds to Refresh() function and watches all playlist queue items for changes in %list_index%, %list_total% and %playlist_name%
// If changes are found, ui-element refresh is requested
class playlist_queue_item_watcher : public window_manager_window, public playlist_callback_impl_base {
public:
	playlist_queue_item_watcher();
	~playlist_queue_item_watcher();
	virtual void Refresh() override;

private:
	void addToWatchList(t_playback_queue_item);

	void globalRefresh();
	void SaveUILayout() {}

	void on_items_added(t_size p_playlist,t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection);
	void on_items_reordered(t_size p_playlist,const t_size * p_order,t_size p_count);
	void on_items_removed(t_size p_playlist,const bit_array & p_mask,t_size p_old_count,t_size p_new_count);
	void on_items_modified(t_size p_playlist,const bit_array & p_mask);
	void on_items_replaced(t_size p_playlist,const bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data);
	void on_playlists_removed(const bit_array & p_mask,t_size p_old_count,t_size p_new_count);
	void on_playlist_renamed(t_size p_index,const char * p_new_name,t_size p_new_name_len);

	// playlist index<->playlist items mapping of watched items
	pfc::map_t< t_size, pfc::avltree_t<t_size> > watched_items;

	bool invalidated;
};

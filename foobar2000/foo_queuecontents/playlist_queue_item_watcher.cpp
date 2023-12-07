#include "stdafx.h"
#include "playlist_queue_item_watcher.h"

playlist_queue_item_watcher::playlist_queue_item_watcher(): watched_items(), invalidated(false) {
	window_manager::AddWindow(this);
}

playlist_queue_item_watcher::~playlist_queue_item_watcher() {
	window_manager::RemoveWindow(this);
}

void playlist_queue_item_watcher::Refresh() {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::Refresh");

	static_api_ptr_t<playlist_manager> playlist_api;
	pfc::list_t<t_playback_queue_item> queue;
	playlist_api->queue_get_contents(queue);
	
	watched_items.remove_all();
	t_size count = queue.get_count();
	for(t_size i = 0; i < count; i++) {
		addToWatchList(queue.get_item(i));
	}
	invalidated = false;
}

void playlist_queue_item_watcher::addToWatchList(t_playback_queue_item item) {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::addToWatchList");
	if(item.m_item != pfc::infinite_size && item.m_playlist != pfc::infinite_size) {
		pfc::avltree_t<t_size> &playlist_items = watched_items.find_or_add(item.m_playlist);
		playlist_items.add(item.m_item);
	}
}

void playlist_queue_item_watcher::globalRefresh() {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::globalRefresh");
	static_api_ptr_t<main_thread_callback_manager>()->add_callback(new service_impl_t<global_refresh_callback>());
}

void playlist_queue_item_watcher::on_items_added(t_size p_playlist,t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection) {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::on_items_added");
	if(invalidated) return;
	if(!watched_items.exists(p_playlist)) return;
	
	// list_total has changed anyways -> refresh
	invalidated = true;
	globalRefresh();
}

void playlist_queue_item_watcher::on_items_reordered(t_size p_playlist,const t_size * p_order,t_size p_count) {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::on_items_reordered");
	if(invalidated) return;
	if(!watched_items.exists(p_playlist)) return;
	pfc::map_t< t_size, pfc::avltree_t<t_size> >::const_iterator iterator = watched_items.find(p_playlist);
	
	for(t_size i = 0; i < p_count; i++) {
		if(p_order[i] != i) {
			// Order changed for item with index i
			if(iterator->m_value.have_item(i)) {
				DEBUG_PRINT << "playlist_queue_item_watcher::on_items_reordered: Playlist " << p_playlist << " has invalidated item " << i << ".";
				invalidated = true;
				globalRefresh();
				return;
			}
		}
	}
}

void playlist_queue_item_watcher::on_items_removed(t_size p_playlist,const bit_array & p_mask,t_size p_old_count,t_size p_new_count) {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::on_items_removed");
	if(invalidated) return;
	if(!watched_items.exists(p_playlist)) return;
	// list_total has changed anyways -> refresh

	invalidated = true;
	globalRefresh();
}

void playlist_queue_item_watcher::on_items_modified(t_size p_playlist,const bit_array & p_mask) {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::on_items_modified");
	if(invalidated) return;
	if(!watched_items.exists(p_playlist)) return;
	invalidated = true;
	globalRefresh();
}

void playlist_queue_item_watcher::on_items_replaced(t_size p_playlist,const bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data) {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::on_items_replaced");
	if(invalidated) return;
	if(!watched_items.exists(p_playlist)) return;
	invalidated = true;
	globalRefresh();
}

void playlist_queue_item_watcher::on_playlists_removed(const bit_array & p_mask,t_size p_old_count,t_size p_new_count) {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::on_playlists_removed");
	if(invalidated) return;	
	for(pfc::map_t< t_size, pfc::avltree_t<t_size> >::const_iterator iterator = watched_items.first(); iterator.is_valid(); iterator.next()) {
		t_size playlist = iterator->m_key;
		if(p_mask.get(playlist)) {
			DEBUG_PRINT << "playlist_queue_item_watcher::on_playlists_removed: playlist " << playlist << " has been removed.";
			invalidated = true;
			globalRefresh();
		}
	}
}

void playlist_queue_item_watcher::on_playlist_renamed(t_size p_index,const char * p_new_name,t_size p_new_name_len) {
	TRACK_CALL_TEXT("playlist_queue_item_watcher::on_playlist_renamed");
	if(invalidated) return;
	if(watched_items.have_item(p_index)) {
		DEBUG_PRINT << "playlist_queue_item_watcher::on_playlist_renamed: playlist " << p_index << " has been renamed.";
		invalidated = true;
		globalRefresh();	
	}
}
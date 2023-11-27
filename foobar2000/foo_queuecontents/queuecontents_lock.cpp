#include "stdafx.h"
#include "queuecontents_lock.h"
#include "playlist_updater.h"
#include "window_manager.h"

service_ptr_t<queuecontents_lock> queuecontents_lock::plLock = NULL;
playlist_updater queuecontents_lock::m_playlist_updater;
bool queuecontents_lock::updateQueuePlaylist = true;

//! Queries whether specified item insertion operation is allowed in the locked playlist.
//! @param p_base Index from which the items are being inserted.
//! @param p_data Items being inserted.
//! @param p_selection Caller-requested selection state of items being inserted.
//! @returns True to allow the operation, false to block it.

bool queuecontents_lock::query_items_add(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection)
{
	TRACK_CALL_TEXT("queuecontents_lock::query_items_add");
	DEBUG_PRINT << "query_items_add";
	if(!updateQueuePlaylist) {
		return true;
	}

	// We've already done the queue changes so we do *not*
	// want them propagated back again
	updateQueuePlaylist = false;
	queue_helpers::queue_add_items(p_base, p_data);
	updateQueuePlaylist = true;

	return true;
}
bool queuecontents_lock::query_items_reorder(const t_size * p_order, t_size p_count)
{
	TRACK_CALL_TEXT("queuecontents_lock::query_items_reorder");
	DEBUG_PRINT << "query_items_reorder";
	if(!updateQueuePlaylist) {
		DEBUG_PRINT << "Queue playlist: query_items_reorder. Allowing action, but not propagating it to queue.";
		return true;
	}

	PFC_ASSERT( p_count == static_api_ptr_t<playlist_manager>()->queue_get_count() );	

	// We've already done the queue changes so we do *not*
	// want them propagated back again
	updateQueuePlaylist = false;
	DEBUG_PRINT << "Queue playlist: reordering queue";
	queue_helpers::queue_reorder(p_order);
	updateQueuePlaylist = true;

	return true;
}
bool queuecontents_lock::query_items_remove(const bit_array & p_mask, bool p_force)
{
	TRACK_CALL_TEXT("queuecontents_lock::query_items_remove");
	if(!updateQueuePlaylist) {
		DEBUG_PRINT << "Queue playlist: query_items_remove. Allowing action, but not propagating it to queue.";
		return true;
	}

	// We've already done the queue changes so we do *not*
	// want them propagated back again
	updateQueuePlaylist = false;
	queue_helpers::queue_remove_mask(p_mask);
	updateQueuePlaylist = true;

	return true;
}
bool queuecontents_lock::query_item_replace(t_size p_index,const metadb_handle_ptr & p_old,const metadb_handle_ptr & p_new)
{
	// do not allow replace
	return false;
}
bool queuecontents_lock::query_playlist_rename(const char * p_new_name,t_size p_new_name_len)
{
	TRACK_CALL_TEXT("queuecontents_lock::query_playlist_rename");
	//update config accordingly
	cfg_playlist_name = pfc::string8(p_new_name);
	return true;
}
bool queuecontents_lock::query_playlist_remove()
{
	return false;
}
bool queuecontents_lock::execute_default_action(t_size p_item)
{		
	// custom default action was executed
	return true;
}
void queuecontents_lock::on_playlist_index_change(t_size p_new_index)
{
	//queuecontents_state::queuePlaylistIndex = p_new_index;
}
void queuecontents_lock::on_playlist_remove()
{
	//..
}
void queuecontents_lock::get_lock_name(pfc::string_base & p_out)
{
	p_out = "foo_queuecontents lock";
}
void queuecontents_lock::show_ui()
{
	console::formatter() << "No UI available for " COMPONENT_NAME_H ". See Settings for configuration.";
}
//! Queries which actions the lock filters. The return value must not change while the lock is registered with playlist_manager. The return value is a combination of one or more filter_* constants.
t_uint32 queuecontents_lock::get_filter_mask()
{
	// Doesn't filter out much. Adding filters like remove_playlist caused problems with drag & drop behaviour
	return playlist_lock::filter_replace;
}

void queuecontents_lock::clean_lock_playlist()
{
	TRACK_CALL_TEXT("queuecontents_lock::clean_lock_playlist");
	static_api_ptr_t<playlist_manager> playlist_api;

	pfc::list_t<t_playback_queue_item> queue;
	pfc::list_t<metadb_handle_ptr> queue_metadbs;
	playlist_api->queue_get_contents(queue);
	t_size qSize = playlist_api->queue_get_count();

	for(size_t j = 0; j < qSize; j++)
	{
		queue_metadbs.add_item(queue[j].m_handle);
	}



	//disable delete and insert triggers
	updateQueuePlaylist = false;		
	t_size queuePlaylistIndex = find_or_create_playlist();
	playlist_api->playlist_clear(queuePlaylistIndex);		
	playlist_api->playlist_add_items(queuePlaylistIndex, queue_metadbs, bit_array_true());


	//restore delete and insert triggers
	updateQueuePlaylist = true;
}

void queuecontents_lock::install_lock() {
	TRACK_CALL_TEXT("queuecontents_lock::install_lock");
	// Uninstall lock before installing
	uninstall_lock();

	//get playlist name from configs
	static_api_ptr_t<playlist_manager> playlist_api;

	//ensure that playlist exists
	t_size queuePlaylistIndex = find_or_create_playlist();

	
	//create new reference counter for queueLock-instantiation
	plLock = new service_impl_t<queuecontents_lock>();

	//install playlist lock
	playlist_api->playlist_lock_install(queuePlaylistIndex, plLock);

	clean_lock_playlist();

	window_manager::AddWindow(&m_playlist_updater);

}


void queuecontents_lock::uninstall_lock() {	
	TRACK_CALL_TEXT("queuecontents_lock::uninstall_lock");
	if(plLock != NULL) {
		static_api_ptr_t<playlist_manager_v2> playlist_api;
		t_size queuePlaylistIndex = find_playlist();
		if(queuePlaylistIndex != pfc::infinite_size && playlist_api->playlist_lock_is_present(queuePlaylistIndex)) {
			playlist_api->playlist_lock_uninstall(queuePlaylistIndex, plLock);			
		}
		plLock.release();
	}

	window_manager::RemoveWindow(&m_playlist_updater);
}


t_size queuecontents_lock::find_or_create_playlist() {
	t_size playlist_index = find_playlist();
	if(playlist_index == pfc::infinite_size) {
		static_api_ptr_t<playlist_manager_v2> playlist_api;
		playlist_index = playlist_api->create_playlist(cfg_playlist_name, cfg_playlist_name.get_length(), pfc::infinite_size);
		playlist_api->playlist_set_property_int(playlist_index, guid_queue_playlist_property, (t_size)1);
	}
	return playlist_index;
}

t_size queuecontents_lock::find_playlist() {
	static_api_ptr_t<playlist_manager_v2> playlist_api;
	t_size playlist_count = playlist_api->get_playlist_count();
	for(t_size i = 0; i < playlist_count; i++) {
		
		if(playlist_api->playlist_have_property(i, guid_queue_playlist_property)) {
			pfc::string8 name;
			playlist_api->playlist_get_name(i, name);
			if(cfg_playlist_name != name) {
				// Somehow the playlist name is changed. Discard the playlist
				PFC_ASSERT(!playlist_api->playlist_lock_is_present(i)); // No lock should be present...
				playlist_api->playlist_remove_property(i, guid_queue_playlist_property);
			} else {
				return i;
			}
		}
	}
	return pfc::infinite_size;
}


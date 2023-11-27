#include "stdafx.h"
#include "playlist_updater.h"
#include "queuecontents_lock.h"
#include "queue_helpers.h"

void playlist_updater::Refresh(){
	TRACK_CALL_TEXT("playlist_updater::Refresh");
	DEBUG_PRINT << "Updating queue playlist";
	if(!queuecontents_lock::updateQueuePlaylist || !cfg_playlist_enabled) {
		DEBUG_PRINT << "Updating queue playlist....Stopping the update";
		return;
	}

	static_api_ptr_t<playlist_manager> playlist_api;
	t_size queuePlaylistIndex = queuecontents_lock::find_or_create_playlist();

	pfc::list_t<t_playback_queue_item> queue;
	pfc::list_t<metadb_handle_ptr> queue_metadbs;
	playlist_api->queue_get_contents(queue);
	t_size qSize = queue.get_count();

	queue_helpers::extract_metadb_handles(queue, queue_metadbs);

	PFC_ASSERT( qSize == queue_metadbs.get_count() );

	//try to keep selection
	//save selection to bittable
	bit_array_bittable curSel(qSize);
	playlist_api->playlist_get_selection_mask(queuePlaylistIndex, curSel);

#ifdef _DEBUG
	//console::formatter() << "Deletion and Insertion operations are now disabled.";
#endif

	//disable delete and insert triggers
	queuecontents_lock::updateQueuePlaylist = false;	

	playlist_api->playlist_clear(queuePlaylistIndex);		
	bool playlist_add_item_ok = playlist_api->playlist_add_items(queuePlaylistIndex, queue_metadbs, bit_array_true());
	PFC_ASSERT( playlist_add_item_ok );

#ifdef _DEBUG
	//console::formatter() << "Deletion and Insertion operations are now enabled.";
#endif

	//restore delete and insert triggers
	queuecontents_lock::updateQueuePlaylist = true;

	//restore selection
	playlist_api->playlist_set_selection(queuePlaylistIndex, bit_array_true(), curSel);

	//free objects
	queue_metadbs.remove_all();
	queue.remove_all();

}

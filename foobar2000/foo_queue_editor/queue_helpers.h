#pragma once
#include "window_manager.h" //NoRefreshScope

struct inplay_t {
	size_t trk_pos = SIZE_MAX;
	size_t trk_dup = SIZE_MAX;
	size_t pl_ndx = SIZE_MAX;
	size_t pl_size = 0;
};

class queue_helpers {
public:
	static void queue_remove_mask(const pfc::bit_array& p_mask) {
		TRACK_CALL_TEXT("queue_helpers::queue_remove_mask");
		static_api_ptr_t<playlist_manager> playlist_api;

		//disable updating queue UIs
		{
			NoRefreshScope tmp;
			playlist_api->queue_remove_mask(p_mask);
		}

	}

	static void queue_reorder(const pfc::list_t<t_size>& p_order) {
		TRACK_CALL_TEXT("queue_helpers::queue_reorder");

		queue_reorder(p_order.get_ptr());
	}

	// Duplicates queue items specified as 0-based queue indices, indicesToDuplicate. All duplicated
	// queue entries are inserted into duplicationPoint
	static void queue_duplicate_items(t_size duplicationPoint, const pfc::list_base_const_t<t_size>& indicesToDuplicate) {
		TRACK_CALL_TEXT("queue_helpers::queue_duplicate_items");
		static_api_ptr_t<playlist_manager> playlist_api;
		pfc::list_t<t_playback_queue_item> queue;
		t_size new_data_count = indicesToDuplicate.get_count();
		playlist_api->queue_get_contents(queue);
		t_size queue_orig_count = queue.get_count();

		NoRefreshScope tmp;

		playlist_api->queue_flush();

		for(t_size i = 0; i < duplicationPoint; i++) {
			add_queue_item(queue[i]);
		}

		for(t_size i = 0; i < new_data_count; i++) {
			t_size j = indicesToDuplicate[i];
			add_queue_item(queue[j]);
		}

		for(t_size i = duplicationPoint; i < queue_orig_count; i++) {
			add_queue_item(queue[i]);
		}

	}

	// Duplicates queue items specified as bit_array, indicesToDuplicate. All duplicated
	// queue entries are inserted into duplicationPoint
	static void queue_duplicate_items(t_size insertionPoint, const bit_array & items, t_size items_count) {
		TRACK_CALL_TEXT("queue_helpers::queue_duplicate_items - bit array");
		pfc::list_t<t_size> indicesToDuplicate;
		t_size item = items.find_first(true, 0, items_count);
		
		while(item < items_count) {
			indicesToDuplicate.add_item(item);
			item = items.find_next(true, item, items_count);			
		}

		queue_duplicate_items(insertionPoint, indicesToDuplicate);
	}

	static size_t queue_insert_items(t_size insertionPoint, t_size playlist_index, const std::vector <inplay_t*> &vmixed_chunk_ref, t_size pl_count) {
		size_t cinserts = 0;
	
		static_api_ptr_t<playlist_manager> playlist_api;
		pfc::list_t<t_playback_queue_item> queue;
		playlist_api->queue_get_contents(queue);
		t_size queue_orig_count = queue.get_count();

		NoRefreshScope tmp;

		if (insertionPoint <= queue_orig_count) {
			playlist_api->queue_flush();
		}

		for (t_size i = 0; i < insertionPoint; i++) {
			add_queue_item(queue[i]);
		}

		for (auto inplay : vmixed_chunk_ref) {
			playlist_api->queue_add_item_playlist(playlist_index, inplay->trk_pos);
			cinserts++;
		}

		for (t_size i = insertionPoint; i < queue_orig_count; i++) {
			add_queue_item(queue[i]);
		}
		return cinserts;
	}

	// Queue items from a single playlist, preserves "connection" to the playlist
	static size_t queue_insert_items(t_size insertionPoint, t_size playlist_index, const bit_array & pl_mask, t_size pl_count) {
		TRACK_CALL_TEXT("queue_helpers::queue_insert_items - bit array");

		size_t cinserts = 0;

		static_api_ptr_t<playlist_manager> playlist_api;
		pfc::list_t<t_playback_queue_item> queue;
		playlist_api->queue_get_contents(queue);
		t_size queue_orig_count = queue.get_count();

		NoRefreshScope tmp;

		if (insertionPoint <= queue_orig_count) {
			playlist_api->queue_flush();
		}

		for(t_size i = 0; i < insertionPoint; i++) {
			add_queue_item(queue[i]);
		}

		t_size track_ndx = pl_mask.find_first(true, 0, pl_count);
		while(track_ndx < pl_count) {
			playlist_api->queue_add_item_playlist(playlist_index, track_ndx);
			cinserts++;
			track_ndx = pl_mask.find_next(true, track_ndx, pl_count);
		}

		for(t_size i = insertionPoint; i < queue_orig_count; i++) {
			add_queue_item(queue[i]);
		}
		return cinserts;
	}

	// Inserts metadb_handles to a given point
	static void queue_insert_items(t_size insertionPoint, const pfc::list_base_const_t<metadb_handle_ptr> & p_data) {
		TRACK_CALL_TEXT("queue_helpers::queue_insert_items");
		static_api_ptr_t<playlist_manager> playlist_api;
		pfc::list_t<t_playback_queue_item> queue;
		t_size new_data_count = p_data.get_count();
		playlist_api->queue_get_contents(queue);
		t_size queue_orig_count = queue.get_count();

		//disable updating queue UIs
		NoRefreshScope tmp;

		playlist_api->queue_flush();

		for(t_size i = 0; i < insertionPoint; i++) {
			add_queue_item(queue[i]);
		}

		for(t_size i = 0; i < new_data_count; i++) {
			playlist_api->queue_add_item(p_data[i]);
		}

		for(t_size i = insertionPoint; i < queue_orig_count; i++) {
			add_queue_item(queue[i]);
		}
	}

	/**
	* Add items to queue at a specific position
	*/
	static void queue_add_items(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr> & p_data) {

		TRACK_CALL_TEXT("queue_helpers::queue_add_items p_base overload");
		static_api_ptr_t<playlist_manager> playlist_api;
		t_size dataSize = p_data.get_size();
		t_size queue_old_size = playlist_api->queue_get_count();
		t_size queue_new_size = queue_old_size + dataSize;

		PFC_ASSERT( p_base <= queue_old_size);

		NoRefreshScope tmp;

		// 1. Add items to the end of queue
		queue_push_items(p_data, false);

		pfc::bit_array_bittable ori_mask(bit_array_false(), queue_new_size);

		// 2. Reorder items in the queue so that are in the correct place

		for(t_size j = queue_old_size; j < queue_new_size; j++) {
			ori_mask.set(j, true);
		}

		size_t itemCount = p_data.get_count();
		size_t insertMark = p_base;
		pfc::array_t<size_t> out; out.resize(queue_new_size);
		for (auto i = 0; i < queue_new_size; i++) { out[i] = i; }

		bool res = pfc::create_drop_permutation(out.get_ptr(), queue_new_size, ori_mask, p_base);

		queue_reorder(out.get_ptr());
	}

	static void queue_push_items(const pfc::list_base_const_t<metadb_handle_ptr> & p_data, bool refreshScopeEnabled = true) {

		TRACK_CALL_TEXT("queue_helpers::queue_add_items");

		static_api_ptr_t<playlist_manager> playlist_api;

		//disable updating queue UIs
		NoRefreshScope tmp(refreshScopeEnabled);

		for(t_size j = 0; j < p_data.get_size(); j++)
		{
			//..
			playlist_api->queue_add_item(p_data[j]);
			//..
		}
	}

	static void queue_reorder(const t_size* p_order) {
		TRACK_CALL_TEXT("queue_helpers::queue_reorder");
		static_api_ptr_t<playlist_manager> playlist_api;
		pfc::list_t<t_playback_queue_item> queue;

		playlist_api->queue_get_contents(queue);
		queue.reorder(p_order);

		//disable updating queue UIs
		NoRefreshScope tmp;

		playlist_api->queue_flush();

		queue.for_each(add_queue_item);
	}

	static void extract_metadb_handles(const pfc::list_t<t_playback_queue_item>& queue_items, metadb_handle_list_ref p_out) {
		TRACK_CALL_TEXT("queue_helpers::extract_metadb_handles");
		t_size queue_items_count = queue_items.get_count();
		for(t_size i = 0; i < queue_items_count; i++) {
			p_out.add_item(queue_items[i].m_handle);
		}
	}
private:
	static void add_queue_item(t_playback_queue_item item) {
		TRACK_CALL_TEXT("queue_helpers::add_queue_item");
		if(item.m_item == pfc::infinite_size) {
			static_api_ptr_t<playlist_manager>()->queue_add_item(item.m_handle);
		} else {
			static_api_ptr_t<playlist_manager>()->queue_add_item_playlist(item.m_playlist, item.m_item);
		}
	}
};




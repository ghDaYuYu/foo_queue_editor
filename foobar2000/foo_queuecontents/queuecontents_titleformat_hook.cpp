#include "stdafx.h"
#include "queuecontents_titleformat_hook.h"

void queuecontents_titleformat_hook::setData(t_size queue_index, t_playback_queue_item queue_item) {
	// We consider the item "dead" if either the playlist or item index is undefined (i.e. infinite)
	// When deleting queued items, foobar seems to return queue_items with the other index pfc:infinite_size and the other finite!
	if(queue_item.m_playlist == pfc::infinite_size || queue_item.m_item == pfc::infinite_size) {
		queue_item.m_item = pfc::infinite_size;
		queue_item.m_playlist = pfc::infinite_size;
	}
	m_queue_item = queue_item;

	t_size queue_index_zero_based = queue_index - 1;

	static_api_ptr_t<playlist_manager> playlist_api;
	t_size queue_count = playlist_api->queue_get_count();

	t_size playlist_total;
	if(queue_item.m_playlist != pfc::infinite_size) {
		playlist_total = playlist_api->playlist_get_item_count(queue_item.m_playlist);
	} else {
		playlist_total = pfc::infinite_size;
	}	

	m_queue_list_hook.setData(queue_index_zero_based, queue_count);
	m_list_hook.setData(queue_item.m_item, playlist_total);
}


t_playback_queue_item queuecontents_titleformat_hook::getQueueItem() {
	return m_queue_item;
}

bool queuecontents_titleformat_hook::process_field(titleformat_text_out * p_out,
		const char * 	p_name,
		t_size 	p_name_length,
		bool & 	p_found_flag) {
		
		p_found_flag = false;
		if(m_list_hook.process_field(p_out, p_name, p_name_length, p_found_flag)) return true;
		
		p_found_flag = false;
		if(m_queue_list_hook.process_field(p_out, p_name, p_name_length, p_found_flag)) return true;
		
		p_found_flag = false;
		if(m_queue_item.m_playlist != pfc::infinite_size && stricmp_utf8_ex(p_name,p_name_length,FIELD_PLAYLIST_NAME,~0) == 0) {
			pfc::string8 playlist_name;
			static_api_ptr_t<playlist_manager> playlist_api;
			playlist_api->playlist_get_name(m_queue_item.m_playlist, playlist_name);
			p_out->write(titleformat_inputtypes::unknown, playlist_name, playlist_name.length());
			p_found_flag = true;
			return true;
		}

		p_found_flag = false;
		return false;
}
		

bool queuecontents_titleformat_hook::process_function(titleformat_text_out * p_out,
		const char * p_name,
		t_size p_name_length,
		titleformat_hook_function_params * p_params,
		bool & p_found_flag) {
		// Do not process functions
		return false;
}
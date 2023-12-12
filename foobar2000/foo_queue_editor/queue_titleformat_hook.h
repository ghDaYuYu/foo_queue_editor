#pragma once
#include "stdafx.h"

#define FIELD_QUEUE_INDEX "queue_index"
#define FIELD_QUEUE_TOTAL "queue_total"
#define FIELD_PLAYLIST_NAME "playlist_name"

// Copied and slightly modified from SDK/titleformat.h
class titleformat_hook_impl_list_custom : public titleformat_hook {
public:
	titleformat_hook_impl_list_custom(t_size p_index /* zero-based! */,t_size p_total, char* list_index, char* list_total) : 
	  m_index(p_index), m_total(p_total), m_list_index(list_index), m_list_total(list_total) {}
	
	bool process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag) {
		if (
			m_index != pfc::infinite_size && stricmp_utf8_ex(p_name,p_name_length,m_list_index,~0) == 0
			) {
			p_out->write_int_padded(titleformat_inputtypes::unknown,m_index+1, m_total);
			p_found_flag = true; return true;
		} else if (
			m_total != pfc::infinite_size && stricmp_utf8_ex(p_name,p_name_length,m_list_total,~0) == 0
			) {
			p_out->write_int(titleformat_inputtypes::unknown,m_total);
			p_found_flag = true; return true;			
		} else {
			p_found_flag = false; return false;
		}
	}

	void setData(t_size p_index /* zero-based! */,t_size p_total) {
		m_index = p_index;
		m_total = p_total;
	}

	bool process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag) {return false;}

private:
	t_size m_index, m_total;
	char* m_list_index;
	char* m_list_total;
};

class queuecontents_titleformat_hook : public titleformat_hook {
public:	
	queuecontents_titleformat_hook(t_size queue_index /*1-based!*/ = pfc::infinite_size, t_playback_queue_item queue_item = t_playback_queue_item()) : 
	  m_list_hook(pfc::infinite_size, pfc::infinite_size, "list_index", "list_total"), 
	  m_queue_list_hook(pfc::infinite_size, pfc::infinite_size, FIELD_QUEUE_INDEX, FIELD_QUEUE_TOTAL) { 
		// setData also initializes list hooks
		setData(queue_index, queue_item);
	}

	virtual bool process_field(titleformat_text_out * 	p_out,
		const char * 	p_name,
		t_size 	p_name_length,
		bool & 	p_found_flag);

	virtual bool process_function(titleformat_text_out * 	p_out,
		const char * 	p_name,
		t_size 	p_name_length,
		titleformat_hook_function_params * 	p_params,
		bool & 	p_found_flag);

	void setData(t_size queue_index, t_playback_queue_item queue_item);

	t_playback_queue_item getQueueItem();

private:
	t_playback_queue_item m_queue_item;
	titleformat_hook_impl_list_custom m_list_hook; // %list_index% and %list_total%
	titleformat_hook_impl_list_custom m_queue_list_hook; // %queue_index% and %queue_total%
};

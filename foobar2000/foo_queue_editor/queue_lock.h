#pragma once
#include "config.h"
#include "config.h"
#include "queue_helpers.h"

// Forward-declare
class playlist_updater;

class queuecontents_lock : public playlist_lock {
public:

	//! Queries whether specified item insertiion operation is allowed in the locked playlist.
	//! @param p_base Index from which the items are being inserted.
	//! @param p_data Items being inserted.
	//! @param p_selection Caller-requested selection state of items being inserted.
	//! @returns True to allow the operation, false to block it.
	bool query_items_add(t_size p_base, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection);
	bool query_items_reorder(const t_size * p_order, t_size p_count);
	bool query_items_remove(const bit_array & p_mask, bool p_force);
	bool query_item_replace(t_size p_index,const metadb_handle_ptr & p_old,const metadb_handle_ptr & p_new);
	bool query_playlist_rename(const char * p_new_name,t_size p_new_name_len);
	bool query_playlist_remove();
	bool execute_default_action(t_size p_item);
	void on_playlist_index_change(t_size p_new_index);
	void on_playlist_remove();
	void get_lock_name(pfc::string_base & p_out);
	void show_ui();
	//! Queries which actions the lock filters. The return value must not change while the lock is registered with playlist_manager. The return value is a combination of one or more filter_* constants.
	t_uint32 get_filter_mask();
	
	static void install_lock();
	static void uninstall_lock();

	static t_size find_or_create_playlist();
	static t_size find_playlist();

	// Do we allow Queue->Playlist (Refresh) and Playlist->Queue (whether playlist updates induce queue changes) updates?
	// This is used temporarily to disable loop Queue->Playlist->Queue->...
	static bool updateQueuePlaylist;

private:
	static void clean_lock_playlist();
	static playlist_updater m_playlist_updater;
	static service_ptr_t<queuecontents_lock> plLock;
	
};

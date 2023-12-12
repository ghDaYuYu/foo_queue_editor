#include "stdafx.h"
#include "config.h"
#include "queue_persistence.h"
#include "initquit.h"

void queue_initquit::on_init() 
{
	if (cfg_load_init) {

		static_api_ptr_t<playlist_manager> playlist_api;
		pfc::list_t<t_playback_queue_item> queue;
		playlist_api->queue_get_contents(queue);

		if (!queue.get_count()) {
			queue_persistence qp;
			qp.readDataFileJSON(true);
			window_manager::VisualsChanged();
		}
	}
	if(cfg_playlist_enabled) {
		queue_lock::install_lock();
	}

	watcher.attach(new playlist_queue_item_watcher());
}

void queue_initquit::on_quit() {

	if (cfg_save_quit) {
		queue_persistence qp;
		qp.writeDataFile();
	}

	queue_lock::uninstall_lock();
	watcher.release();
}
static initquit_factory_t<queue_initquit> initquitter;

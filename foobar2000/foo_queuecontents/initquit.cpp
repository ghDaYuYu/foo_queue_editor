#include "stdafx.h"
#include "config.h"
#include "queue_persistence.h"
#include "initquit.h"

void queuecontents_initquit::on_init() 
{
	if(cfg_playlist_enabled) {
		queuecontents_lock::install_lock();
	}

	watcher.attach(new playlist_queue_item_watcher());
}

void queuecontents_initquit::on_quit() {

	if (cfg_save_quit) {
		queue_persistence qp;
		qp.writeDataFile();
	}

	queuecontents_lock::uninstall_lock();
	watcher.release();
}

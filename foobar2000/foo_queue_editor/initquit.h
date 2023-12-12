#pragma once 

#include "stdafx.h"
#include "config.h"
#include "playlist_updater.h"
#include "queuecontents_lock.h"
#include "playlist_queue_item_watcher.h"

class queuecontents_initquit : public initquit
{
public:

	void on_init();
	void on_quit();

private:
	pfc::ptrholder_t<playlist_queue_item_watcher> watcher;
};

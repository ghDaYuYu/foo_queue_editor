#pragma once 

#include "stdafx.h"
#include "config.h"
#include "playlist_updater.h"
#include "queue_lock.h"
#include "playlist_queue_item_watcher.h"

class queue_initquit : public initquit
{
public:

	void on_init();
	void on_quit();

private:
	pfc::ptrholder_t<playlist_queue_item_watcher> watcher;
};

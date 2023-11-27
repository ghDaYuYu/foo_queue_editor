#pragma once

#include "window_manager.h"
#include "queue_helpers.h"
#include "queuecontents_lock.h"
#include "config.h"

// class that responds to Refresh() function and updates the playlist to match queue contents
class playlist_updater : public window_manager_window {
public:
	void Refresh();
};

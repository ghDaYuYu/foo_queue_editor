#pragma once
#include "stdafx.h"
#include "ui_element_configuration.h"
#include "window_manager.h"

// Interface for reading and writing configuration
class ui_element_configuration_host {
public:
	// persists current configuration
	virtual void save_configuration() = 0;
	// gets access to current configuration
	virtual void get_configuration(ui_element_settings** configuration) = 0;
};

class ui_element_container : public window_manager_window {
public:
	virtual bool is_popup() = 0;
	virtual void close() = 0;
	virtual bool is_dui() = 0;
};

class ui_element_host : public ui_element_configuration_host, public ui_element_container {
	//..
};

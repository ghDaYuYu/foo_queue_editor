#include "stdafx.h"
#include "config.h"
#include "window_manager.h"
#include "queue_persistence.h"

// {E445BE2A-2150-470D-8F5D-5A0D8654FA7D}
static const GUID guid_menu_group_queue_editor_id =
{ 0xe445be2a, 0x2150, 0x470d, { 0x8f, 0x5d, 0x5a, 0xd, 0x86, 0x54, 0xfa, 0x7d } };

// {883C2192-6628-4D42-AEFB-F6933BF49FE4}
static const GUID guid_store = { 0x883c2192, 0x6628, 0x4d42, { 0xae, 0xfb, 0xf6, 0x93, 0x3b, 0xf4, 0x9f, 0xe4 } };

// {79F8F425-D128-4F55-8B77-9AF6A657F1B6}
static const GUID guid_restore = { 0x79f8f425, 0xd128, 0x4f55, { 0x8b, 0x77, 0x9a, 0xf6, 0xa6, 0x57, 0xf1, 0xb6 } };

static const pfc::string8 store_label = "Save session queue";
static const pfc::string8 restore_label = "Restore last session queue";

// static group

static mainmenu_group_popup_factory g_mainmenu_group(guid_menu_group_queue_editor_id, mainmenu_groups::playback, mainmenu_commands::sort_priority_dontcare, COMPONENT_NAME_HC);

class mainmenu_commands_basic_queue_editor : public mainmenu_commands {

public:

	enum {
		cmd_store = 0,
		cmd_restore,
		cmd_total
	};

	virtual t_uint32 get_command_count() override {
		return cmd_total;
	}

	GUID get_parent() {
		return guid_menu_group_queue_editor_id;
	}

	GUID get_command(unsigned p_index) override {

		switch (p_index) {
		case 0:
			return guid_store;
		case 1:
			return guid_restore;
		}
		return GUID_NULL;
	}

	void get_item_name(t_uint32 p_index, pfc::string_base& p_out) {

		switch (p_index) {
		case cmd_store:
			p_out = store_label.c_str();
			break;
		case cmd_restore:
			p_out = restore_label.c_str();
			break;

		default: uBugCheck();
		}
	}

	void get_name(t_uint32 p_index, pfc::string_base& p_out) {

		switch (p_index) {
		case cmd_store:
			p_out = store_label.c_str();
			break;
		case cmd_restore:
			p_out = restore_label.c_str();
			break;

		default: uBugCheck();
		}
	}

	bool get_description(t_uint32 p_index, pfc::string_base& p_out) {

		switch (p_index) {
		case cmd_store: p_out = "Stores the current session queue"; return true;
		case cmd_restore: p_out = "Restores the last saved session queue."; return true;

		default: uBugCheck();
		}
	}
	
	virtual bool get_display(t_uint32 p_index, pfc::string_base & p_text, t_uint32 & p_flags) override {

		p_flags = 0;
		switch (p_index) {
		case cmd_store:
			//todo can store
			break;
		case cmd_restore: {
			//todo can restore
			break;
		}
		}
		get_item_name(p_index, p_text);
		return true;
	}

	void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback) {

		switch (p_index) {
		case cmd_store: {

			queue_persistence qp;
			qp.writeDataFile(true);
			break;
		}
		case cmd_restore: {

			queue_persistence qp;
			qp.readDataFileJSON(true);
			window_manager::VisualsChanged();
			break;
		}

		default: uBugCheck();
		}
	}

};

static mainmenu_commands_factory_t<mainmenu_commands_basic_queue_editor> g_mainmenu_commands_basic_bookmark_factory;
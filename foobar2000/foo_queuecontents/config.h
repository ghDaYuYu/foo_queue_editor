#pragma once
#include "stdafx.h"
#include "cfg_objMapWithDefault.h"

// definition of a column, these are referenced from ui elements
class ui_column_definition {
public:
	ui_column_definition(pfc::string8 name, pfc::string8 pattern, t_int alignment):
	  m_name(name), m_pattern(pattern), m_alignment(alignment) {}
	
	ui_column_definition(){};

	pfc::string8 m_name;
	pfc::string8 m_pattern;
	t_int m_alignment;
};

FB2K_STREAM_WRITER_OVERLOAD(ui_column_definition);
FB2K_STREAM_READER_OVERLOAD(ui_column_definition);

// Must correspond to listview alignment definitions, so don't change these!
static const t_int COLUMN_ALIGNMENT_LEFT = LVCFMT_LEFT;
static const t_int COLUMN_ALIGNMENT_CENTER = LVCFMT_CENTER;
static const t_int COLUMN_ALIGNMENT_RIGHT = LVCFMT_RIGHT;


// {3526DFD2-C5DB-4541-96BE-B4F99FE06C07}
static const GUID guid_cfg_playlist_enabled =
{ 0x3526dfd2, 0xc5db, 0x4541, { 0x96, 0xbe, 0xb4, 0xf9, 0x9f, 0xe0, 0x6c, 0x7 } }; //mod

// {08415AFC-3FC2-425F-91B2-F583528839C0}
static const GUID guid_cfg_playlist_name =
{ 0x8415afc, 0x3fc2, 0x425f, { 0x91, 0xb2, 0xf5, 0x83, 0x52, 0x88, 0x39, 0xc0 } }; //mod

// Legacy setting.
// {8E696057-24A6-4D6E-8EC7-E9E13598CCB0}
//static const GUID guid_cfg_ui_format = 
//{ 0x8e696057, 0x24a6, 0x4d6e, { 0x8e, 0xc7, 0xe9, 0xe1, 0x35, 0x98, 0xcc, 0xb0 } };

// {F5DA3648-8655-43F9-BC69-67C7E56E574D}
static const GUID guid_cfg_ui_columns =
{ 0xf5da3648, 0x8655, 0x43f9, { 0xbc, 0x69, 0x67, 0xc7, 0xe5, 0x6e, 0x57, 0x4d } }; //mod

static const int default_cfg_playlist_enabled = 0;
static const char* default_cfg_playlist_name = "Queue";

// default columns
static t_storage_impl<long, ui_column_definition> default_cfg_ui_columns[] = {
	t_storage_impl<long, ui_column_definition>::create(0, ui_column_definition(pfc::string8("Queue Item"), pfc::string8("%queue_index% - %title%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(1, ui_column_definition(pfc::string8("Queue Index"), pfc::string8("%queue_index%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(2, ui_column_definition(pfc::string8("Queue Total"), pfc::string8("%queue_total%"), COLUMN_ALIGNMENT_LEFT)), 
	t_storage_impl<long, ui_column_definition>::create(3, ui_column_definition(pfc::string8("Album Artist"), pfc::string8("%album artist%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(4, ui_column_definition(pfc::string8("Album"), pfc::string8("%album%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(5, ui_column_definition(pfc::string8("Title"), pfc::string8("%title%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(6, ui_column_definition(pfc::string8("Track No"), pfc::string8("%tracknumber%"), COLUMN_ALIGNMENT_LEFT)),	
	t_storage_impl<long, ui_column_definition>::create(7, ui_column_definition(pfc::string8("Codec"), pfc::string8("%codec%"), COLUMN_ALIGNMENT_LEFT))
};


extern cfg_string cfg_playlist_name;
extern cfg_bool cfg_playlist_enabled;

extern cfg_objMapWithDefault< pfc::map_t<long, ui_column_definition> > cfg_ui_columns;
#pragma once
#include "cfg_objMapWithDefault.h"

using namespace cfg_var_legacy;

// definition of a column, these are referenced from ui elements
class ui_column_definition {
public:
	ui_column_definition(pfc::string8 name, pfc::string8 pattern, t_int alignment):
	  m_name(name), m_pattern(pattern), m_alignment(alignment) {}
	
	ui_column_definition(){};

	pfc::string8 m_name;
	pfc::string8 m_pattern;
	t_int m_alignment = 0;
};

typedef pfc::map_t<long, ui_column_definition>::const_iterator cfg_ui_iterator;

FB2K_STREAM_WRITER_OVERLOAD(ui_column_definition);
FB2K_STREAM_READER_OVERLOAD(ui_column_definition);

// Must correspond to listview alignment definitions, so don't change these!
static const t_int COLUMN_ALIGNMENT_LEFT = LVCFMT_LEFT;
static const t_int COLUMN_ALIGNMENT_CENTER = LVCFMT_CENTER;
static const t_int COLUMN_ALIGNMENT_RIGHT = LVCFMT_RIGHT;

// {B2AA15B4-4F6B-495A-B45C-C5ADAEE863FB}
static const GUID guid_cfg_show_header =
{ 0xb2aa15b4, 0x4f6b, 0x495a, { 0xb4, 0x5c, 0xc5, 0xad, 0xae, 0xe8, 0x63, 0xfb } }; //new

// {4E149626-29BA-42D0-831B-EE9E8FBA23C8}
static const GUID guid_cfg_load_init =
{ 0x4e149626, 0x29ba, 0x42d0, { 0x83, 0x1b, 0xee, 0x9e, 0x8f, 0xba, 0x23, 0xc8 } };

// {816465C4-2B6B-4556-A7F6-C7AA9179DECA}
static const GUID guid_cfg_save_quit =
{ 0x816465c4, 0x2b6b, 0x4556, { 0xa7, 0xf6, 0xc7, 0xaa, 0x91, 0x79, 0xde, 0xca } };

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

static const bool default_cfg_load_init = false;
static const bool default_cfg_save_quit = false;
static const bool default_cfg_show_header = true;
static const int default_cfg_playlist_enabled = 0;
static const char* default_cfg_playlist_name = "Queue";

// default columns
static t_storage_impl<long, ui_column_definition> default_cfg_ui_columns[] = {
	t_storage_impl<long, ui_column_definition>::create(0, ui_column_definition(pfc::string8("Queue item"), pfc::string8("%queue_index% - %title%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(1, ui_column_definition(pfc::string8("Queue index"), pfc::string8("%queue_index%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(2, ui_column_definition(pfc::string8("Queue total"), pfc::string8("%queue_total%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(3, ui_column_definition(pfc::string8("Album Artist"), pfc::string8("%album artist%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(4, ui_column_definition(pfc::string8("Album"), pfc::string8("%album%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(5, ui_column_definition(pfc::string8("Title"), pfc::string8("%title%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(6, ui_column_definition(pfc::string8("Track No"), pfc::string8("%tracknumber%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(7, ui_column_definition(pfc::string8("Codec"), pfc::string8("%codec%"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(8, ui_column_definition(pfc::string8("Playlist"), pfc::string8("$if2(%playlist_name%,)"), COLUMN_ALIGNMENT_LEFT)),
	t_storage_impl<long, ui_column_definition>::create(9, ui_column_definition(pfc::string8("Playlist index"), pfc::string8("$if2(%list_index%,)"), COLUMN_ALIGNMENT_LEFT))
};

extern cfg_var_legacy::cfg_bool cfg_show_header;
extern cfg_var_legacy::cfg_bool cfg_load_init;
extern cfg_var_legacy::cfg_bool cfg_save_quit;
extern cfg_var_legacy::cfg_string cfg_playlist_name;
extern cfg_var_legacy::cfg_bool cfg_playlist_enabled;

extern cfg_objMapWithDefault< pfc::map_t<long, ui_column_definition> > cfg_ui_columns;

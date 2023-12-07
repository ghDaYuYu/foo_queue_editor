#include "stdafx.h"
#include "config.h"

using namespace cfg_var_legacy;

cfg_var_legacy::cfg_bool cfg_show_header(guid_cfg_show_header, default_cfg_show_header);
cfg_var_legacy::cfg_bool cfg_save_quit(guid_cfg_save_quit, default_cfg_save_quit);
cfg_var_legacy::cfg_string cfg_playlist_name(guid_cfg_playlist_name, default_cfg_playlist_name);
cfg_var_legacy::cfg_bool cfg_playlist_enabled(guid_cfg_playlist_enabled, default_cfg_playlist_enabled);
cfg_objMapWithDefault< pfc::map_t<long, ui_column_definition> > cfg_ui_columns(guid_cfg_ui_columns, default_cfg_ui_columns);

FB2K_STREAM_WRITER_OVERLOAD(ui_column_definition) {
	int version = COMPONENTCONFIGVERSION;
	stream << version;

	stream << value.m_name;
	stream << value.m_pattern;
	stream << value.m_alignment;

	return stream;
}

FB2K_STREAM_READER_OVERLOAD(ui_column_definition) {
	try {
		// For backwards-compatibility, even though it has overhead
		int version;
		stream >> version;

		stream >> value.m_name;
		stream >> value.m_pattern;
		stream >> value.m_alignment;

	} catch(exception_io_data_truncation e) {
		console::warning(COMPONENT_NAME_HC ": Failed to read ui column definition.");
		value.m_name = pfc::string8("INVALID");
		value.m_pattern = pfc::string8("INVALID");
		value.m_alignment = COLUMN_ALIGNMENT_LEFT;
	}

	return stream;
}
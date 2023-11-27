#pragma once
#include "stdafx.h"
#include "config.h"

// settings of one column in a ui element

class ui_column_settings {
public:

	ui_column_settings(long column_id, int column_width = 100, t_uint32 order = ~0):
		m_id(column_id), m_column_width(column_width), m_order(order) {

	}

	ui_column_settings(){}

	// width of this column, should be kept updated all the time
	long m_id;
	int m_column_width;
	t_uint32 m_order = ~0;
};

// settings for single ui leement
class ui_element_settings {

public:

	ui_element_settings() : m_show_column_header(true), m_relative_column_widths(false),
		m_control_width(100), m_columns(), m_border(WS_EX_STATICEDGE) {}

	inline static int in_version;

	int m_version = 5;

	// Whether to show colum header
	bool m_show_column_header;

	// whether column widths should be understood relative to ui control width
	bool m_relative_column_widths;

	// width of the control
	int m_control_width;

	// order is from left-to-right
	pfc::list_t<ui_column_settings> m_columns;

	// WS_EX_STATICEDGE (dui/cui), WS_EX_CLIENTEDGE (cui) or 0=no border (cui)
	unsigned long m_border;

	bool column_exists(long id);
	t_size column_index_from_id(long id);

};

FB2K_STREAM_WRITER_OVERLOAD(ui_column_settings) {
	stream << value.m_id;
	stream << value.m_column_width;

	stream << value.m_order;

	return stream;
}

FB2K_STREAM_READER_OVERLOAD(ui_column_settings) {
	stream >> value.m_id;
	stream >> value.m_column_width;
	if (ui_element_settings::in_version >= 5) {
		stream >> value.m_order;
	}
	return stream;
}

FB2K_STREAM_READER_OVERLOAD(ui_element_settings) {

	ui_element_settings::in_version = -1;

	try {
		stream >> ui_element_settings::in_version;

		stream >> value.m_show_column_header;
		stream >> value.m_relative_column_widths;
		stream >> value.m_control_width;

		value.m_columns.remove_all();
		stream.read_array(value.m_columns);

		if(value.m_version >= 4) {
			DEBUG_PRINT << "Reading m_border since version >= 4";
			stream >> value.m_border;
		} else {
			DEBUG_PRINT << "Defaulting m_border=WS_EX_STATICEDGE since version < 4";
			value.m_border = WS_EX_STATICEDGE; // DUI default border
		}

		if (value.m_version < 5) {
			uint32_t order = 0;
			for (auto &w : value.m_columns) {
				w.m_order = order;
				++order;
			}
		}

		DEBUG_PRINT << "Reading ui element settings:";
		DEBUG_PRINT << "Config version: " << value.m_version;
		DEBUG_PRINT << "m_show_column_header: " << value.m_show_column_header;
		DEBUG_PRINT << "m_relative_column_widths: " << value.m_relative_column_widths;
		DEBUG_PRINT << "m_control_width: " << value.m_control_width;
		DEBUG_PRINT << "m_columns (size): " << value.m_columns.get_count();
		DEBUG_PRINT << "m_border: " << ((t_uint64) value.m_border);

	} catch(exception_io_data_truncation e) {
		// We do not concern the user for nothing. If didn't even
		// succeed reading the version it *probably* means
		// we are constructing new settings
		if(value.m_version != -1) {
			console::warning(COMPONENT_NAME_HC ": Failed to read ui element settings. Reseting settings.");
		} else {
			console::info(COMPONENT_DESC ": Constructing default settings for ui element.");
		}
		value.m_show_column_header = true;
		value.m_relative_column_widths = false;
		value.m_control_width = 100;
		value.m_columns.remove_all();
		if(cfg_ui_columns.first().is_valid()) {
			value.m_columns.add_item(ui_column_settings(cfg_ui_columns.first()->m_key));
		} else {
			// object.m_columns is empty
		}
		value.m_border = WS_EX_STATICEDGE;
	}

	return stream;
}

FB2K_STREAM_WRITER_OVERLOAD(ui_element_settings) {
	// For backwards-compatibility
	int version = COMPONENTCONFIGVERSION;
	stream << version;

	stream << value.m_show_column_header;
	stream << value.m_relative_column_widths;
	stream << value.m_control_width;

	DEBUG_PRINT << "Writing ui element settings:";
	DEBUG_PRINT << "Config version: " << version;
	DEBUG_PRINT << "m_show_column_header: " << value.m_show_column_header;
	DEBUG_PRINT << "m_relative_column_widths: " << value.m_relative_column_widths;
	DEBUG_PRINT << "m_control_width: " << value.m_control_width;
	DEBUG_PRINT << "m_columns (size): " << value.m_columns.get_count();

	stream.write_array(value.m_columns);	
	stream.write_int(value.m_border);
	
	return stream;
}




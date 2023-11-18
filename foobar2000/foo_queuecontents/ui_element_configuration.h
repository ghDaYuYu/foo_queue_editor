#pragma once
#include "stdafx.h"
#include "config.h"

// settings of one column in a ui element

class ui_column_settings {
public:
	ui_column_settings(long column_id, int column_width = 100):
	   m_id(column_id), m_column_width(column_width) {}	

	   ui_column_settings(){}

	// width of this column, should be kept updated all the time
	int m_column_width;
	long m_id;
};

// settings for single ui leement
class ui_element_settings {
public:
	ui_element_settings() : m_show_column_header(true), m_relative_column_widths(false), m_control_width(100), m_columns(), m_border(WS_EX_STATICEDGE) {}
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
	
	return stream;
}

FB2K_STREAM_READER_OVERLOAD(ui_column_settings) {
	stream >> value.m_id;
	stream >> value.m_column_width;
	return stream;
}


FB2K_STREAM_READER_OVERLOAD(ui_element_settings) {	
	int version = -1;
	try {				
		stream >> version;

		stream >> value.m_show_column_header;
		stream >> value.m_relative_column_widths;
		stream >> value.m_control_width;

		value.m_columns.remove_all();
		stream.read_array(value.m_columns);

		if(version >= 4) {
			DEBUG_PRINT << "Reading m_border since version >= 4";
			stream >> value.m_border;
		} else {
			DEBUG_PRINT << "Defaulting m_border=WS_EX_STATICEDGE since version < 4";
			value.m_border = WS_EX_STATICEDGE; // DUI default border
		}

		DEBUG_PRINT << "Reading ui element settings:";
		DEBUG_PRINT << "Config version: " << version;
		DEBUG_PRINT << "m_show_column_header: " << value.m_show_column_header;
		DEBUG_PRINT << "m_relative_column_widths: " << value.m_relative_column_widths;
		DEBUG_PRINT << "m_control_width: " << value.m_control_width;
		DEBUG_PRINT << "m_columns (size): " << value.m_columns.get_count();
		DEBUG_PRINT << "m_border: " << ((t_uint64) value.m_border);

	} catch(exception_io_data_truncation e) {
		// We do not concern the user for nothing. If didn't even
		// succeed reading the version it *probably* means
		// we are constructing new settings
		if(version != -1) {
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
	
#if _DEBUG
	t_size col_count = value.m_columns.get_count();
	for(t_size i = 0; i < col_count; i++) {
		auto col_setting_iter = cfg_ui_columns.find(value.m_columns[i].m_id);
		DEBUG_PRINT << " Column #" << i << ": id:" << value.m_columns[i].m_id 
			<< ", width:" << value.m_columns[i].m_column_width 
			<< ", corresponding name. " << col_setting_iter->m_value.m_name;
	}
#endif

	stream.write_array(value.m_columns);	
	stream.write_int(value.m_border);
	
	return stream;
}




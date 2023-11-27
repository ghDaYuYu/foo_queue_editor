#include "stdafx.h"
#include "ui_element_configuration.h"

bool ui_element_settings::column_exists(long field_id) {

	return column_index_from_id(field_id) != SIZE_MAX;
}

t_size ui_element_settings::column_index_from_id(long field_id) {

	t_size count = m_columns.get_count();

	for(t_size w = 0; w < count; w++) {
		if(m_columns[w].m_id == field_id)
			return w;
	}
	return SIZE_MAX;
}

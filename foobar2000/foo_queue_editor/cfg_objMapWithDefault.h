#pragma once
#include "SDK/cfg_var_legacy.h"

using namespace cfg_var_legacy;

class map_utils {	
public:
	template<typename TMap, typename t_storage, unsigned Count >
	static void fill_map_with_values(TMap & map, const t_storage (& values)[Count]) {
		for(t_size i = 0; i < Count; i++) {
			t_storage value = values[i];
			map.set(value.m_key, value.m_value);
		}
	}
};

// extension of cfg_objMap allowing specification of default contents of the map
template<typename TMap>
class cfg_objMapWithDefault : public cfg_objMap<TMap> {
	
public:
	template<typename t_storage, unsigned Count >
	cfg_objMapWithDefault(const GUID & id, const t_storage (& defaults)[Count]) : cfg_objMap(id) {
		map_utils::fill_map_with_values(*this, defaults);
	}
};

// used with cfg_objMapWithDefault
template<typename t_storage_key, typename t_storage_value>
class t_storage_impl {
public:
	static t_storage_impl create(t_storage_key key, t_storage_value value) {
		t_storage_impl returnValue;
		returnValue.m_key = key;
		returnValue.m_value = value;
		return returnValue;
	}

	t_storage_key m_key;
	t_storage_value m_value;
};

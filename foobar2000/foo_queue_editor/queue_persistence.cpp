#include "stdafx.h"

#include <fcntl.h>
#include <io.h>

#include <string>
#include <map>

#include "jansson.h"

#include "config.h"
#include "window_manager.h"
#include "queue_persistence.h"

queue_persistence::queue_persistence() {
	//..
}

queue_persistence::~queue_persistence()
{
	//..
}

std::filesystem::path queue_persistence::genFilePath() {

	pfc::string8 n8_path = core_api::pathInProfile("configuration");
	extract_native_path(n8_path, n8_path);

	std::filesystem::path os_dst = std::filesystem::u8path(n8_path.c_str());

	pfc::string8 filename = core_api::get_my_file_name();
	filename << ".dll.dat";
	os_dst.append(filename.c_str());
	return os_dst;
}

void add_rec(std::vector<json_t*> &vjson, const std::vector<pfc::string8>& vlbl, const t_playback_queue_item & rec) {

	const int nfields = static_cast<int>(vlbl.size());

	static_api_ptr_t<playlist_manager_v5> playlist_api;
	GUID guid = playlist_api->playlist_get_guid(rec.m_playlist);

	std::vector<pfc::string8> vrec = {
		rec.m_handle->get_path(),                                  //path
		std::to_string(rec.m_handle->get_subsong_index()).c_str(), //subsong
		pfc::print_guid(guid).get_ptr(),                           //pl_guid
		std::to_string(rec.m_item).c_str(),                        //pos
	};

	vjson.emplace_back(json_object());

	auto nobj = vjson.size() - 1;
	for (auto wfield = 0; wfield < nfields; wfield++) {
		int res = json_object_set_new_nocheck(vjson[nobj], vlbl[wfield], json_string_nocheck(vrec[wfield].get_ptr()));
	}
}

void queue_persistence::writeDataFile() {
	cmdThFile.add([this] { writeDataFileJSON(); });
}

void queue_persistence::writeDataFileJSON() {

	if (core_api::is_quiet_mode_enabled()) {
		DEBUG_PRINT << "Quiet mode, will not write queue entries to file";
		return;
	}

	static_api_ptr_t<playlist_manager> playlist_api;
	pfc::list_t<t_playback_queue_item> queue;
	playlist_api->queue_get_contents(queue);
	t_size cq = queue.get_count();

	if (!cq) {
		std::filesystem::path os_file = genFilePath();
		if (std::filesystem::exists(os_file)) {
			std::error_code ec;
			std::filesystem::remove(os_file, ec);
		}
		return;
	}

	int jf = -1;

	try {

		DEBUG_PRINT << "Write data file...";

		size_t n_entries = queue.get_count();

		std::vector<json_t*> vjson;
		std::vector<pfc::string8> vlbl = { "path", "subsong", "guid", "pos" };

		//first pass

		for (size_t i = 0; i < n_entries; i++) {
			add_rec(vjson, vlbl, queue[i]);
		}

		//second pass

		json_t* arr_top = json_array();

		for (auto wobj : vjson) {
			auto res = json_array_append(arr_top, wobj);
		}

		// dump object array

		std::filesystem::path os_file = genFilePath();

		jf = _wopen(os_file.wstring().c_str(), _O_CREAT | _O_TRUNC | _O_WRONLY | _O_TEXT/*_O_U8TEXT*/, _S_IWRITE);

		if (jf == -1) {
			foobar2000_io::exception_io e("Open failed on output file");
			throw e;
		}
		auto res = json_dumpfd(arr_top, jf, JSON_INDENT(5));
		_close(jf);

		for (auto w : vjson) {
			free(w);
		}

		DEBUG_PRINT << "Wrote " << std::to_string(n_entries).c_str() << " queue entries to file";
	}
	catch (foobar2000_io::exception_io e) {
		if (jf != -1) {
			_close(jf);
		}
		DEBUG_PRINT << "Could not queue entries to file " << e.what();
	}
	catch (...) {
		if (jf != -1) {
			_close(jf);
		}
		DEBUG_PRINT << "Could not queue entries to file, Unhandled Exception";
	}
}

bool queue_persistence::readDataFileJSON(bool reset) {
	try {

		struct rec_per_t {
			pfc::string8 path;
			t_uint32 subsong = SIZE_MAX;
			GUID guid = pfc::guid_null;
			t_uint32 pos = SIZE_MAX;
		};

		DEBUG_PRINT << "Reading queue entries from file";

		int jf = -1;
		size_t clines = 0;
		std::vector<rec_per_t> temp_data;

		try {

			std::filesystem::path os_file = genFilePath();

			jf = _wopen(os_file.wstring().c_str(), _O_RDONLY);

			if (jf == -1) {
				foobar2000_io::exception_io e("Open failed on input file");
				throw e;
			}

			json_error_t error = {};
			auto json = json_loadfd(jf, JSON_DECODE_ANY, &error);
			_close(jf);

			if (strlen(error.text) && error.line != -1) {
				DEBUG_PRINT << "JSON error: "<< error.text,
						" in line: "   ,error.line,
						", column: "   ,error.column,
						", position: " ,error.position,
						", src: "      ,error.source;
				try {
					if (std::filesystem::file_size(genFilePath().c_str())) {
						DEBUG_PRINT << "JSON error: Creating backup file...";
						pfc::string8 buffer_bak;
						buffer_bak << genFilePath().c_str() << ".bak";
						std::filesystem::copy(genFilePath().c_str(), buffer_bak.c_str(), std::filesystem::copy_options::update_existing);
						DEBUG_PRINT << "JSON error: Backup file created.";
					}
				}
				catch (std::filesystem::filesystem_error const& ex) {
					DEBUG_PRINT << "Error: Creating backup file. " << ex.what();
				}
				//free(error);
			}

			clines = json_array_size(json);

			rec_per_t elem;

			size_t index;
			json_t* js_wobj;

			json_array_foreach(json, index, js_wobj) {

				if (!json_is_object(js_wobj)) break;

				json_t* js_fld;
				{
					elem.path = "";
					js_fld = json_object_get(js_wobj, "path");
					const char* dmp_str = json_string_value(js_fld);
					if (dmp_str) {
						elem.path = pfc::string8(dmp_str);
					}
				}

				{
					elem.subsong = 0;
					js_fld = json_object_get(js_wobj, "subsong");
					const char* dmp_str = json_string_value(js_fld);
					if (dmp_str) {
						elem.subsong = static_cast<t_uint32>(atoi(dmp_str));
					}
				}

				{
					elem.guid = pfc::guid_null;
					js_fld = json_object_get(js_wobj, "guid");
					const char* dmp_str = json_string_value(js_fld);
					if (dmp_str) {
						pfc::string8 tmpguid = pfc::string8(dmp_str);
						elem.guid = pfc::GUID_from_text(tmpguid);
					}
				}

				{
					elem.pos = 0;
					js_fld = json_object_get(js_wobj, "pos");
					const char* dmp_str = json_string_value(js_fld);
					if (dmp_str) {
						elem.pos = static_cast<t_uint32>(atoi(dmp_str));
					}
				}

				temp_data.emplace_back(elem);
				elem = rec_per_t();
			}
		}
		catch (foobar2000_io::exception_io e) {
			if (jf != -1) {
				_close(jf);
			}
			DEBUG_PRINT << "Reading data from file failed - " << e.what();
			return false;
		}
		catch (...) {
			if (jf != -1) {
				_close(jf);
			}
			DEBUG_PRINT << "Reading data from file failed - Unhandled Exception";
			return false;
		}

		DEBUG_PRINT << "Restored " << std::to_string(clines).c_str() << " queue entries from file";

		window_manager::EnableUpdates(false);

		static_api_ptr_t<playlist_manager_v5> playlist_api;
		pfc::list_t<t_playback_queue_item> queue;
		playlist_api->queue_get_contents(queue);

		if (reset) {
			playlist_api->queue_flush();
		}

		auto metadb_ptr = metadb::get();

		for (auto w_it = temp_data.begin(); w_it != temp_data.end(); w_it++) {
			size_t pl_ndx = SIZE_MAX;
			if (!pfc::guid_equal(pfc::guid_null, w_it->guid)) {
				pl_ndx = playlist_api->find_playlist_by_guid(w_it->guid);
			}
			if (pl_ndx != SIZE_MAX) {
				playlist_api->queue_add_item_playlist(pl_ndx, w_it->pos);
			}
			else {
				metadb_handle_ptr track_bm = metadb_ptr->handle_create(w_it->path.c_str(), w_it->subsong);
				playlist_api->queue_add_item(track_bm);
			}
		}

		window_manager::EnableUpdates(true);

	}
	catch (...) {
		//..
	}

	return true;
}


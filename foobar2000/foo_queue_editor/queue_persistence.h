#pragma once
#include <filesystem>
#include "helpers/CmdThread.h"

inline ThreadUtils::cmdThread cmdThFile;

class queue_persistence {

public:
	queue_persistence();
	~queue_persistence();

	void writeDataFile(bool thread_pool);
	void writeDataFileJSON();
	bool readDataFileJSON(bool reset);

	void SetDirty(bool st) {
    	m_is_dirty = st;
    }

private:
	std::filesystem::path genFilePath();

	static bool m_json_loaded;
	static bool m_is_dirty;
};

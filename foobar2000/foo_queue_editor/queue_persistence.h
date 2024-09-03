#pragma once
#include <filesystem>
#include "helpers/CmdThread.h"

inline ThreadUtils::cmdThread cmdThFile;

class queue_persistence {

public:
	queue_persistence();
	~queue_persistence();

	void writeDataFile();
	void writeDataFileJSON();
	bool readDataFileJSON(bool reset);

private:
	std::filesystem::path genFilePath();
};

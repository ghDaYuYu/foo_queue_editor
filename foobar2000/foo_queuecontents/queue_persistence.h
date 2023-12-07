#pragma once
#include "helpers/CmdThread.h"

inline ThreadUtils::cmdThread cmdThFile;

class queue_persistence {

public:
	queue_persistence();
	~queue_persistence();

	void writeDataFile();
	void writeDataFileJSON();
	bool readDataFileJSON();

private:
	pfc::string8 genFilePath();
};

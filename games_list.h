#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct GameInfo {
	uint32_t tid;
	std::string name;
	std::string filename;
};

extern const std::vector<GameInfo> GamesList;
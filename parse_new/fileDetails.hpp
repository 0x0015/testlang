#pragma once
#include <string_view>

struct fileSourceInfo{
	std::string_view filename;
	unsigned int fileLocation;
	unsigned int fileSize;
	void print() const;
};


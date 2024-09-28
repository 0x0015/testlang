#include "fileIO.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>

std::optional<std::string> readFile(const std::string_view filename, bool suppressErrors){
	if(!std::filesystem::exists(filename)){
		if(!suppressErrors)
			std::cerr<<"Unable to open file \""<<filename<<"\""<<std::endl;
		return std::nullopt;
	}
	auto size = std::filesystem::file_size(filename);
	std::ifstream file;
	file.open(std::string(filename)); //no direct call, so cast to std::string
	std::string output;
	output.resize(size);
	file.read(output.data(), size);
	return output;
}


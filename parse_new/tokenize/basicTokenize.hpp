#pragma once
#include "../fileDetails.hpp"
#include <string>
#include <optional>
#include <vector>

struct basicToken{
	std::string val;
	fileSourceInfo file;
	void print() const;
};

std::optional<std::vector<basicToken>> basicTokenizeFile(const std::string_view filename);
std::optional<std::vector<basicToken>> basicTokenizeString(const std::string_view str, const std::string_view filename = "[internal]");


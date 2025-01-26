#pragma once
#include <optional>
#include <string>

std::optional<std::string> readFile(const std::string_view filename, bool suppressErrors = false);
bool writeFile(const std::string_view filename, const std::string_view fileContents);

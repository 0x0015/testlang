#pragma once
#include <optional>
#include <string>

std::optional<std::string> readFile(const std::string_view filename);

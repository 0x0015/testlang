#pragma once
#include "../../ast.hpp"
#include <optional>
#include <span>

class mediumToken;

std::optional<std::pair<ast::type, int>> parseType(std::span<const mediumToken> tokens);


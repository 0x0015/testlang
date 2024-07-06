#pragma once
#include "../../ast.hpp"
#include <optional>
#include <span>

class mediumToken;

std::optional<ast::context> parseContext(std::span<const mediumToken> tokens);


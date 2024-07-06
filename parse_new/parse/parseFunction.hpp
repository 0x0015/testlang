#pragma once
#include "../../ast.hpp"
#include <optional>
#include <span>

class mediumToken;

std::optional<std::pair<ast::function, int>> parseFunction(std::span<const mediumToken> tokens);
std::optional<std::pair<std::vector<ast::function::argument>, int>> parseFunctionArgs(std::span<const mediumToken> tokens);
std::optional<std::pair<std::vector<ast::function::statement>, int>> parseFunctionBody(std::span<const mediumToken> tokens);

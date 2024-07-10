#pragma once
#include "../../ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;

parseRes<ast::function> parseFunction(std::span<const mediumToken> tokens);
parseRes<std::vector<ast::function::argument>> parseFunctionArgs(std::span<const mediumToken> tokens);
parseRes<std::vector<ast::function::statement>> parseFunctionBody(std::span<const mediumToken> tokens);

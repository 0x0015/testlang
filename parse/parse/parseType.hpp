#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;

parseRes<ast::type> parseType(std::span<const mediumToken> tokens);


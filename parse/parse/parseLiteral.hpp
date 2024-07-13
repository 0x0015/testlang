#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;

parseRes<ast::literal> parseLiteral(std::span<const mediumToken> tokens);

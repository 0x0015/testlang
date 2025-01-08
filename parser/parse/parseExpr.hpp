#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;

parseRes<ast::call> parseCall(std::span<const mediumToken> tokens);
parseRes<ast::expr> parseExpr(std::span<const mediumToken> tokens);


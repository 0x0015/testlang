#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;

parseRes<ast::expr> parseExpr(std::span<const mediumToken> tokens);
parseRes<std::vector<ast::expr>> parseCommaSeperatedExprList(std::span<const mediumToken> tokens);

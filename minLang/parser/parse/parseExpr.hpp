#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;
namespace minLang{

parseRes<minLang::ast::expr> parseExpr(std::span<const mediumToken> tokens);
parseRes<std::vector<minLang::ast::expr>> parseCommaSeperatedExprList(std::span<const mediumToken> tokens);
}

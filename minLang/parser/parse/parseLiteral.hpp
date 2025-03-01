#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;
namespace minLang{

parseRes<minLang::ast::literal> parseLiteral(std::span<const mediumToken> tokens);
}

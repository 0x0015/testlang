#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"
#include <unordered_map>

class mediumToken;
namespace minLang{

parseRes<minLang::ast::type> parseType(std::span<const mediumToken> tokens);
}

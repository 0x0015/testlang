#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;
namespace minLang{

parseRes<minLang::ast::function> parseFunction(std::span<const mediumToken> tokens);
parseRes<minLang::ast::function> parseExternalFunction(std::span<const mediumToken> tokens);
parseRes<std::vector<minLang::ast::function::argument>> parseFunctionArgs(std::span<const mediumToken> tokens);
parseRes<std::monostate> looksLikeFunction(std::span<const mediumToken> tokens);
parseRes<std::monostate> looksLikeExternalFunction(std::span<const mediumToken> tokens);
}

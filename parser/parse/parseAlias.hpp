#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;

parseRes<std::monostate> looksLikeAlias(std::span<const mediumToken> tokens);
parseRes<ast::type::alias_type> parseAlias(std::span<const mediumToken> tokens);

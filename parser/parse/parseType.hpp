#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"
#include <unordered_map>

class mediumToken;

inline std::unordered_map<std::string, ast::type::alias_type> parserKnownAliases;
parseRes<ast::type> parseType(std::span<const mediumToken> tokens);


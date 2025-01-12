#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"
#include <unordered_map>
#include "parseTemplate.hpp"

class mediumToken;

inline std::unordered_map<std::string, ast::type::alias_type> parserKnownAliases;
inline std::unordered_map<std::string, aliasTemplate> parserKnownAliasTemplates;

//ugly type to avoid copies
inline std::optional<std::reference_wrapper<const std::vector<std::string>>> parserKnownTemplateTypesDefined = std::nullopt;//used when inside a template

parseRes<ast::type> parseType(std::span<const mediumToken> tokens);


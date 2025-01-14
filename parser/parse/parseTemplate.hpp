#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;

struct aliasTemplate{
	unsigned int numTemplateArgs;
	ast::type::alias_type alias;
	ast::type::alias_type instantiate(const std::vector<ast::type>& replacementTypes) const;
};

void switchTemplatesForReplacements(ast::type& ty, const std::vector<ast::type>& replacementTypes);
parseRes<std::vector<ast::type>> parseTemplateArgs(std::span<const mediumToken> tokens);
parseRes<aliasTemplate> parseAliasTemplate(std::span<const mediumToken> tokens);
parseRes<ast::functionTemplate> parseFunctionTemplate(std::span<const mediumToken> tokens);
parseRes<std::monostate> looksLikeAliasTemplate(std::span<const mediumToken> tokens);
parseRes<std::monostate> looksLikeFunctionTemplate(std::span<const mediumToken> tokens);


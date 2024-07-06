#include "parseType.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "../parseUtil.hpp"

std::optional<std::pair<ast::type, int>> parseType(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(std::holds_alternative<basicToken>(tokens.front().value)){
		const basicToken bToken = std::get<basicToken>(tokens.front().value);
		constexpr std::string_view basicTokNames[] = {"void", "int", "float", "bool"};
		constexpr ast::type::builtin_type basicTokTypes[] = {ast::type::builtin_type::void_type, ast::type::builtin_type::int_type, ast::type::builtin_type::float_type, ast::type::builtin_type::bool_type};
		auto found = findInList(bToken.val, basicTokNames);
		if(!found)
			return std::nullopt;
		return std::make_pair(ast::type(basicTokTypes[*found]), 1);
	}else{
		const auto lToken = std::get<mediumToken::tokList>(tokens.front().value);
		return std::nullopt;
	}
}


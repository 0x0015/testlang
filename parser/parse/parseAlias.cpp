#include "parseAlias.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseType.hpp"
#include "../parseUtil.hpp"
#include "parseLiteral.hpp"
#include "../errorUtil.hpp"
#include "parseExpr.hpp"
#include "parseBlock.hpp"

parseRes<std::monostate> looksLikeAlias(std::span<const mediumToken> tokens){
	//alias
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	int offset = 1;
	if(std::get<basicToken>(tokens.front().value).val == "strict"){
		tokens = tokens.subspan(1);
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		if(std::get<basicToken>(tokens.front().value).val != "alias")
			return std::nullopt;
		offset++;
	}
	tokens = tokens.subspan(1);
	//type
	auto ty = parseType(tokens);
	if(ty){
		tokens = tokens.subspan(ty->toksConsumed);
		offset+=ty->toksConsumed;
	}else{
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		tokens = tokens.subspan(1);
		offset++;
	}
	parse_debug_print("alias parsed type");
	
	//arrow ->
	if(tokens.size() < 2)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "-")
		return std::nullopt;
	tokens = tokens.subspan(1);
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ">")
		return std::nullopt;
	tokens = tokens.subspan(1);
	offset+=2;
	parse_debug_print("alias parsed arrow");

	//name
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	tokens = tokens.subspan(1);
	offset++;
	parse_debug_print("alias parsed name");

	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;
	offset++;

	return makeParseRes(std::monostate{}, offset);
}

parseRes<ast::type::alias_type> parseAlias(std::span<const mediumToken> tokens){
	//alias
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	int offset = 1;
	bool strict = false;
	if(std::get<basicToken>(tokens.front().value).val == "strict"){
		strict = true;
		tokens = tokens.subspan(1);
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		if(std::get<basicToken>(tokens.front().value).val != "alias")
			return std::nullopt;
		offset++;
	}
	tokens = tokens.subspan(1);
	//type
	auto ty = parseType(tokens);
	if(!ty)
		return std::nullopt;
	tokens = tokens.subspan(ty->toksConsumed);
	offset+=ty->toksConsumed;
	parse_debug_print("alias parsed type");
	
	//arrow ->
	if(tokens.size() < 2)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "-")
		return std::nullopt;
	tokens = tokens.subspan(1);
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ">")
		return std::nullopt;
	tokens = tokens.subspan(1);
	offset+=2;
	parse_debug_print("alias parsed arrow");

	//name
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& name = std::get<basicToken>(tokens.front().value).val;
	tokens = tokens.subspan(1);
	offset++;
	parse_debug_print("alias parsed name");

	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;
	offset++;

	auto underlyingType = std::make_shared<ast::type>(ty->val);
	return makeParseRes(ast::type::alias_type{name, underlyingType, strict}, offset);
}

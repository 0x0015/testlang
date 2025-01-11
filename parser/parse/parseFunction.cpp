#include "parseFunction.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseType.hpp"
#include "../parseUtil.hpp"
#include "parseLiteral.hpp"
#include "../errorUtil.hpp"
#include "parseExpr.hpp"
#include "parseBlock.hpp"

parseRes<ast::function> parseFunction(std::span<const mediumToken> tokens){
	//type
	auto ty = parseType(tokens);
	if(!ty)
		return std::nullopt;
	tokens = tokens.subspan(ty->toksConsumed);
	int offset = ty->toksConsumed;
	parse_debug_print("function parsed type");
	//name
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& name = std::get<basicToken>(tokens.front().value).val;
	tokens = tokens.subspan(1);
	offset++;
	parse_debug_print("function parsed name");
	//args
	const auto& args = parseFunctionArgs(tokens);
	if(!args)
		return std::nullopt;

	tokens = tokens.subspan(args->toksConsumed);
	offset += args->toksConsumed;
	parse_debug_print("function parsed args");
	//body
	const auto& body = parseBlock(tokens);
	if(!body)
		return std::nullopt;
	parse_debug_print("function parsed body");
	offset += body->toksConsumed;

	return makeParseRes(ast::function{ty->val, name, args->val, body->val}, offset);
}

//mostly copied from parseFunction
parseRes<ast::function> parseExternalFunction(std::span<const mediumToken> tokens){
	//extern
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "extern")
		return std::nullopt;
	tokens = tokens.subspan(1);
	int offset = 1;
	//type
	auto ty = parseType(tokens);
	if(!ty)
		return std::nullopt;
	tokens = tokens.subspan(ty->toksConsumed);
	offset += ty->toksConsumed;
	parse_debug_print("extern function parsed type");
	//name
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& name = std::get<basicToken>(tokens.front().value).val;
	tokens = tokens.subspan(1);
	offset++;
	parse_debug_print("extern function parsed name");
	//args
	const auto& args = parseFunctionArgs(tokens);
	if(!args)
		return std::nullopt;
	tokens = tokens.subspan(args->toksConsumed);
	offset += args->toksConsumed;

	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;
	offset++;

	parse_debug_print("function parsed args");

	return makeParseRes(ast::function{ty->val, name, args->val, {}, ast::function::positionStatus::external}, offset);
}

parseRes<std::vector<ast::function::argument>> parseFunctionArgs(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& list = std::get<mediumToken::tokList>(tokens.front().value);
	if(list.type != mediumToken::tokList::type_t::PAREN)
		return std::nullopt;
	std::vector<ast::function::argument> output;
	auto lTokens = std::span<const mediumToken>(list.value);
	for(unsigned int i=0;i<list.value.size();i++){
		//type
		if(lTokens.empty())
			break;
		auto ty = parseType(lTokens);
		if(!ty)
			return std::nullopt;
		lTokens = lTokens.subspan(ty->toksConsumed);
		parse_debug_print("function arg parsed type");
		//name
		if(lTokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(lTokens.front().value))
			return std::nullopt;
		const auto& name = std::get<basicToken>(lTokens.front().value);
		lTokens = lTokens.subspan(1);
		parse_debug_print("function arg parsed name");
		//comma
		if(!lTokens.empty()){
			if(!std::holds_alternative<basicToken>(lTokens.front().value))
				return std::nullopt;
			if(std::get<basicToken>(lTokens.front().value).val != ",")
				return std::nullopt;
			lTokens = lTokens.subspan(1);
			parse_debug_print("function arg parsed comma");
		}

		output.push_back(ast::function::argument{ty->val, name.val});
	}

	return makeParseRes(output, 1);
}

parseRes<std::monostate> looksLikeFunction(std::span<const mediumToken> tokens){
	//type
	auto ty = parseType(tokens);
	int offset = 0;
	if(ty){
		tokens = tokens.subspan(ty->toksConsumed);
		offset+=ty->toksConsumed;
		parse_debug_print("function parsed type");
	}else{
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		tokens = tokens.subspan(1);
		offset++;
	}
	//name
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	tokens = tokens.subspan(1);
	offset++;
	parse_debug_print("function parsed name");
	//args
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& argList = std::get<mediumToken::tokList>(tokens.front().value);
	if(argList.type != mediumToken::tokList::type_t::PAREN)
		return std::nullopt;
	tokens = tokens.subspan(1);
	offset++;
	parse_debug_print("function parsed args");
	//body
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& list = std::get<mediumToken::tokList>(tokens.front().value);
	if(list.type != mediumToken::tokList::type_t::CURL_BRACK)
		return std::nullopt;
	tokens = tokens.subspan(1);
	offset++;
	parse_debug_print("function parsed body");

	return makeParseRes(std::monostate{}, offset);
}

//mostly copied from parseFunction
parseRes<std::monostate> looksLikeExternalFunction(std::span<const mediumToken> tokens){
	//extern
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "extern")
		return std::nullopt;
	tokens = tokens.subspan(1);
	int offset = 1;
	//type
	auto ty = parseType(tokens);
	if(ty){
		tokens = tokens.subspan(ty->toksConsumed);
		offset+=ty->toksConsumed;
		parse_debug_print("function parsed type");
	}else{
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		tokens = tokens.subspan(1);
		offset++;
	}
	parse_debug_print("extern function parsed type");
	//name
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	tokens = tokens.subspan(1);
	offset++;
	parse_debug_print("extern function parsed name");
	//args
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& argList = std::get<mediumToken::tokList>(tokens.front().value);
	if(argList.type != mediumToken::tokList::type_t::PAREN)
		return std::nullopt;
	tokens = tokens.subspan(1);
	offset++;

	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;
	offset++;

	parse_debug_print("function parsed args");

	return makeParseRes(std::monostate{}, offset);
}

#include "parseFunction.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseType.hpp"
#include "../parseUtil.hpp"
#include "parseLiteral.hpp"

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
	const auto& body = parseFunctionBody(tokens);
	if(!body)
		return std::nullopt;
	parse_debug_print("function parsed body");
	offset += body->toksConsumed;

	return makeParseRes(ast::function{ty->val, name, args->val, body->val}, offset);
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

parseRes<ast::function::declaration> parseFunctionBodyDeclaration(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	auto tyTry = parseType(tokens);
	if(!tyTry)
		return std::nullopt;
	int outputSize = tyTry->toksConsumed;
	tokens = tokens.subspan(tyTry->toksConsumed);

	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& name = std::get<basicToken>(tokens.front().value).val;
	outputSize++;
	tokens = tokens.subspan(1);

	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;
	outputSize++;
	//don't need to shrink tokens for the last one
	return makeParseRes(ast::function::declaration{tyTry->val, name}, outputSize);
}

parseRes<ast::function::call::argument> parseFunctionCallArg(std::span<const mediumToken> tokens){
	auto literalTry = parseLiteral(tokens);
	if(literalTry){
		return makeParseRes(ast::function::call::argument(literalTry->val), literalTry->toksConsumed);
	}
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& name = std::get<basicToken>(tokens.front().value).val;
	return makeParseRes(ast::function::call::argument(name), 1);
}

parseRes<ast::function::call> parseFunctionBodyCall(std::span<const mediumToken> tokens){
	
	if(tokens.size() < 3)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& name = std::get<basicToken>(tokens.front().value).val;
	tokens = tokens.subspan(1);

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& list = std::get<mediumToken::tokList>(tokens.front().value);
	if(list.type != mediumToken::tokList::type_t::PAREN)
		return std::nullopt;

	tokens = tokens.subspan(1);
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;

	std::vector<ast::function::call::argument> args;
	std::span<const mediumToken> tList(list.value);
	while(!tList.empty()){
		auto argTry = parseFunctionCallArg(tList);
		if(!argTry)
			return std::nullopt;
		args.push_back(argTry->val);
		tList = tList.subspan(argTry->toksConsumed);
		if(!tList.empty()){
			if(!std::holds_alternative<basicToken>(tList.front().value))
				return std::nullopt;
			if(std::get<basicToken>(tList.front().value).val != ",")
				return std::nullopt;
			tList = tList.subspan(1);
		}
	}

	return makeParseRes(ast::function::call{name, args}, 3);
}

parseRes<std::vector<ast::function::statement>> parseFunctionBody(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& list = std::get<mediumToken::tokList>(tokens.front().value);
	if(list.type != mediumToken::tokList::type_t::CURL_BRACK)
		return std::nullopt;
	std::vector<ast::function::statement> output;
	std::span<const mediumToken> toks(list.value);
	while(!toks.empty()){
		auto declTry = parseFunctionBodyDeclaration(toks);
		if(declTry){
			output.push_back(declTry->val);
			toks = toks.subspan(declTry->toksConsumed);
			continue;
		}
		auto callTry = parseFunctionBodyCall(toks);
		if(callTry){
			output.push_back(callTry->val);
			toks = toks.subspan(callTry->toksConsumed);
			continue;
		}
		std::cerr<<"Found structure in function body that did not parse as either variable declaration or function call"<<std::endl;
		break;
	}
	return makeParseRes(output, 1);
}

#include "parseFunction.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseType.hpp"
#include "../parseUtil.hpp"

std::optional<std::pair<ast::function, int>> parseFunction(std::span<const mediumToken> tokens){
	//type
	auto ty = parseType(tokens);
	if(!ty)
		return std::nullopt;
	tokens = tokens.subspan(ty->second);
	int offset = ty->second;
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

	tokens = tokens.subspan(args->second);
	offset += args->second;
	parse_debug_print("function parsed args");
	//body
	const auto& body = parseFunctionBody(tokens);
	if(!body)
		return std::nullopt;
	parse_debug_print("function parsed body");
	offset += body->second;

	return std::make_pair(ast::function{ty->first, name, args->first, body->first}, offset);
}

std::optional<std::pair<std::vector<ast::function::argument>, int>> parseFunctionArgs(std::span<const mediumToken> tokens){
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
		lTokens = lTokens.subspan(ty->second);
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

		output.push_back(ast::function::argument{ty->first, name.val});
	}

	return std::make_pair(output, 1);
}

std::optional<std::pair<ast::function::declaration, int>> parseFunctionBodyDeclaration(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	auto tyTry = parseType(tokens);
	if(!tyTry)
		return std::nullopt;
	int outputSize = tyTry->second;
	tokens = tokens.subspan(tyTry->second);

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
	return std::make_pair(ast::function::declaration{tyTry->first, name}, outputSize);
}

std::optional<std::pair<ast::function::call::argument, int>> parseFunctionCallArg(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& name = std::get<basicToken>(tokens.front().value).val;
	return std::make_pair(ast::function::call::argument(name), 1);
}

std::optional<std::pair<ast::function::call, int>> parseFunctionBodyCall(std::span<const mediumToken> tokens){
	
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
		args.push_back(argTry->first);
		tList = tList.subspan(argTry->second);
		if(!tList.empty()){
			if(!std::holds_alternative<basicToken>(tList.front().value))
				return std::nullopt;
			if(std::get<basicToken>(tList.front().value).val != ",")
				return std::nullopt;
			tList = tList.subspan(1);
		}
	}

	return std::make_pair(ast::function::call{name, args}, 3);
}

std::optional<std::pair<std::vector<ast::function::statement>, int>> parseFunctionBody(std::span<const mediumToken> tokens){
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
			output.push_back(declTry->first);
			toks = toks.subspan(declTry->second);
			continue;
		}
		auto callTry = parseFunctionBodyCall(toks);
		if(callTry){
			output.push_back(callTry->first);
			toks = toks.subspan(callTry->second);
			continue;
		}
		std::cerr<<"Found structure in function body that did not parse as either variable declaration or function call"<<std::endl;
		break;
	}
	return std::make_pair(output, 1);
}

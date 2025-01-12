#include "parseExpr.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseLiteral.hpp"
#include "parseTemplate.hpp"

parseRes<ast::call> parseCall(std::span<const mediumToken> tokens){
	if(tokens.size() < 2)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const std::string& name = std::get<basicToken>(tokens.front().value).val;
	tokens = tokens.subspan(1);

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& list = std::get<mediumToken::tokList>(tokens.front().value);
	if(list.type != mediumToken::tokList::PAREN)
		return std::nullopt;
	parse_debug_print("found likely call expr");
	std::span<const mediumToken> tList(list.value);
	std::vector<ast::expr> callArgs;
	while(!tList.empty()){
		auto argTry = parseExpr(tList);
		if(!argTry)
			return std::nullopt;
		callArgs.push_back(argTry->val);
		tList = tList.subspan(argTry->toksConsumed);
		if(!tList.empty()){
			if(!std::holds_alternative<basicToken>(tList.front().value))
				return std::nullopt;
			if(std::get<basicToken>(tList.front().value).val != ",")
				return std::nullopt;
			tList = tList.subspan(1);
		}
	}

	parse_debug_print("Parsed expr as call");

	return makeParseRes(ast::call{name, callArgs}, 2);
}

parseRes<ast::templateCall> parseTemplateCall(std::span<const mediumToken> tokens){
	if(tokens.size() < 2)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const std::string& name = std::get<basicToken>(tokens.front().value).val;
	tokens = tokens.subspan(1);

	auto templateArgs = parseTemplateArgs(tokens);
	if(!templateArgs)
		return std::nullopt;
	tokens = tokens.subspan(templateArgs->toksConsumed);

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& list = std::get<mediumToken::tokList>(tokens.front().value);
	if(list.type != mediumToken::tokList::PAREN)
		return std::nullopt;
	std::span<const mediumToken> tList(list.value);
	std::vector<ast::expr> callArgs;
	while(!tList.empty()){
		auto argTry = parseExpr(tList);
		if(!argTry)
			return std::nullopt;
		callArgs.push_back(argTry->val);
		tList = tList.subspan(argTry->toksConsumed);
		if(!tList.empty()){
			if(!std::holds_alternative<basicToken>(tList.front().value))
				return std::nullopt;
			if(std::get<basicToken>(tList.front().value).val != ",")
				return std::nullopt;
			tList = tList.subspan(1);
		}
	}

	parse_debug_print("Parsed expr as template call");

	return makeParseRes(ast::templateCall{name, callArgs, templateArgs->val}, 2+templateArgs->toksConsumed);
}

parseRes<ast::expr> parseExpr(std::span<const mediumToken> tokens){
	//try literal
	const auto& literalTry = parseLiteral(tokens);
	if(literalTry){
		return makeParseRes(ast::expr(literalTry->val), literalTry->toksConsumed);
	}

	//try function template call
	const auto& callTemplateTry = parseTemplateCall(tokens);
	if(callTemplateTry){
		return makeParseRes(ast::expr(callTemplateTry->val), callTemplateTry->toksConsumed);
	}

	//try function call
	const auto& callTry = parseCall(tokens);
	if(callTry){
		return makeParseRes(ast::expr(callTry->val), callTry->toksConsumed);
	}

	//try var name
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const std::string& name = std::get<basicToken>(tokens.front().value).val;
	return makeParseRes(ast::expr(name), 1);
	return std::nullopt;
}


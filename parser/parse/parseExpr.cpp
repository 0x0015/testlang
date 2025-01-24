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

template<class Arr> constexpr bool isTwoCharsInList(char c1, char c2, const Arr& arr){
	for(const auto& c : arr){
		//assuming that the size of c == 2
		if(c[0] == c1 && c[1] == c2)
			return true;
	}
	return false;
}

parseRes<ast::expr> parseExpr(std::span<const mediumToken> tokens){
	std::optional<ast::expr> output;
	unsigned int outputToks = 0;
	//try literal
	{
		const auto& literalTry = parseLiteral(tokens);
		if(literalTry){
			output = literalTry->val;
			outputToks += literalTry->toksConsumed;
		}
	}

	//try function template call
	if(!output){
		const auto& callTemplateTry = parseTemplateCall(tokens);
		if(callTemplateTry){
			output = callTemplateTry->val;
			outputToks += callTemplateTry->toksConsumed;
		}
	}

	//try function call
	if(!output){
		const auto& callTry = parseCall(tokens);
		if(callTry){
			output = callTry->val;
			outputToks += callTry->toksConsumed;
		}
	}

	//try var name
	if(!output){
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		const std::string& name = std::get<basicToken>(tokens.front().value).val;
		output = ast::expr(name);
		outputToks++;
	}

	if(!output)
		return std::nullopt;

	
	//try arithmatic operators
	
	//note: although this works in simple cases, it has much farther to go.
	//the big thing that's missing is operator pereference.  For example || or && should have lower pereference than == or > so that something along the lines of "i < 10 || i == 16" gets parsed to "(i < 10) || (i == 16)" rather than "i < (10 || (i == 16))" (as it would now)
	tokens = tokens.subspan(outputToks);
	if(tokens.size() >= 2 && std::holds_alternative<basicToken>(tokens.front().value)){
		const auto& nextTok = std::get<basicToken>(tokens.front().value);
		if(nextTok.val.size() == 1){
			constexpr char singleCharArithmaticOps[] = {'+', '-', '*', '/', '%', '<', '>'};
			constexpr std::string_view twoCharArithmaticOps[] = {"==", "!=", "<=", ">=", "&&", "||"};
			if(tokens.size() > 2 && std::holds_alternative<basicToken>(tokens[1].value) && std::get<basicToken>(tokens[1].value).val.size() == 1 && isTwoCharsInList(nextTok.val[0], std::get<basicToken>(tokens[1].value).val[0], twoCharArithmaticOps)){
				//const auto& secondTok = std::get<basicToken>(tokens[1].value); //we don't actually need to know this right now as each two char arithmatic operator has a unique starting char
				tokens = tokens.subspan(2);
				const auto nextExpr = parseExpr(tokens);
				if(nextExpr){
					outputToks+=2;//for the two chars of the arithmatic op
					outputToks+= nextExpr->toksConsumed;
					switch(nextTok.val[0]){
						case '=':// == is only option
							return makeParseRes(ast::expr(ast::call{"equal", {*output, nextExpr->val}}), outputToks);
						case '!':// != is only option
							return makeParseRes(ast::expr(ast::call{"not", {ast::call{"equal", {*output, nextExpr->val}}}}), outputToks);
						case '<':// <= is only option
							return makeParseRes(ast::expr(ast::call{"lessOrEqual", {*output, nextExpr->val}}), outputToks);
						case '>':// >= is only option
							return makeParseRes(ast::expr(ast::call{"greaterOrEqual", {*output, nextExpr->val}}), outputToks);
						case '&':// && is only option
							return makeParseRes(ast::expr(ast::call{"and", {*output, nextExpr->val}}), outputToks);
						case '|':// || is only option
							return makeParseRes(ast::expr(ast::call{"or", {*output, nextExpr->val}}), outputToks);
						default:
							break;
					}
				}
			}else if(isInList(nextTok.val[0], singleCharArithmaticOps)){
				tokens = tokens.subspan(1);
				const auto nextExpr = parseExpr(tokens);
				if(nextExpr){
					outputToks++;//for the single char arithmatic op
					outputToks+= nextExpr->toksConsumed;
					switch(nextTok.val[0]){
						case '+':
							return makeParseRes(ast::expr(ast::call{"add", {*output, nextExpr->val}}), outputToks);
						case '-':
							return makeParseRes(ast::expr(ast::call{"sub", {*output, nextExpr->val}}), outputToks);
						case '*':
							return makeParseRes(ast::expr(ast::call{"mul", {*output, nextExpr->val}}), outputToks);
						case '/':
							return makeParseRes(ast::expr(ast::call{"div", {*output, nextExpr->val}}), outputToks);
						case '%':
							return makeParseRes(ast::expr(ast::call{"mod", {*output, nextExpr->val}}), outputToks);
						case '<':
							return makeParseRes(ast::expr(ast::call{"less", {*output, nextExpr->val}}), outputToks);
						case '>':
							return makeParseRes(ast::expr(ast::call{"greater", {*output, nextExpr->val}}), outputToks);
						default:
							break;
					}
				}
			}
		}
	}
	

	return makeParseRes(*output, outputToks);
}


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

parseRes<ast::expr> parseExpr_noOperators(std::span<const mediumToken> tokens){
	//try literal
	{
		const auto& literalTry = parseLiteral(tokens);
		if(literalTry){
			return makeParseRes<ast::expr>(literalTry->val, literalTry->toksConsumed);
		}
	}

	//try function template call
	{
		const auto& callTemplateTry = parseTemplateCall(tokens);
		if(callTemplateTry){
			return makeParseRes<ast::expr>(callTemplateTry->val, callTemplateTry->toksConsumed);
		}
	}

	//try function call
	{
		const auto& callTry = parseCall(tokens);
		if(callTry){
			return makeParseRes<ast::expr>(callTry->val, callTry->toksConsumed);
		}
	}

	//try var name
	{
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		const std::string& name = std::get<basicToken>(tokens.front().value).val;
		return makeParseRes(ast::expr(name), 1);
	}

	return std::nullopt;
}

//these are listed in order of precedence (so * first, || last)
constexpr std::string_view operators[] = {"*", "/", "%", "+", "-", "<", "<=", ">", ">=", "==", "!=", "&&", "||"};
constexpr std::optional<std::string> operator_replacements[] = {"mul", "div", "mod", "add", "sub", "less", "lessOrEqual", "greater", "greaterOrEqual", "equal", std::nullopt /*special case*/, "and", "or"};

constexpr char singleCharArithmaticOps[] = {'+', '-', '*', '/', '%', '<', '>'};
constexpr std::string_view twoCharArithmaticOps[] = {"==", "!=", "<=", ">=", "&&", "||"};

constexpr unsigned int getTwoCharPrescedenceNum(char c1, char c2){
	//all have unique first chars, so we can know just from that (assuming that the two chars make a operator)
	switch(c1){
		case '<': return 6;
		case '>': return 8;
		case '=': return 9;
		case '!': return 10;
		case '&': return 11;
		case '|': return 12;
		default: return -1;
	}
}

constexpr unsigned int getSingleCharPrescedenceNum(char c){
	switch(c){
		case '*': return 0;
		case '/': return 1;
		case '%': return 2;
		case '+': return 3;
		case '-': return 4;
		case '<': return 5;
		case '>': return 7;
		default: return -1;
	}
}

template<class Arr> constexpr bool isTwoCharsInList(char c1, char c2, const Arr& arr){
	for(const auto& c : arr){
		//assuming that the size of c == 2
		if(c[0] == c1 && c[1] == c2)
			return true;
	}
	return false;
}

struct exprOperatorList{
	std::vector<ast::expr> exprs;
	std::vector<unsigned int> betweenExprOperators;
	//exprs.size() should always be betweenExprOperators.size() + 1
	//the idea is the first item of betweenExprOperators is the expr between the first two exprs, etc...
	exprOperatorList(const ast::expr& exp) : exprs{exp}{};
	void addExpr(unsigned int opNum, const ast::expr& exp){
		exprs.push_back(exp);
		betweenExprOperators.push_back(opNum);
	};
	//given a + b * c + d replace with a + (b * c) + d
	void replaceExprsAndOp(unsigned int firstExprIndex, const ast::expr& newExpr){
		exprs.erase(exprs.begin() + firstExprIndex + 1);
		betweenExprOperators.erase(betweenExprOperators.begin() + firstExprIndex);
		exprs[firstExprIndex] = newExpr;
	}
	void replacePairsWithCall(unsigned int operaterPrescNum, const std::string& callName){
		for(unsigned int i=0;i<betweenExprOperators.size();i++){
			if(betweenExprOperators[i] == operaterPrescNum){
				const auto& newExpr = ast::call{callName, {exprs[i], exprs[i+1]}};
				replaceExprsAndOp(i, newExpr);
				i--;
			}
		}
	}
};

//returns prescedence num of upcoming operator as well as number of toks in the operator (optionally)
std::optional<std::pair<unsigned int, unsigned char>> isOperatorUpcoming(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& nextTok = std::get<basicToken>(tokens.front().value);
	if(nextTok.val.size() != 1)
		return std::nullopt;

	if(tokens.size() >= 2 && std::holds_alternative<basicToken>(tokens[1].value)){
		const auto& furtherTok = std::get<basicToken>(tokens[1].value);
		if(furtherTok.val.size() == 1){
			if(isTwoCharsInList(nextTok.val[0], furtherTok.val[0], twoCharArithmaticOps))
				return std::make_pair(getTwoCharPrescedenceNum(nextTok.val[0], furtherTok.val[0]), 2);
		}
	}
	//fallthrough to single tok operator logic if it's not two tokens
	
	if(isInList(nextTok.val[0], singleCharArithmaticOps))
		return std::make_pair(getSingleCharPrescedenceNum(nextTok.val[0]), 1);

	return std::nullopt;
}

parseRes<ast::expr> parseExpr(std::span<const mediumToken> tokens){
	auto firstExpr = parseExpr_noOperators(tokens);
	if(!firstExpr)
		return std::nullopt;

	unsigned int toksConsumed = firstExpr->toksConsumed;
	tokens = tokens.subspan(firstExpr->toksConsumed);

	if(!isOperatorUpcoming(tokens))
		return firstExpr;

	parse_debug_print("Parsed expr with upcoming op");
	
	exprOperatorList opList(firstExpr->val);

	while(true){
		const auto& upcomingOps = isOperatorUpcoming(tokens);
		if(!upcomingOps)
			break;

		toksConsumed += upcomingOps->second;
		tokens = tokens.subspan(upcomingOps->second);

		const auto& nextExpr = parseExpr_noOperators(tokens);
		if(!nextExpr)
			break;

		toksConsumed += nextExpr->toksConsumed;
		tokens = tokens.subspan(nextExpr->toksConsumed);

		opList.addExpr(upcomingOps->first, nextExpr->val);
	}

	for(unsigned int i=0;i<sizeof(operators);i++){
		if(i == getTwoCharPrescedenceNum('!', '=')){
			for(unsigned int i=0;i<opList.betweenExprOperators.size();i++){
				if(opList.betweenExprOperators[i] == getTwoCharPrescedenceNum('!', '=')){
					const auto& eqCall = ast::call{"equal", {opList.exprs[i], opList.exprs[i+1]}};
					const auto& notCall = ast::call{"not", {eqCall}};
					opList.replaceExprsAndOp(i, notCall);
					i--;
				}
			}
		}else{
			opList.replacePairsWithCall(i, *(operator_replacements[i]));
		}
	}

	if(opList.exprs.size() != 1 || !opList.betweenExprOperators.empty()){
		std::cerr<<"Internal Error: Reduced operators according to prescedence, but there were still expers and/or operators left!"<<std::endl;
		return std::nullopt;
	}

	parse_debug_print("Parsed expr with ops");

	return makeParseRes(opList.exprs.front(), toksConsumed);
}

parseRes<std::vector<ast::expr>> parseCommaSeperatedExprList(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return makeParseRes(std::vector<ast::expr>(), 0);

	std::vector<ast::expr> output;

	auto firstExpr = parseExpr(tokens);
	if(!firstExpr)
		return std::nullopt;
	output.push_back(firstExpr->val);
	tokens = tokens.subspan(firstExpr->toksConsumed);

	while(!tokens.empty()){
		if(tokens.size() < 2)
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		if(std::get<basicToken>(tokens.front().value).val != ",")
			return std::nullopt;
		tokens = tokens.subspan(1);

		auto nextExpr = parseExpr(tokens);
		if(!nextExpr)
			return std::nullopt;
		output.push_back(nextExpr->val);
		tokens = tokens.subspan(nextExpr->toksConsumed);
	}

	return makeParseRes(output, tokens.size());
}


#include "parseFunction.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseType.hpp"
#include "../parseUtil.hpp"
#include "parseLiteral.hpp"
#include "../errorUtil.hpp"
#include "parseExpr.hpp"

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

parseRes<ast::function::assignment> parseFunctionBodyAssignment(std::span<const mediumToken> tokens){
	if(tokens.size() < 4)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& asgnTo = std::get<basicToken>(tokens.front().value).val;
	int outputSize = 1;
	tokens = tokens.subspan(1);
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "=")
		return std::nullopt;
	outputSize++;
	tokens = tokens.subspan(1);
	auto literalTry = parseLiteral(tokens);
	ast::function::assignment output;
	output.assignTo = asgnTo;
	if(literalTry){
		output.assignFrom = literalTry->val;
		outputSize += literalTry->toksConsumed;
		tokens = tokens.subspan(literalTry->toksConsumed);
	}else{
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		const auto& asgnFrom = std::get<basicToken>(tokens.front().value).val;
		output.assignFrom = asgnFrom;
		tokens = tokens.subspan(1);
		outputSize++;
	}

	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;
	outputSize++;

	return makeParseRes(output, outputSize);
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
	bool gotParseError = false;
	while(!toks.empty()){
		auto declTry = parseFunctionBodyDeclaration(toks);
		if(declTry){
			output.push_back(declTry->val);
			toks = toks.subspan(declTry->toksConsumed);
			continue;
		}
		auto asgnTry = parseFunctionBodyAssignment(toks);
		if(asgnTry){
			output.push_back(asgnTry->val);
			toks = toks.subspan(asgnTry->toksConsumed);
			continue;
		}
		auto exprTry = parseExpr(toks);
		if(exprTry){
			output.push_back(exprTry->val);
			toks = toks.subspan(exprTry->toksConsumed);
			continue;
		}
		std::cerr<<"Found structure in function body that did not parse as either variable declaration, variable assignment, or function call"<<std::endl;
		printErrorFileSpot(toks.front());
		gotParseError = true;
		break;
	}
	return makeParseRes(output, 1, gotParseError);
}

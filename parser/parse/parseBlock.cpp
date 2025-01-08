#include "parseBlock.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseType.hpp"
#include "../parseUtil.hpp"
#include "parseLiteral.hpp"
#include "../errorUtil.hpp"
#include "parseExpr.hpp"

parseRes<ast::block::declaration> parseDeclaration(std::span<const mediumToken> tokens){
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
	return makeParseRes(ast::block::declaration{tyTry->val, name}, outputSize);
}

parseRes<ast::block::assignment> parseAssignment(std::span<const mediumToken> tokens){
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

	const auto& asgnFrom = parseExpr(tokens);
	if(!asgnFrom)
		return std::nullopt;
	tokens = tokens.subspan(asgnFrom->toksConsumed);
	outputSize += asgnFrom->toksConsumed;

	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;
	outputSize++;

	return makeParseRes(ast::block::assignment{asgnTo, asgnFrom->val}, outputSize);
}

parseRes<ast::expr> parseExprStatement(std::span<const mediumToken> tokens){
	auto exprTry = parseExpr(tokens);
	if(!exprTry)
		return std::nullopt;
	tokens = tokens.subspan(exprTry->toksConsumed);

	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;

	return makeParseRes(exprTry->val, exprTry->toksConsumed+1);
}

parseRes<ast::block::ifStatement> parseIfStatement(std::span<const mediumToken> tokens){
	if(tokens.size() < 3)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "if")
		return std::nullopt;
	unsigned int outputSize = 1;
	tokens = tokens.subspan(1);

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	if(std::get<mediumToken::tokList>(tokens.front().value).type != mediumToken::tokList::PAREN)
		return std::nullopt;
	auto condExpr = parseExpr(std::get<mediumToken::tokList>(tokens.front().value).value);
	if(!condExpr)
		return std::nullopt;
	outputSize++;
	tokens = tokens.subspan(1);

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	if(std::get<mediumToken::tokList>(tokens.front().value).type != mediumToken::tokList::CURL_BRACK)
		return std::nullopt;
	auto ifBlock = parseBlock(std::get<mediumToken::tokList>(tokens.front().value).value, false);
	if(!ifBlock)
		return std::nullopt;
	outputSize++;
	tokens = tokens.subspan(1);

	if(tokens.size() < 2 || !(std::holds_alternative<basicToken>(tokens.front().value) && std::get<basicToken>(tokens.front().value).val == "else")){
		return makeParseRes(ast::block::ifStatement{condExpr->val, std::make_shared<ast::block>(ifBlock->val), std::make_shared<ast::block>()}, 3);
	}

	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "else")
		return std::nullopt;
	outputSize++;
	tokens = tokens.subspan(1);

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	if(std::get<mediumToken::tokList>(tokens.front().value).type != mediumToken::tokList::CURL_BRACK)
		return std::nullopt;
	auto elseBlock = parseBlock(std::get<mediumToken::tokList>(tokens.front().value).value, false);
	if(!elseBlock)
		return std::nullopt;
	outputSize++;

	return makeParseRes(ast::block::ifStatement{condExpr->val, std::make_shared<ast::block>(ifBlock->val), std::make_shared<ast::block>(elseBlock->val)}, 5);
}

parseRes<ast::block::returnStatement> parseReturnStatement(std::span<const mediumToken> tokens){
	if(tokens.size() < 3)
		return std::nullopt;

	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "return")
		return std::nullopt;
	tokens = tokens.subspan(1);

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	if(std::get<mediumToken::tokList>(tokens.front().value).type != mediumToken::tokList::PAREN)
		return std::nullopt;
	auto retVal = parseExpr(std::get<mediumToken::tokList>(tokens.front().value).value);
	if(!retVal)
		return std::nullopt;
	tokens = tokens.subspan(1);

	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != ";")
		return std::nullopt;

	//always consumes 3
	return makeParseRes(ast::block::returnStatement{retVal->val}, 3);
}

parseRes<std::vector<ast::block::statement>> parseBlock_internal(std::span<const mediumToken> tokens){
	std::vector<ast::block::statement> output;
	bool gotParseError = false;
	//all tokens will be consumed
	while(!tokens.empty()){
		auto declTry = parseDeclaration(tokens);
		if(declTry){
			output.push_back(declTry->val);
			tokens = tokens.subspan(declTry->toksConsumed);
			continue;
		}
		auto asgnTry = parseAssignment(tokens);
		if(asgnTry){
			output.push_back(asgnTry->val);
			tokens = tokens.subspan(asgnTry->toksConsumed);
			continue;
		}
		auto retTry = parseReturnStatement(tokens);
		if(retTry){
			output.push_back(retTry->val);
			tokens = tokens.subspan(retTry->toksConsumed);
			continue;
		}
		auto exprTry = parseExprStatement(tokens);
		if(exprTry){
			output.push_back(exprTry->val);
			tokens = tokens.subspan(exprTry->toksConsumed);
			continue;
		}
		auto ifTry = parseIfStatement(tokens);
		if(ifTry){
			output.push_back(ifTry->val);
			tokens = tokens.subspan(ifTry->toksConsumed);
			continue;
		}
		std::cerr<<"Found structure in code block that did not parse as either variable declaration, variable assignment, or function call"<<std::endl;
		printErrorFileSpot(tokens.front());
		gotParseError = true;
		break;
	}
	return makeParseRes(output, tokens.size(), gotParseError);
}

parseRes<ast::block> parseBlock(std::span<const mediumToken> tokens, bool includeCurlBrackets){
	if(includeCurlBrackets){
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
			return std::nullopt;
		const auto& list = std::get<mediumToken::tokList>(tokens.front().value);
		if(list.type != mediumToken::tokList::type_t::CURL_BRACK)
			return std::nullopt;
		std::span<const mediumToken> toks(list.value);

		auto insideParsed = parseBlock_internal(toks);
		if(!insideParsed)
			return std::nullopt;
		return makeParseRes(ast::block{insideParsed->val}, 1, insideParsed->encounteredError);
	}

	auto insideParsed = parseBlock_internal(tokens);
	return makeParseRes(ast::block(insideParsed->val), insideParsed->toksConsumed, insideParsed->encounteredError);
}

#include "parseBlock.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseType.hpp"
#include "../parseUtil.hpp"
#include "parseLiteral.hpp"
#include "../errorUtil.hpp"
#include "parseExpr.hpp"

namespace minLang{
parseRes<minLang::ast::block::declaration> parseDeclaration(std::span<const mediumToken> tokens){
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

	parse_debug_print("parsed variable declaration");
	//don't need to shrink tokens for the last one
	return makeParseRes(minLang::ast::block::declaration{tyTry->val, name}, outputSize);
}

parseRes<minLang::ast::block::assignment> parseBasicAssignment(std::span<const mediumToken> tokens, bool inculdeSemicolin = true /*disableable for reuse in parsing last statement in for loop*/){
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

	parse_debug_print("parsed assignment up to =");

	const auto& asgnFrom = parseExpr(tokens);
	if(!asgnFrom)
		return std::nullopt;
	tokens = tokens.subspan(asgnFrom->toksConsumed);
	outputSize += asgnFrom->toksConsumed;

	if(inculdeSemicolin){
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		if(std::get<basicToken>(tokens.front().value).val != ";")
			return std::nullopt;
		outputSize++;
	}

	return makeParseRes(minLang::ast::block::assignment{asgnTo, asgnFrom->val}, outputSize);
}

parseRes<minLang::ast::block::assignment> parseTransformationAssignment(std::span<const mediumToken> tokens, bool inculdeSemicolin = true /*disableable for reuse in parsing last statement in for loop*/){
	if(tokens.size() < 4)
		return std::nullopt;

	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& asgnTo = std::get<basicToken>(tokens.front().value).val;
	int outputSize = 1;
	tokens = tokens.subspan(1);
	
	std::string transformFuncName;
	parseRes<std::vector<minLang::ast::expr>> extraArgs;
	if(std::holds_alternative<basicToken>(tokens.front().value)){
		transformFuncName = std::get<basicToken>(tokens.front().value).val;
		if(transformFuncName.size() == 1){
			switch(transformFuncName.front()){
				case '+':
					transformFuncName = "add";
					break;
				case '-':
					transformFuncName = "sub";
					break;
				case '/':
					transformFuncName = "div";
					break;
				case '*':
					transformFuncName = "mul";
					break;
				default:
					break;
			}
		}
		outputSize++;
		tokens = tokens.subspan(1);
	
		if(std::holds_alternative<mediumToken::tokList>(tokens.front().value)){
			//parse other args for the transformation func (eg index for get)
			const auto& list = std::get<mediumToken::tokList>(tokens.front().value);
			if(list.type != mediumToken::tokList::PAREN)
				return std::nullopt;
			
			extraArgs = parseCommaSeperatedExprList(list.value);
			if(!extraArgs)
				return std::nullopt;
	
			outputSize++;
			tokens = tokens.subspan(1);
		}
	}else if(std::holds_alternative<mediumToken::tokList>(tokens.front().value)){
		const auto& tokList = std::get<mediumToken::tokList>(tokens.front().value);
		if(tokList.type != mediumToken::tokList::SQUARE_BRACK)
			return std::nullopt;
		transformFuncName = "set";
		extraArgs = parseCommaSeperatedExprList(tokList.value);
		if(!extraArgs)
			return std::nullopt;

		outputSize++;
		tokens = tokens.subspan(1);
	}else
		return std::nullopt;

	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "=")
		return std::nullopt;
	outputSize++;
	tokens = tokens.subspan(1);

	parse_debug_print("parsed transformation assignment up to =");

	const auto& asgnFrom = parseExpr(tokens);
	if(!asgnFrom)
		return std::nullopt;
	tokens = tokens.subspan(asgnFrom->toksConsumed);
	outputSize += asgnFrom->toksConsumed;

	if(inculdeSemicolin){
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		if(std::get<basicToken>(tokens.front().value).val != ";")
			return std::nullopt;
		outputSize++;
	}

	minLang::ast::call transformationCall{transformFuncName, {asgnTo, asgnFrom->val}};
	if(extraArgs){
		for(const auto& extraArg : extraArgs->val)
			transformationCall.args.push_back(extraArg);
	}
	minLang::ast::block::assignment output{asgnTo, transformationCall};
	return makeParseRes(output, outputSize);
}

parseRes<minLang::ast::block::assignment> parseAssignment(std::span<const mediumToken> tokens, bool inculdeSemicolin = true /*disableable for reuse in parsing last statement in for loop*/){
	 const auto& basicAssignment = parseBasicAssignment(tokens, inculdeSemicolin);
	 if(basicAssignment)
		 return basicAssignment;

	 const auto& transformationAssignment = parseTransformationAssignment(tokens, inculdeSemicolin);
	 if(transformationAssignment)
		 return transformationAssignment;

	 return std::nullopt;
}

parseRes<minLang::ast::expr> parseExprStatement(std::span<const mediumToken> tokens){
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

	parse_debug_print("parsed direct expr in block");

	return makeParseRes(exprTry->val, exprTry->toksConsumed+1);
}

parseRes<minLang::ast::block::ifStatement> parseIfStatement(std::span<const mediumToken> tokens){
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
	if((int)std::get<mediumToken::tokList>(tokens.front().value).value.size() != condExpr->toksConsumed)
		return std::nullopt;//if the condition left tokens unused, that is not okay
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
		parse_debug_print("parsed if statement in block");
		return makeParseRes(minLang::ast::block::ifStatement{condExpr->val, std::make_shared<minLang::ast::block>(ifBlock->val), std::make_shared<minLang::ast::block>()}, 3);
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
	parse_debug_print("parsed if else statement in block");

	return makeParseRes(minLang::ast::block::ifStatement{condExpr->val, std::make_shared<minLang::ast::block>(ifBlock->val), std::make_shared<minLang::ast::block>(elseBlock->val)}, 5);
}

parseRes<minLang::ast::block::forStatement_normal> parseForStatement(std::span<const mediumToken> tokens){
	if(tokens.size() < 3)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "for")
		return std::nullopt;
	unsigned int outputSize = 1;
	tokens = tokens.subspan(1);

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	if(std::get<mediumToken::tokList>(tokens.front().value).type != mediumToken::tokList::PAREN)
		return std::nullopt;
	auto condToks = std::span<const mediumToken>(std::get<mediumToken::tokList>(tokens.front().value).value);
	outputSize++;
	tokens = tokens.subspan(1);

	auto initialDecl = parseAssignment(condToks);
	if(!initialDecl)
		return std::nullopt;
	condToks = condToks.subspan(initialDecl->toksConsumed);
	auto breakCond = parseExpr(condToks);
	if(!breakCond)
		return std::nullopt;
	condToks = condToks.subspan(breakCond->toksConsumed);
	if(condToks.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(condToks.front().value))
		return std::nullopt;
	if(std::get<basicToken>(condToks.front().value).val != ";")
		return std::nullopt;
	condToks = condToks.subspan(1);
	auto perloopAsgn = parseAssignment(condToks, false);
	if(!perloopAsgn)
		return std::nullopt;
	condToks = condToks.subspan(perloopAsgn->toksConsumed);
	if(!condToks.empty())
		return std::nullopt;

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	if(std::get<mediumToken::tokList>(tokens.front().value).type != mediumToken::tokList::CURL_BRACK)
		return std::nullopt;
	auto whileBlock = parseBlock(std::get<mediumToken::tokList>(tokens.front().value).value, false);
	if(!whileBlock)
		return std::nullopt;
	outputSize++;

	parse_debug_print("parsed for loop in block");
	return makeParseRes(minLang::ast::block::forStatement_normal{initialDecl->val, breakCond->val, perloopAsgn->val, std::make_shared<minLang::ast::block>(whileBlock->val)}, 3);
}

parseRes<minLang::ast::block::forStatement_while> parseWhileStatement(std::span<const mediumToken> tokens){
	if(tokens.size() < 3)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "for")
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
	if((int)std::get<mediumToken::tokList>(tokens.front().value).value.size() != condExpr->toksConsumed)
		return std::nullopt;//if the condition left tokens unused, that is not okay (think while(true and false) "and" wouldn't be recognized so it would be ignored and this would be parsed as while(true) which is disceptive)
	outputSize++;
	tokens = tokens.subspan(1);

	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	if(std::get<mediumToken::tokList>(tokens.front().value).type != mediumToken::tokList::CURL_BRACK)
		return std::nullopt;
	auto whileBlock = parseBlock(std::get<mediumToken::tokList>(tokens.front().value).value, false);
	if(!whileBlock)
		return std::nullopt;
	outputSize++;

	parse_debug_print("parsed for (while) loop in block");
	return makeParseRes(minLang::ast::block::forStatement_while{condExpr->val, std::make_shared<minLang::ast::block>(whileBlock->val)}, 3);
}

parseRes<minLang::ast::block::returnStatement> parseReturnStatement(std::span<const mediumToken> tokens){
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
	return makeParseRes(minLang::ast::block::returnStatement{retVal->val}, 3);
}

parseRes<std::vector<minLang::ast::block::statement>> parseBlock_internal(std::span<const mediumToken> tokens){
	std::vector<minLang::ast::block::statement> output;
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
		auto forTry = parseForStatement(tokens);
		if(forTry){
			output.push_back(forTry->val);
			tokens = tokens.subspan(forTry->toksConsumed);
			continue;
		}
		auto whileTry = parseWhileStatement(tokens);
		if(whileTry){
			output.push_back(whileTry->val);
			tokens = tokens.subspan(whileTry->toksConsumed);
			continue;
		}
		std::cerr<<"Found structure in code block that did not parse as either variable declaration, variable assignment, or function call"<<std::endl;
		printErrorFileSpot(tokens.front());
		gotParseError = true;
		break;
	}
	return makeParseRes(output, tokens.size(), gotParseError);
}
}

parseRes<minLang::ast::block> minLang::parseBlock(std::span<const mediumToken> tokens, bool includeCurlBrackets){
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
		return makeParseRes(minLang::ast::block{insideParsed->val}, 1, insideParsed->encounteredError);
	}

	auto insideParsed = parseBlock_internal(tokens);
	return makeParseRes(minLang::ast::block(insideParsed->val), insideParsed->toksConsumed, insideParsed->encounteredError);
}

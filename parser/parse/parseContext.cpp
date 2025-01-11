#include "parseContext.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseFunction.hpp"
#include "parseAlias.hpp"
#include "../parseUtil.hpp"
#include "parseType.hpp"

//when parsing a context, we want to parse all type information first, then functions (which would be using them).
//This is desirable as then the functions know all the type information in the whole program

std::optional<ast::context> parseContext(std::span<const mediumToken> tokens){
	ast::context context;
	bool trying = true;

	std::vector<std::span<const mediumToken>> functionsFound;
	std::vector<std::span<const mediumToken>> externalFunctionsFound;
	std::vector<std::span<const mediumToken>> aliasesFound;
	while(trying){
		trying = false;
		if(tokens.empty())
			break;
		const auto& aliasTry = looksLikeAlias(tokens);
		if(aliasTry){
			parse_debug_print("context found alias like structure");
			aliasesFound.push_back(tokens.subspan(0, aliasTry->toksConsumed));
			tokens = tokens.subspan(aliasTry->toksConsumed);
			trying = true;
			continue;
		}
		const auto& funcTry = looksLikeFunction(tokens);
		if(funcTry){
			parse_debug_print("context found function like structure");
			functionsFound.push_back(tokens.subspan(0, funcTry->toksConsumed));
			tokens = tokens.subspan(funcTry->toksConsumed);
			trying = true;
			continue;
		}
		const auto& externFuncTry = looksLikeExternalFunction(tokens);
		if(externFuncTry){
			parse_debug_print("context parsed external function");
			externalFunctionsFound.push_back(tokens.subspan(0, externFuncTry->toksConsumed));
			tokens = tokens.subspan(externFuncTry->toksConsumed);
			trying = true;
			continue;
		}
		std::cout<<"Error: Failed to parse structure: ";
		tokens.front().print();
		std::cerr<<std::endl;
		return std::nullopt;
	}

	for(const auto& aliasToks : aliasesFound){
		const auto& aliasTry = parseAlias(aliasToks);
		if(aliasTry){
			parserKnownAliases[aliasTry->val.name] = aliasTry->val;
		}else{
			std::cout<<"Error: Failed to parse alias"<<std::endl;
			return std::nullopt;
		}
	}
	for(const auto& funcToks : functionsFound){
		const auto& funcTry = parseFunction(funcToks);
		if(funcTry){
			context.funcs.push_back(funcTry->val);
		}else{
			std::cout<<"Error: Failed to parse function"<<std::endl;
			return std::nullopt;
		}
	}
	for(const auto& externFuncToks : externalFunctionsFound){
		const auto& funcTry = parseExternalFunction(externFuncToks);
		if(funcTry){
			context.funcs.push_back(funcTry->val);
		}else{
			std::cout<<"Error: Failed to parse external function"<<std::endl;
			return std::nullopt;
		}
	}

	return context;
}


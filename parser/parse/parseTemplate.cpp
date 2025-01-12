#include "parseTemplate.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseType.hpp"
#include "../parseUtil.hpp"
#include "../errorUtil.hpp"
#include "parseExpr.hpp"
#include "parseAlias.hpp"
#include "parseFunction.hpp"

void switchTemplatesForReplacements(ast::type& ty, const std::vector<ast::type>& replacementTypes){
	if(std::holds_alternative<ast::type::template_type>(ty.ty)){
		ty = replacementTypes[std::get<ast::type::template_type>(ty.ty).templateParamNum];
	}else if(std::holds_alternative<ast::type::tuple_type>(ty.ty)){
		for(auto& tuple_ty : std::get<ast::type::tuple_type>(ty.ty))
			switchTemplatesForReplacements(tuple_ty, replacementTypes);
	}else if(std::holds_alternative<ast::type::array_type>(ty.ty)){
		switchTemplatesForReplacements(*std::get<ast::type::array_type>(ty.ty).ty, replacementTypes);
	}
	//for all other cases, no work is needed
}

ast::type::alias_type aliasTemplate::instantiate(const std::vector<ast::type>& replacementTypes) const{
	auto output = alias;
	output.underlyingType = std::make_shared<ast::type>(alias.underlyingType->clone());//do not screw up the underlying type in the alias template definition
	
	switchTemplatesForReplacements(*output.underlyingType, replacementTypes);
	output.underlyingType = std::make_shared<ast::type>(ast::type::fullyDealias(*output.underlyingType));

	output.name = alias.name + "<";
	for(unsigned int i=0;i<replacementTypes.size();i++){
		output.name += replacementTypes[i].toString();
		if(i+1 < replacementTypes.size())
			output.name += ", ";
	}
	output.name += ">";

	return output;
}

parseRes<std::vector<std::string>> parseCommaSeperatedBTokenList(std::span<const mediumToken> tokens){
	//check for the commas
	for(unsigned int i=1;i<tokens.size();i+=2){
		if(!std::holds_alternative<basicToken>(tokens[i].value))
			return std::nullopt;
		const auto& btoken = std::get<basicToken>(tokens[i].value);
		if(btoken.val != ",")
			return std::nullopt;
	}
	//get the actual values
	std::vector<std::string> output;
	for(unsigned int i=0;i<tokens.size();i+=2){
		if(!std::holds_alternative<basicToken>(tokens[i].value))
			return std::nullopt;
		const auto& btoken = std::get<basicToken>(tokens[i].value);
		if(btoken.val == ",")
			return std::nullopt;
		output.push_back(btoken.val);
	}

	return makeParseRes(output, tokens.size());
}

parseRes<std::vector<std::string>> parseTemplateKeywordArgs(std::span<const mediumToken> tokens){
	//template<
	if(tokens.size() < 2)
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "template")
		return std::nullopt;
	tokens = tokens.subspan(1);
	parse_debug_print("template parsed template");
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "<")
		return std::nullopt;
	tokens = tokens.subspan(1);
	int offset = 2;

	//search for template closing >
	int closingChevronPos = -1;
	unsigned int braceOpeningsUnclosed = 1;
	for(unsigned int i=0;i<tokens.size();i++){
		if(!std::holds_alternative<basicToken>(tokens[i].value))
			return std::nullopt;
		const auto& btoken = std::get<basicToken>(tokens[i].value);
		if(btoken.val == "<"){
			braceOpeningsUnclosed++;
		}else if(btoken.val == ">"){
			braceOpeningsUnclosed--;
			if(braceOpeningsUnclosed == 0){
				closingChevronPos = i;
				break;
			}
		}
	}

	if(closingChevronPos == -1)
		return std::nullopt;
	parse_debug_print("template parsed angle brackets");

	auto args = parseCommaSeperatedBTokenList(tokens.subspan(0, closingChevronPos));
	if(!args)
		return std::nullopt;

	return makeParseRes(args->val, offset+closingChevronPos+1);
}

parseRes<aliasTemplate> parseAliasTemplate(std::span<const mediumToken> tokens){
	auto args = parseTemplateKeywordArgs(tokens);
	if(!args)
		return std::nullopt;
	tokens = tokens.subspan(args->toksConsumed);
	parse_debug_print("template parsed args");

	parserKnownTemplateTypesDefined = std::cref(args->val);

	auto alias = parseAlias(tokens);
	if(!alias){
		parserKnownTemplateTypesDefined = std::nullopt;
		return std::nullopt;
	}

	parserKnownTemplateTypesDefined = std::nullopt;

	return makeParseRes(aliasTemplate{(unsigned int)args->val.size(), alias->val}, args->toksConsumed + alias->toksConsumed);
}

parseRes<ast::functionTemplate> parseFunctionTemplate(std::span<const mediumToken> tokens){
	auto args = parseTemplateKeywordArgs(tokens);
	if(!args)
		return std::nullopt;
	tokens = tokens.subspan(args->toksConsumed);
	parse_debug_print("template parsed args");

	parserKnownTemplateTypesDefined = std::cref(args->val);

	auto func = parseFunction(tokens);
	if(!func){
		parserKnownTemplateTypesDefined = std::nullopt;
		return std::nullopt;
	}

	parserKnownTemplateTypesDefined = std::nullopt;

	return makeParseRes(ast::functionTemplate{(unsigned int)args->val.size(), func->val}, args->toksConsumed + func->toksConsumed);
}

parseRes<std::monostate> looksLikeAliasTemplate(std::span<const mediumToken> tokens){
	auto args = parseTemplateKeywordArgs(tokens);
	if(!args)
		return std::nullopt;
	tokens = tokens.subspan(args->toksConsumed);
	parse_debug_print("template parsed args");

	parserKnownTemplateTypesDefined = std::cref(args->val);

	auto alias = looksLikeAlias(tokens);
	if(!alias){
		parserKnownTemplateTypesDefined = std::nullopt;
		return std::nullopt;
	}

	parserKnownTemplateTypesDefined = std::nullopt;

	return makeParseRes(std::monostate{}, args->toksConsumed + alias->toksConsumed);
}

parseRes<std::monostate> looksLikeFunctionTemplate(std::span<const mediumToken> tokens){
	auto args = parseTemplateKeywordArgs(tokens);
	if(!args)
		return std::nullopt;
	tokens = tokens.subspan(args->toksConsumed);
	parse_debug_print("template parsed args");

	parserKnownTemplateTypesDefined = std::cref(args->val);

	auto func = looksLikeFunction(tokens);
	if(!func){
		parserKnownTemplateTypesDefined = std::nullopt;
		return std::nullopt;
	}

	parserKnownTemplateTypesDefined = std::nullopt;

	return makeParseRes(std::monostate{}, args->toksConsumed + func->toksConsumed);
}


parseRes<std::vector<ast::type>> parseCommaSeperatedTypeList(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return makeParseRes(std::vector<ast::type>(), 0);

	std::vector<ast::type> output;

	auto firstType = parseType(tokens);
	if(!firstType)
		return std::nullopt;
	output.push_back(firstType->val);
	tokens = tokens.subspan(firstType->toksConsumed);

	while(!tokens.empty()){
		if(tokens.size() < 2)
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		if(std::get<basicToken>(tokens.front().value).val != ",")
			return std::nullopt;
		tokens = tokens.subspan(1);

		auto nextType = parseType(tokens);
		if(!nextType)
			return std::nullopt;
		output.push_back(nextType->val);
		tokens = tokens.subspan(nextType->toksConsumed);
	}

	return makeParseRes(output, tokens.size());
}

parseRes<std::vector<ast::type>> parseTemplateArgs(std::span<const mediumToken> tokens){
	//<
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	if(std::get<basicToken>(tokens.front().value).val != "<")
		return std::nullopt;
	tokens = tokens.subspan(1);
	int offset = 1;

	//search for template closing >
	int closingChevronPos = -1;
	unsigned int braceOpeningsUnclosed = 1;
	for(unsigned int i=0;i<tokens.size();i++){
		if(!std::holds_alternative<basicToken>(tokens[i].value))
			continue;
		const auto& btoken = std::get<basicToken>(tokens[i].value);
		if(btoken.val == "<"){
			braceOpeningsUnclosed++;
		}else if(btoken.val == ">"){
			braceOpeningsUnclosed--;
			if(braceOpeningsUnclosed == 0){
				closingChevronPos = i;
				break;
			}
		}
	}

	if(closingChevronPos == -1)
		return std::nullopt;
	parse_debug_print("template parsed angle brackets");

	auto args = parseCommaSeperatedTypeList(tokens.subspan(0, closingChevronPos));
	if(!args)
		return std::nullopt;

	return makeParseRes(args->val, offset+closingChevronPos+1);
}



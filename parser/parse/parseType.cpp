#include "parseType.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseTemplate.hpp"

parseRes<ast::type::builtin_type> parseBuiltinType(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const basicToken& bToken = std::get<basicToken>(tokens.front().value);
	constexpr std::string_view basicTokNames[] = {"void", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64", "float32", "float64", "bool"};
	constexpr ast::type::builtin_type basicTokTypes[] = {ast::type::builtin_type::void_type, ast::type::builtin_type::int8_type, ast::type::builtin_type::int16_type, ast::type::builtin_type::int32_type, ast::type::builtin_type::int64_type, ast::type::builtin_type::uint8_type, ast::type::builtin_type::uint16_type, ast::type::builtin_type::uint32_type, ast::type::builtin_type::uint64_type, ast::type::builtin_type::float32_type, ast::type::builtin_type::float64_type, ast::type::builtin_type::bool_type};
	auto found = findInList(bToken.val, basicTokNames);
	if(!found)
		return std::nullopt;
	return makeParseRes(basicTokTypes[*found], 1);
}

parseRes<ast::type::array_type> parseArrayType(std::span<const mediumToken> tokens, const ast::type& currentTy){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& tList = std::get<mediumToken::tokList>(tokens.front().value);
	if(tList.type != mediumToken::tokList::SQUARE_BRACK)
		return std::nullopt;
	if(tList.value.size() != 1)
		return std::nullopt;
	ast::type::array_type output{currentTy, (unsigned int)std::stoul(std::get<basicToken>(tList.value.front().value).val)};
	return makeParseRes(output, 1);
}

parseRes<ast::type::tuple_type> parseTupleType(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<mediumToken::tokList>(tokens.front().value))
		return std::nullopt;
	const auto& tList = std::get<mediumToken::tokList>(tokens.front().value);
	if(tList.type != mediumToken::tokList::PAREN)
		return std::nullopt;
	std::span<const mediumToken> toks(tList.value);
	ast::type::tuple_type output;
	while(!toks.empty()){
		auto typeTry = parseType(toks);
		if(!typeTry)
			return std::nullopt;
		output.push_back(typeTry->val);
		toks = toks.subspan(typeTry->toksConsumed);
		//if that's all the type in the tuple, just end now
		if(toks.empty())
			break;
		//alright, there must be at least one more
		if(toks.size() < 2)
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(toks.front().value))
			return std::nullopt;
		if(std::get<basicToken>(toks.front().value).val != ",")
			return std::nullopt;
		toks = toks.subspan(1);
	}
	return makeParseRes(output, 1);
}

parseRes<ast::type::alias_type> parseAliasType(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const basicToken& bToken = std::get<basicToken>(tokens.front().value);
	if(!parserKnownAliases.contains(bToken.val))
		return std::nullopt;
	return makeParseRes(parserKnownAliases[bToken.val], 1);
}

parseRes<ast::type::template_type> parseTemplateType(std::span<const mediumToken> tokens){
	if(!parserKnownTemplateTypesDefined)
		return std::nullopt;
	
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const basicToken& bToken = std::get<basicToken>(tokens.front().value);

	for(unsigned int i=0;i<parserKnownTemplateTypesDefined->get().size();i++){
		if(bToken.val == parserKnownTemplateTypesDefined->get()[i]){
			return makeParseRes(ast::type::template_type{i}, 1);
		}
	}

	return std::nullopt;
}

parseRes<ast::type::alias_type> parseAliasTypeTemplateInstantiation(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const basicToken& bToken = std::get<basicToken>(tokens.front().value);
	if(!parserKnownAliasTemplates.contains(bToken.val))
		return std::nullopt;
	tokens = tokens.subspan(1);

	parse_debug_print("template alias instantiation name found");

	auto args = parseTemplateArgs(tokens);
	if(!args)
		return std::nullopt;

	parse_debug_print("template alias instantiation args parsed");

	const auto& aliasTemplate = parserKnownAliasTemplates[bToken.val];
	if(aliasTemplate.numTemplateArgs != args->val.size())
		return std::nullopt;

	return makeParseRes(aliasTemplate.instantiate(args->val), args->toksConsumed+1);
}


parseRes<ast::type> parseType(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	auto builtinTry = parseBuiltinType(tokens);
	ast::type ty;
	unsigned int consumed = 0;
	//ugly logic here, good enough for now
	if(builtinTry){
		tokens = tokens.subspan(builtinTry->toksConsumed);
		consumed += builtinTry->toksConsumed;
		ty = builtinTry->val;
	}else{
		auto aliasTry = parseAliasType(tokens);
		if(aliasTry){
			tokens = tokens.subspan(aliasTry->toksConsumed);
			consumed += aliasTry->toksConsumed;
			ty = aliasTry->val;
		}else{
			auto templateTry = parseTemplateType(tokens);
			if(templateTry){
				tokens = tokens.subspan(templateTry->toksConsumed);
				consumed += templateTry->toksConsumed;
				ty = templateTry->val;
			}else{
				auto aliasTemplateTry = parseAliasTypeTemplateInstantiation(tokens);
				if(aliasTemplateTry){
					tokens = tokens.subspan(aliasTemplateTry->toksConsumed);
					consumed += aliasTemplateTry->toksConsumed;
					ty = aliasTemplateTry->val;
				}else{
					auto tupleTry = parseTupleType(tokens);
					if(tupleTry){
						tokens = tokens.subspan(tupleTry->toksConsumed);
						consumed += tupleTry->toksConsumed;
						ty = tupleTry->val;
					}else
						return std::nullopt;
				}
			}
		}
	}
	while(!tokens.empty()){
		auto arrayTry = parseArrayType(tokens, ty);
		if(!arrayTry)
			return makeParseRes(ty, consumed);
		ty = arrayTry->val;
		tokens = tokens.subspan(arrayTry->toksConsumed);
		consumed += arrayTry->toksConsumed;
	}
	return makeParseRes(ty, consumed);
}


#include "parseLiteral.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include <sstream>

parseRes<bool> parseBool(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& val = std::get<basicToken>(tokens.front().value).val;
	if(val == "true")
		return makeParseRes(true, 1);
	if(val == "false")
		return makeParseRes(false, 1);
	return std::nullopt;
}

parseRes<uint64_t> parseUInt(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& str = std::get<basicToken>(tokens.front().value).val;
	//using sstream for float makes sense (hard to parse accurately), but for int I could just manually check digits (mabe would make it faster)
	std::istringstream iss(str);
	uint64_t val;
	iss >> std::noskipws >> val;
	if(!iss.eof() || iss.fail())
		return std::nullopt;
	return makeParseRes(val, 1);
}

parseRes<int> parseInt(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& str = std::get<basicToken>(tokens.front().value).val;
	std::istringstream iss(str);
	int val;
	iss >> std::noskipws >> val;
	if(!iss.eof() || iss.fail())
		return std::nullopt;
	return makeParseRes(val, 1);
}

parseRes<float> parseFloat(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& str = std::get<basicToken>(tokens.front().value).val;
	std::istringstream iss(str);
	float val;
	iss >> std::noskipws >> val;
	if(!iss.eof() || iss.fail())
		return std::nullopt;
	return makeParseRes(val, 1);
}

parseRes<ast::literal> parseLiteral(std::span<const mediumToken> tokens){
	auto boolTry = parseBool(tokens);
	if(boolTry)
		return makeParseRes(ast::literal(boolTry->val), boolTry->toksConsumed);
	auto floatTry = parseFloat(tokens);
	if(floatTry)
		return makeParseRes(ast::literal(floatTry->val), floatTry->toksConsumed);
	auto intTry = parseInt(tokens);
	if(intTry)
		return makeParseRes(ast::literal(intTry->val), intTry->toksConsumed);
	return std::nullopt;
}

#include "parseLiteral.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include <sstream>
#include <limits>

namespace minLang{
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
	if(!str.ends_with('u'))
		return std::nullopt;
	//using sstream for float makes sense (hard to parse accurately), but for int I could just manually check digits (mabe would make it faster)
	std::istringstream iss(str.substr(0, str.size()-1));
	uint64_t val;
	iss >> std::noskipws >> val;
	if(!iss.eof() || iss.fail())
		return std::nullopt;

	parse_debug_print("parsed uint literal");
	return makeParseRes(val, 1);
}

parseRes<int64_t> parseInt(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& str = std::get<basicToken>(tokens.front().value).val;
	std::istringstream iss(str);
	unsigned int outputSize = 1;
	if(str == "-"){
		tokens = tokens.subspan(1);
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		iss = std::istringstream(str + std::get<basicToken>(tokens.front().value).val);
		outputSize++;
	}
	int64_t val;
	iss >> std::noskipws >> val;
	if(!iss.eof() || iss.fail())
		return std::nullopt;

	parse_debug_print("parsed int literal");
	return makeParseRes(val, outputSize);
}

parseRes<float> parseFloat32(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& str = std::get<basicToken>(tokens.front().value).val;
	
	std::istringstream iss(str);
	unsigned int outputSize = 1;
	if(str == "-"){
		tokens = tokens.subspan(1);
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		const auto& str2 = std::get<basicToken>(tokens.front().value).val;
		if(!str2.ends_with('f'))
			return std::nullopt;
		iss = std::istringstream(str + str2);
		outputSize++;
	}else{
		if(!str.ends_with('f'))
			return std::nullopt;
		iss = std::istringstream(str.substr(0, str.size()-1));
	}
	if(iss.str().find('.') == std::string::npos && iss.str().find('e') == std::string::npos)
		return std::nullopt;//An integer is an integer, not a float
	float val;
	iss >> std::noskipws >> val;
	if(!iss.eof() || iss.fail())
		return std::nullopt;
	return makeParseRes(val, outputSize);
}

parseRes<double> parseFloat64(std::span<const mediumToken> tokens){
	if(tokens.empty())
		return std::nullopt;
	if(!std::holds_alternative<basicToken>(tokens.front().value))
		return std::nullopt;
	const auto& str = std::get<basicToken>(tokens.front().value).val;
	
	std::istringstream iss(str);
	unsigned int outputSize = 1;
	if(str == "-"){
		tokens = tokens.subspan(1);
		if(tokens.empty())
			return std::nullopt;
		if(!std::holds_alternative<basicToken>(tokens.front().value))
			return std::nullopt;
		iss = std::istringstream(str + std::get<basicToken>(tokens.front().value).val);
		outputSize++;
	}
	if(iss.str().find('.') == std::string::npos && iss.str().find('e') == std::string::npos)
		return std::nullopt;//An integer is an integer, not a float
	double val;
	iss >> std::noskipws >> val;
	if(!iss.eof() || iss.fail())
		return std::nullopt;
	return makeParseRes(val, outputSize);
}
}

parseRes<minLang::ast::literal> minLang::parseLiteral(std::span<const mediumToken> tokens){
	auto boolTry = parseBool(tokens);
	if(boolTry)
		return makeParseRes(minLang::ast::literal(boolTry->val), boolTry->toksConsumed);
	auto float32Try = parseFloat32(tokens);
	if(float32Try)
		return makeParseRes(minLang::ast::literal(float32Try->val), float32Try->toksConsumed);
	auto float64Try = parseFloat64(tokens);
	if(float64Try)
		return makeParseRes(minLang::ast::literal(float64Try->val), float64Try->toksConsumed);
	auto uintTry = parseUInt(tokens);
	if(uintTry){
		if(uintTry->val > std::numeric_limits<uint32_t>::max()){
			return makeParseRes(minLang::ast::literal(uintTry->val), uintTry->toksConsumed);
		}else{
			return makeParseRes(minLang::ast::literal((uint32_t)uintTry->val), uintTry->toksConsumed);
		}
	}
	auto intTry = parseInt(tokens);
	if(intTry){
		if(std::abs(intTry->val) > std::numeric_limits<int32_t>::max()){
			return makeParseRes(minLang::ast::literal(intTry->val), intTry->toksConsumed);
		}else{
			//parse_debug_print("demoted int64 to int32 (not big enough)");
			return makeParseRes(minLang::ast::literal((int32_t)intTry->val), intTry->toksConsumed);
		}
	}
	return std::nullopt;
}

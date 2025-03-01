#include "ast_type.hpp"
#include <iostream>
#include "../parser/parse/parseType.hpp"
#include "../parser/tokenize/basicTokenize.hpp"
#include "../parser/tokenize/mediumTokenize.hpp"
#include "../hashCombine.hpp"

minLang::ast::type minLang::ast::type::fromString(const std::string_view type){
	auto basicToks = basicTokenizeString(type);
	if(!basicToks)
		return none_type;
	auto mediumToks = parseBrackets(*basicToks);
	if(!mediumToks)
		return none_type;
	auto typeTry = parseType(*mediumToks);
	if(!typeTry)
		return none_type;
	return typeTry->val;
}

std::string minLang::ast::type::toString() const{
	if(std::holds_alternative<builtin_type>(ty)){
		switch(std::get<builtin_type>(ty)){
			case void_type:
				return "void";
			case int8_type:
				return "int8";
			case int16_type:
				return "int16";
			case int32_type:
				return "int32";
			case int64_type:
				return "int64";
			case uint8_type:
				return "uint8";
			case uint16_type:
				return "uint16";
			case uint32_type:
				return "uint32";
			case uint64_type:
				return "uint64";
			case float32_type:
				return "float32";
			case float64_type:
				return "float64";
			case bool_type:
				return "bool";
			default:
				return "none";
		}
	}else if(std::holds_alternative<array_type>(ty)){
		const auto& array = std::get<array_type>(ty);
		return array.ty->toString() + "[" + std::to_string(array.length) + "]";
	}else{
		std::cerr<<"An unknown error (theoretically impossible) in type::toString has occurred"<<std::endl;
		return "void";
	}
}

unsigned int minLang::ast::type::getSize() const{
	if(std::holds_alternative<minLang::ast::type::builtin_type>(ty)){
		switch(std::get<minLang::ast::type::builtin_type>(ty)){
			case minLang::ast::type::void_type:
				return 0;
			case minLang::ast::type::int8_type:
			case minLang::ast::type::uint8_type:
				return 1;
			case minLang::ast::type::int16_type:
			case minLang::ast::type::uint16_type:
				return 2;
			case minLang::ast::type::int32_type:
			case minLang::ast::type::uint32_type:
			case minLang::ast::type::float32_type:
				return 4;
			case minLang::ast::type::int64_type:
			case minLang::ast::type::uint64_type:
			case minLang::ast::type::float64_type:
				return 4;
			case minLang::ast::type::bool_type:
				return 1;
			default:
				std::cerr<<"Error: Encountered builtin type of unknown size! (all should be known)"<<std::endl;
				return 0;
		}
	}else if(std::holds_alternative<minLang::ast::type::array_type>(ty)){
		const auto& array_ty = std::get<minLang::ast::type::array_type>(ty);
		return array_ty.ty->getSize() * array_ty.length;
	}else{
		std::cerr<<"Error: unknown type of type"<<std::endl;
		return 0;
	}
}

minLang::ast::type minLang::ast::type::clone() const{
	if(std::holds_alternative<builtin_type>(ty)){
		return *this;
	}else if(std::holds_alternative<array_type>(ty)){
		const auto& array = std::get<array_type>(ty);
		array_type output;
		output.ty = std::make_shared<type>(*array.ty);
		output.length = array.length;
		return output;
	}else{
		return minLang::ast::type::builtin_type::none_type;
	}
}

std::size_t minLang::ast::type::hash() const{
	if(std::holds_alternative<builtin_type>(ty)){
		return hashing::hashValues(COMPILE_TIME_CRC32_STR("builtin_type"), std::get<builtin_type>(ty));
	}else if(std::holds_alternative<array_type>(ty)){
		const auto& array = std::get<array_type>(ty);
		return hashing::hashValues(COMPILE_TIME_CRC32_STR("array_type"), array.ty->hash(), array.length);
	}else{
		return 0;
	}
}

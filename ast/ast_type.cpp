#include "ast_type.hpp"
#include <iostream>
#include "../parser/parse/parseType.hpp"
#include "../parser/tokenize/basicTokenize.hpp"
#include "../parser/tokenize/mediumTokenize.hpp"
#include "../hashCombine.hpp"

ast::type ast::type::fromString(const std::string_view type){
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

std::string ast::type::toString() const{
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
	}else if(std::holds_alternative<tuple_type>(ty)){
		const auto& tuple = std::get<tuple_type>(ty);
		std::string output = "(";
		for(unsigned int i=0;i<tuple.size();i++){
			output += tuple[i].toString();
			if(i+1 < tuple.size())
				output += ", ";
		}
		output += ")";
		return output;
	}else if(std::holds_alternative<alias_type>(ty)){
		const auto& alias = std::get<alias_type>(ty);
		std::string output = alias.name + " (";
		if(alias.strict)
			output += "strict ";
		output += "alias of ";
		output += alias.underlyingType->toString();
		output += ")";
		return output;
	}else if(std::holds_alternative<template_type>(ty)){
		return "template type";
	}else{
		std::cerr<<"An unknown error (theoretically impossible) in type::toString has occurred"<<std::endl;
		return "void";
	}
}

unsigned int ast::type::getSize() const{
	if(std::holds_alternative<ast::type::builtin_type>(ty)){
		switch(std::get<ast::type::builtin_type>(ty)){
			case ast::type::void_type:
				return 0;
			case ast::type::int8_type:
			case ast::type::uint8_type:
				return 1;
			case ast::type::int16_type:
			case ast::type::uint16_type:
				return 2;
			case ast::type::int32_type:
			case ast::type::uint32_type:
			case ast::type::float32_type:
				return 4;
			case ast::type::int64_type:
			case ast::type::uint64_type:
			case ast::type::float64_type:
				return 4;
			case ast::type::bool_type:
				return 1;
			default:
				std::cerr<<"Error: Encountered builtin type of unknown size! (all should be known)"<<std::endl;
				return 0;
		}
	}else if(std::holds_alternative<ast::type::array_type>(ty)){
		const auto& array_ty = std::get<ast::type::array_type>(ty);
		return array_ty.ty->getSize() * array_ty.length;
	}else if(std::holds_alternative<ast::type::tuple_type>(ty)){
		const auto& tuple_ty = std::get<ast::type::tuple_type>(ty);
		unsigned int output = 0;
		for(const auto& o : tuple_ty){
			output += o.getSize();
		}
		return output;
	}else if(std::holds_alternative<ast::type::tuple_type>(ty)){
		return 8;// must store 64 bit addr I suppose (though this should really be platform dependent)
	}else if(std::holds_alternative<ast::type::alias_type>(ty)){
		return std::get<alias_type>(ty).underlyingType->getSize();
	}else if(std::holds_alternative<template_type>(ty)){
		std::cerr<<"Warning: got size of template type (template should be instantiated first)"<<std::endl;
		return 0;
	}else{
		std::cerr<<"Error: unknown type of type"<<std::endl;
		return 0;
	}
}

ast::type ast::type::fullyDealias(const ast::type& ty){
	if(std::holds_alternative<ast::type::alias_type>(ty.ty)){
		return fullyDealias(*std::get<ast::type::alias_type>(ty.ty).underlyingType);
	}
	return ty;
}

//an alias will always be made to be an alias of a basic (builtin/tuple/array) type, eg. aliases of aliases will be simplified.
ast::type::type(const type::alias_type& alias_ty){
	alias_type al_ty{alias_ty.name};
	al_ty.strict = alias_ty.strict;
	al_ty.underlyingType = std::make_shared<type>(fullyDealias(*alias_ty.underlyingType));
	ty = al_ty;
}

ast::type ast::type::clone() const{
	if(std::holds_alternative<builtin_type>(ty)){
		return *this;
	}else if(std::holds_alternative<array_type>(ty)){
		const auto& array = std::get<array_type>(ty);
		array_type output;
		output.ty = std::make_shared<type>(*array.ty);
		output.length = array.length;
		return output;
	}else if(std::holds_alternative<tuple_type>(ty)){
		const auto& tuple = std::get<tuple_type>(ty);
		tuple_type output;
		output.resize(tuple.size());
		for(unsigned int i=0;i<output.size();i++){
			output[i] = tuple[i].clone();
		}
		return output;
	}else if(std::holds_alternative<alias_type>(ty)){
		const auto& alias = std::get<alias_type>(ty);
		alias_type output;
		output.underlyingType = std::make_shared<type>(*alias.underlyingType);
		output.name = alias.name;
		output.strict = alias.strict;
		return output;
	}else if(std::holds_alternative<template_type>(ty)){
		return *this;
	}else{
		return ast::type::builtin_type::none_type;
	}
}

std::size_t ast::type::hash() const{
	if(std::holds_alternative<builtin_type>(ty)){
		return hashing::hashValues(COMPILE_TIME_CRC32_STR("builtin_type"), std::get<builtin_type>(ty));
	}else if(std::holds_alternative<array_type>(ty)){
		const auto& array = std::get<array_type>(ty);
		return hashing::hashValues(COMPILE_TIME_CRC32_STR("array_type"), array.ty->hash(), array.length);
	}else if(std::holds_alternative<tuple_type>(ty)){
		const auto& tuple = std::get<tuple_type>(ty);
		std::size_t output = COMPILE_TIME_CRC32_STR("tuple_type");
		for(const auto& tuple_ty : tuple)
			output = hashing::hashValues(output, tuple_ty.hash());
		return output;
	}else if(std::holds_alternative<alias_type>(ty)){
		const auto& alias = std::get<alias_type>(ty);
		alias_type output;
		return hashing::hashValues(COMPILE_TIME_CRC32_STR("alias_type"), alias.name, alias.strict, alias.underlyingType->hash());
	}else if(std::holds_alternative<template_type>(ty)){
		return hashing::hashValues(COMPILE_TIME_CRC32_STR("template_type"), std::get<template_type>(ty).templateParamNum);
	}else{
		return 0;
	}
}

#include "ast_type.hpp"
#include <iostream>
#include "../parser/parse/parseType.hpp"
#include "../parser/tokenize/basicTokenize.hpp"
#include "../parser/tokenize/mediumTokenize.hpp"

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
			case int_type:
				return "int";
			case float_type:
				return "float";
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
			case ast::type::int_type://assuming int32
				return 4;
			case ast::type::bool_type:
				return 1;
			case ast::type::float_type://assuming float32
				return 4;
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
	}else{
		std::cerr<<"Error: unknown type of type"<<std::endl;
		return 0;
	}
}


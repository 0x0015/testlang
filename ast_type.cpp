#include "ast_type.hpp"
#include <iostream>
#include "parse/type.hpp"

#include <lexy/action/parse.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

ast::type ast::type_map(const std::string_view type){
	auto input = lexy::string_input<lexy::utf8_encoding>(type);
	auto result = lexy::parse<grammer::type_t>(input, lexy_ext::report_error);

	if(result.has_value()){
		return result.value();
	}else{
		return none_type;
	}
}

std::string ast::type_rmap(const type& ty){
	if(std::holds_alternative<builtin_type>(ty.ty)){
		switch(std::get<builtin_type>(ty.ty)){
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
	}else if(std::holds_alternative<array_type>(ty.ty)){
		const auto& array = std::get<array_type>(ty.ty);
		return type_rmap(*array.ty) + "[]";
	}else if(std::holds_alternative<tuple_type>(ty.ty)){
		const auto& tuple = std::get<tuple_type>(ty.ty);
		std::string output = "(";
		for(const auto& o : tuple)
			output += type_rmap(o);
		output += ")";
		return output;
	}else{
		std::cerr<<"An unknown error (theoretically impossible) in type_rmap has occurred"<<std::endl;
		return "void";
	}
}

unsigned int ast::type::getSize() const{
	if(std::holds_alternative<ast::builtin_type>(ty)){
		switch(std::get<ast::builtin_type>(ty)){
			case ast::void_type:
				return 0;
			case ast::int_type://assuming int32
				return 4;
			case ast::bool_type:
				return 1;
			case ast::float_type://assuming float32
				return 4;
			default:
				std::cerr<<"Error: Encountered builtin type of unknown size! (all should be known)"<<std::endl;
				return 0;
		}
	}else if(std::holds_alternative<ast::array_type>(ty)){
		const auto& array_ty = std::get<ast::array_type>(ty);
		return array_ty.ty->getSize() * array_ty.length;
	}else if(std::holds_alternative<ast::tuple_type>(ty)){
		const auto& tuple_ty = std::get<ast::tuple_type>(ty);
		unsigned int output = 0;
		for(const auto& o : tuple_ty){
			output += o.getSize();
		}
		return output;
	}else{
		std::cerr<<"Error: unknown type of type"<<std::endl;
		return 0;
	}
}


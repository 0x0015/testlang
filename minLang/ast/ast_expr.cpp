#include "ast_expr.hpp"
#include <iostream>
#include "ast.hpp"

void minLang::ast::expr::dump() const{
	if(std::holds_alternative<literal>(value)){
		const auto& lit = std::get<literal>(value);
		std::cout<<lit.toString();
	}else if(std::holds_alternative<call>(value)){
		const auto& cll = std::get<call>(value);
		std::cout<<"\tCall: "<<cll.name<<"(";
		for(unsigned int i=0;i<cll.args.size();i++){
			cll.args[i].dump();
			if(i+1 < cll.args.size())
				std::cout<<", ";
		}
		std::cout<<")";
		if(cll.validatedDef){
			std::cout<<"「matched」";
		}
		std::cout<<std::endl;
	}else if(std::holds_alternative<varName>(value)){
		std::cout<<std::get<varName>(value).name;
	}else{
		std::cout<<"[Unknown Expression]";
	}
}

minLang::ast::expr minLang::ast::expr::clone() const{
	if(std::holds_alternative<literal>(value)){
		return *this;
	}else if(std::holds_alternative<call>(value)){
		const auto& cll = std::get<call>(value);
		call output{cll};
		for(auto& arg : output.args)
			arg = arg.clone();
		return output;
	}else if(std::holds_alternative<varName>(value)){
		return *this;
	}else{
		std::cout<<"[Unknown Expression]";
		return *this;
	}
}

std::optional<minLang::ast::type> minLang::ast::expr::inferType() const{
	if(std::holds_alternative<literal>(value)){
		return std::get<literal>(value).ty;
	}else if(std::holds_alternative<call>(value)){
		const auto& cll = std::get<call>(value);
		if(!cll.validatedDef)
			return std::nullopt;
		return cll.validatedDef->get().ty;
	}else if(std::holds_alternative<varName>(value)){
		const auto& var = std::get<varName>(value);
		if(!var.matchedType)
			return std::nullopt;
		return *var.matchedType;
	}else{
		return std::nullopt;
	}
}

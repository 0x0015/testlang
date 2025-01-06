#include "ast_expr.hpp"
#include <iostream>

void ast::expr::dump() const{
	if(std::holds_alternative<literal>(value)){
		const auto& lit = std::get<literal>(value);
		std::cout<<lit.toString();
	}else if(std::holds_alternative<call>(value)){
		const auto& cll = std::get<call>(value);
		std::cout<<"\tCall: "<<cll.name<<"( ";
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

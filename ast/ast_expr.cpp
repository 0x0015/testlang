#include "ast_expr.hpp"
#include <iostream>
#include "ast.hpp"

void ast::expr::dump() const{
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
	}else if(std::holds_alternative<templateCall>(value)){
		const auto& tcll = std::get<templateCall>(value);
		std::cout<<"\tTemplate Call: "<<tcll.name<<"<";
		for(unsigned int i=0;i<tcll.templateArgs.size();i++){
			std::cout<<tcll.templateArgs[i].toString();
			if(i+1 < tcll.templateArgs.size())
				std::cout<<", ";
		}
		std::cout<<">(";
		for(unsigned int i=0;i<tcll.args.size();i++){
			tcll.args[i].dump();
			if(i+1 < tcll.args.size())
				std::cout<<", ";
		}
		std::cout<<")";
	}else if(std::holds_alternative<varName>(value)){
		std::cout<<std::get<varName>(value).name;
	}else{
		std::cout<<"[Unknown Expression]";
	}
}

ast::expr ast::expr::clone() const{
	if(std::holds_alternative<literal>(value)){
		return *this;
	}else if(std::holds_alternative<call>(value)){
		const auto& cll = std::get<call>(value);
		call output{cll};
		for(auto& arg : output.args)
			arg = arg.clone();
		return output;
	}else if(std::holds_alternative<templateCall>(value)){
		const auto& tcll = std::get<templateCall>(value);
		templateCall output{tcll};
		for(auto& targ : output.templateArgs)
			targ = targ.clone();
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

std::optional<ast::type> ast::expr::inferType() const{
	if(std::holds_alternative<literal>(value)){
		return std::get<literal>(value).ty;
	}else if(std::holds_alternative<call>(value)){
		const auto& cll = std::get<call>(value);
		if(!cll.validatedDef)
			return std::nullopt;
		return cll.validatedDef->get().ty;
	}else if(std::holds_alternative<templateCall>(value)){
		return std::nullopt;//we have no idea what type a non-instantiated template returns
	}else if(std::holds_alternative<varName>(value)){
		const auto& var = std::get<varName>(value);
		if(!var.matchedType)
			return std::nullopt;
		return *var.matchedType;
	}else{
		return std::nullopt;
	}
}

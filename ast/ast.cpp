#include "ast.hpp"

void ast::function::dump() const{
	std::cout<<ty.toString()<<" "<<name<<"("<<std::endl;
	for(const auto& arg : args){
		std::cout<<"\t"<<arg.ty.toString()<<" "<<arg.name<<std::endl;
	}
	std::cout<<"){"<<std::endl;
	for(const auto& state : body){
		if(std::holds_alternative<declaration>(state)){
			const auto& decl = std::get<declaration>(state);
			std::cout<<"\tDeclaration: "<<decl.ty.toString()<<" "<<decl.name<<std::endl;
		}else if(std::holds_alternative<expr>(state)){
			const auto& exp = std::get<expr>(state);
			exp.dump();
		}else if(std::holds_alternative<assignment>(state)){
			const auto& asgn = std::get<assignment>(state);
			std::cout<<"\tAssignment: "<<asgn.assignTo<<" = ";
			asgn.assignFrom.dump();
		}else{
			std::cout<<"\tUnknown statement"<<std::endl;
		}
	}
	std::cout<<"}"<<std::endl;
}

void ast::context::dump() const{
	std::cout<<"Context:"<<std::endl;
	for(const auto& func : funcs){
		std::cout<<"function: ";
		func.dump();
	}
}



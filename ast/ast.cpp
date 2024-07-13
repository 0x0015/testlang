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
		}else if(std::holds_alternative<call>(state)){
			const auto& cll = std::get<call>(state);
			std::cout<<"\tCall: "<<cll.name<<"( ";
			for(const auto& arg : cll.args){
				if(std::holds_alternative<call::varNameArg>(arg)){
					std::cout<<std::get<call::varNameArg>(arg)<<" ";
				}else if(std::holds_alternative<ast::literal>(arg)){
					std::cout<<std::get<ast::literal>(arg).toString()<<" ";
				}else{
					std::cout<<"Unknown call arg"<<std::endl;
				}
			}
			std::cout<<")";
			if(cll.validatedDef){
				std::cout<<" ((matched))";
			}
			std::cout<<std::endl;
		}else if(std::holds_alternative<assignment>(state)){
			const auto& asgn = std::get<assignment>(state);
			std::cout<<"\tAssignment: "<<asgn.assignTo<<" = ";
			if(std::holds_alternative<std::string>(asgn.assignFrom)){
				std::cout<<std::get<std::string>(asgn.assignFrom)<<std::endl;
			}else if(std::holds_alternative<ast::literal>(asgn.assignFrom)){
				std::cout<<std::get<literal>(asgn.assignFrom).toString()<<std::endl;
			}else{
				std::cout<<"Unknown assignment from"<<std::endl;
			}
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



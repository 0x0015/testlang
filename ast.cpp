#include "ast.hpp"

void ast::function::dump() const{
	std::cout<<type_rmap(ty)<<" "<<name<<"("<<std::endl;
	for(const auto& arg : args){
		std::cout<<"\t"<<type_rmap(arg.ty)<<" "<<arg.name<<std::endl;
	}
	std::cout<<"){"<<std::endl;
	for(const auto& state : body){
		if(std::holds_alternative<declaration>(state)){
			const auto& decl = std::get<declaration>(state);
			std::cout<<"\tDeclaration: "<<type_rmap(decl.ty)<<" "<<decl.name<<std::endl;
		}else if(std::holds_alternative<call>(state)){
			const auto& cll = std::get<call>(state);
			std::cout<<"\tCall: "<<cll.name<<"( ";
			for(const auto& arg : cll.args)
				std::cout<<arg<<" ";
			std::cout<<")";
			if(cll.validatedDef){
				std::cout<<" ((matched))";
			}
			std::cout<<std::endl;
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



#include "ast.hpp"

void ast::function::dump() const{
	std::cout<<ty.toString()<<" "<<name<<"("<<std::endl;
	for(const auto& arg : args){
		std::cout<<"\t"<<arg.ty.toString()<<" "<<arg.name<<std::endl;
	}
	std::cout<<")";
	body.dump();
}

void ast::context::dump() const{
	std::cout<<"Context:"<<std::endl;
	for(const auto& func : funcs){
		std::cout<<"function: ";
		func.dump();
	}
}



#include "ast.hpp"

void minLang::ast::function::dump() const{
	std::cout<<ty.toString()<<" "<<name<<"("<<std::endl;
	for(const auto& arg : args){
		std::cout<<"\t"<<arg.ty.toString()<<" "<<arg.name<<std::endl;
	}
	std::cout<<")";
	body.dump();
}

void minLang::ast::context::dump() const{
	std::cout<<"Context:"<<std::endl;
	for(const auto& func : funcs){
		std::cout<<"function: ";
		func.dump();
	}
}

minLang::ast::function minLang::ast::function::clone() const{
	function output{ty.clone(), name, std::vector<argument>(args.size()), {}, status};
	for(unsigned int i=0;i<args.size();i++){
		output.args[i] = argument{args[i].ty.clone(), args[i].name};
	}
	output.body = body.clone();
	return output;
}


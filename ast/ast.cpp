#include "ast.hpp"
#include "../parser/parse/parseTemplate.hpp"

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

ast::function ast::function::clone() const{
	function output{ty.clone(), name, std::vector<argument>(args.size()), {}, status};
	for(unsigned int i=0;i<args.size();i++){
		output.args[i] = argument{args[i].ty.clone(), args[i].name};
	}
	output.body = body.clone();
	return output;
}

void switchTemplatesForReplacements(ast::expr& exp, const std::vector<ast::type>& replacementTypes){
	if(std::holds_alternative<ast::call>(exp.value)){
		auto& call = std::get<ast::call>(exp.value);
		for(auto& arg : call.args)
			switchTemplatesForReplacements(arg, replacementTypes);
	}else if(std::holds_alternative<ast::templateCall>(exp.value)){
		auto& tcall = std::get<ast::templateCall>(exp.value);
		for(auto& targ : tcall.templateArgs)
			switchTemplatesForReplacements(targ, replacementTypes);
		for(auto& arg : tcall.args)
			switchTemplatesForReplacements(arg, replacementTypes);
	}
}

void switchTemplatesForReplacements(ast::block& block, const std::vector<ast::type>& replacementTypes){
	for(auto& statement : block.statements){
		if(std::holds_alternative<ast::block::declaration>(statement)){
			auto& decl = std::get<ast::block::declaration>(statement);
			switchTemplatesForReplacements(decl.ty, replacementTypes);
		}else if(std::holds_alternative<ast::block::assignment>(statement)){
			auto& asgn = std::get<ast::block::assignment>(statement);
			switchTemplatesForReplacements(asgn.assignFrom, replacementTypes);
		}else if(std::holds_alternative<ast::block::ifStatement>(statement)){
			auto& ifState = std::get<ast::block::ifStatement>(statement);
			switchTemplatesForReplacements(ifState.condition, replacementTypes);
			switchTemplatesForReplacements(*ifState.ifBody, replacementTypes);
			switchTemplatesForReplacements(*ifState.elseBody, replacementTypes);
		}else if(std::holds_alternative<ast::block::returnStatement>(statement)){
			auto& ret = std::get<ast::block::returnStatement>(statement);
			switchTemplatesForReplacements(ret.val, replacementTypes);
		}else if(std::holds_alternative<ast::expr>(statement)){
			switchTemplatesForReplacements(std::get<ast::expr>(statement), replacementTypes);
		}else{
			std::cerr<<"Error: unknown block statement type in template instantiation"<<std::endl;
		}
	}
}

ast::function ast::functionTemplate::instantiate(const std::vector<ast::type>& replacementTypes) const{
	std::cout<<"Instantiation stub: "<<func.name<<std::endl;
	auto output = func.clone();
	for(auto& arg : output.args){
		switchTemplatesForReplacements(arg.ty, replacementTypes);
	}

	switchTemplatesForReplacements(output.body, replacementTypes);
	output.name = output.name + "<";
	for(auto& repTy : replacementTypes)
		output.name += repTy.toString();
	output.name += ">";
	return output;
}


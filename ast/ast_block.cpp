#include "ast_block.hpp"

void ast::block::dump() const{
	std::cout<<"{"<<std::endl;
	for(const auto& state : statements){
		if(std::holds_alternative<declaration>(state)){
			const auto& decl = std::get<declaration>(state);
			std::cout<<"\tDeclaration: "<<decl.ty.toString()<<" "<<decl.name<<std::endl;
		}else if(std::holds_alternative<expr>(state)){
			const auto& exp = std::get<expr>(state);
			std::cout<<"\tExpr: ";
			exp.dump();
			std::cout<<";"<<std::endl;
		}else if(std::holds_alternative<assignment>(state)){
			const auto& asgn = std::get<assignment>(state);
			std::cout<<"\tAssignment: "<<asgn.assignTo<<" = ";
			asgn.assignFrom.dump();
			std::cout<<";"<<std::endl;
		}else if(std::holds_alternative<ifStatement>(state)){
			const auto& ifState = std::get<ifStatement>(state);
			std::cout<<"\tIf statement: if(";
			ifState.condition.dump();
			std::cout<<"\t) then ";
			ifState.ifBody->dump();
			std::cout<<"\t else ";
			ifState.elseBody->dump();
		}else if(std::holds_alternative<returnStatement>(state)){
			const auto& retState = std::get<returnStatement>(state);
			std::cout<<"\tReturn: ";
			retState.val.dump();
		}else{
			std::cout<<"\tUnknown statement"<<std::endl;
		}
	}
	std::cout<<"}"<<std::endl;
}

ast::block ast::block::clone() const{
	ast::block output{statements};
	for(auto& state : output.statements){
		if(std::holds_alternative<declaration>(state)){
			auto& decl = std::get<declaration>(state);
			decl.ty = decl.ty.clone();
		}else if(std::holds_alternative<expr>(state)){
			state = std::get<expr>(state).clone();
		}else if(std::holds_alternative<assignment>(state)){
			auto& asgn = std::get<assignment>(state);
			asgn.assignFrom = asgn.assignFrom.clone();
		}else if(std::holds_alternative<ifStatement>(state)){
			auto& ifState = std::get<ifStatement>(state);
			ifState.condition = ifState.condition.clone();
			ifState.ifBody = std::make_shared<ast::block>(ifState.ifBody->clone());
			ifState.elseBody = std::make_shared<ast::block>(ifState.elseBody->clone());
		}else if(std::holds_alternative<returnStatement>(state)){
			auto& retState = std::get<returnStatement>(state);
			retState.val = retState.val.clone();
		}else{
			std::cout<<"\tUnknown statement"<<std::endl;
		}
	}
	return output;
}


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
			std::cout<<"if statement: if(";
			ifState.condition.dump();
			std::cout<<") then ";
			ifState.ifBody->dump();
			std::cout<<" else ";
			ifState.elseBody->dump();
		}else{
			std::cout<<"\tUnknown statement"<<std::endl;
		}
	}
	std::cout<<"}"<<std::endl;
}

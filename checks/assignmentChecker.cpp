#include "assignmentChecker.hpp"

bool checkAssignmentsValid(const ast::context& context){
	bool errored = false;
	for(const auto& func : context.funcs){
		for(const auto& state : func.body){
			if(std::holds_alternative<ast::function::assignment>(state)){
				const auto& asgn = std::get<ast::function::assignment>(state);
			}
		}
	}
}


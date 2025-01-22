#include "builtins.hpp"

void addBuiltins(ast::context& context){
	addPrints(context);
	addArithmatic(context);
	addLogic(context);
}

void addBuiltin(ast::context& context, const ast::function& func){
	context.funcs.push_back(func);
}


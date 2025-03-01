#include "builtins.hpp"

void addBuiltins(minLang::ast::context& context){
	addPrints(context);
	addArithmatic(context);
	addLogic(context);
	addConversions(context);
}

void addBuiltin(minLang::ast::context& context, const minLang::ast::function& func){
	context.funcs.push_back(func);
}


#include "builtins.hpp"

void addBuiltins(ast::context& context){
	addPrints(context);
	addArithmatic(context);
	addLogic(context);
	addArrayOpFuncs(context);
}

void addBuiltin(ast::context& context, const ast::function& func){
	context.funcs.push_back(func);
}

void addBuiltinTemplateFunc(ast::context& context, const ast::functionTemplate& funcTempl){
	context.funcTemplates.push_back(funcTempl);
}

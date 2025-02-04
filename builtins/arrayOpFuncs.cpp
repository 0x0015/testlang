#include "builtins.hpp"

void addArrayOpFuncs(ast::context& context){
	//for an array T[size], template type 1 is T, temlpate type 0 is T[size]
	ast::functionTemplate get{2, ast::function{ast::type::template_type{1}, "get", {ast::function::argument{ast::type::template_type{0}, "array"}, ast::function::argument{ast::type::builtin_type::int_type, "index"}}, {}, ast::function::positionStatus::builtin}};
	addBuiltinTemplateFunc(context, get);

	ast::functionTemplate set{2, ast::function{ast::type::template_type{0}, "set", {ast::function::argument{ast::type::template_type{0}, "array"}, ast::function::argument{ast::type::builtin_type::int_type, "index"}, ast::function::argument{ast::type::template_type{1}, "val"}}, {}, ast::function::positionStatus::builtin}};
	addBuiltinTemplateFunc(context, set);
}


#include "builtins.hpp"

void addPrints(ast::context& context){
	ast::function void_print;
	void_print.name = "print";
	void_print.ty = ast::void_type;
	void_print.args = {};
	void_print.builtin = true;
	context.funcs.push_back(void_print);

	ast::function int_print;
	int_print.name = "print";
	int_print.ty = ast::void_type;
	int_print.args = {{ast::int_type, "val"}};
	int_print.builtin = true;
	context.funcs.push_back(int_print);

	ast::function float_print;
	float_print.name = "print";
	float_print.ty = ast::void_type;
	float_print.args = {{ast::float_type, "val"}};
	float_print.builtin = true;
	context.funcs.push_back(float_print);

	ast::function bool_print;
	bool_print.name = "print";
	bool_print.ty = ast::void_type;
	bool_print.args = {{ast::bool_type, "val"}};
	bool_print.builtin = true;
	context.funcs.push_back(bool_print);
}

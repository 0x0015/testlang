#include "builtins.hpp"

void addPrints(ast::context& context){
	//print
	ast::function int_print;
	int_print.name = "print";
	int_print.ty = ast::type::void_type;
	int_print.args = {{ast::type::int_type, "val"}};
	int_print.builtin = true;
	addBuiltin(context, int_print);

	ast::function float_print;
	float_print.name = "print";
	float_print.ty = ast::type::void_type;
	float_print.args = {{ast::type::float_type, "val"}};
	float_print.builtin = true;
	addBuiltin(context, float_print);

	ast::function bool_print;
	bool_print.name = "print";
	bool_print.ty = ast::type::void_type;
	bool_print.args = {{ast::type::bool_type, "val"}};
	bool_print.builtin = true;
	addBuiltin(context, bool_print);

	//println
	ast::function int_println;
	int_println.name = "println";
	int_println.ty = ast::type::void_type;
	int_println.args = {{ast::type::int_type, "val"}};
	int_println.builtin = true;
	addBuiltin(context, int_println);

	ast::function float_println;
	float_println.name = "println";
	float_println.ty = ast::type::void_type;
	float_println.args = {{ast::type::float_type, "val"}};
	float_println.builtin = true;
	addBuiltin(context, float_println);

	ast::function bool_println;
	bool_println.name = "println";
	bool_println.ty = ast::type::void_type;
	bool_println.args = {{ast::type::bool_type, "val"}};
	bool_println.builtin = true;
	addBuiltin(context, bool_println);
}

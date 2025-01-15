#include "cCodeGen.hpp"

std::string cCodeGen::genBuiltins(){
	std::string output;

	ast::function print_int_func;
	print_int_func.name = "print";
	print_int_func.ty = ast::type::builtin_type::void_type;
	print_int_func.args = {ast::function::argument{ast::type::builtin_type::int_type}};
	output += "void " + cCodeGen::mangleFuncName(print_int_func) + "(int val){\n\tprintf(\"%i\", val);\n}\n";

	ast::function print_bool_func;
	print_bool_func.name = "print";
	print_bool_func.ty = ast::type::builtin_type::void_type;
	print_bool_func.args = {ast::function::argument{ast::type::builtin_type::bool_type}};
	output += "void " + cCodeGen::mangleFuncName(print_bool_func) + "(unsigned char val){\n\tprintf(val ? \"true\" : \"false\");\n}\n";

	ast::function print_float_func;
	print_float_func.name = "print";
	print_float_func.ty = ast::type::builtin_type::void_type;
	print_float_func.args = {ast::function::argument{ast::type::builtin_type::float_type}};
	output += "void " + cCodeGen::mangleFuncName(print_float_func) + "(float val){\n\tprintf(\"%f\", val);\n}\n";

	ast::function println_int_func;
	println_int_func.name = "println";
	println_int_func.ty = ast::type::builtin_type::void_type;
	println_int_func.args = {ast::function::argument{ast::type::builtin_type::int_type}};
	output += "void " + cCodeGen::mangleFuncName(println_int_func) + "(int val){\n\tprintf(\"%i\\n\", val);\n}\n";

	ast::function println_bool_func;
	println_bool_func.name = "println";
	println_bool_func.ty = ast::type::builtin_type::void_type;
	println_bool_func.args = {ast::function::argument{ast::type::builtin_type::bool_type}};
	output += "void " + cCodeGen::mangleFuncName(println_bool_func) + "(unsigned char val){\n\tprintf(val ? \"true\\n\" : \"false\\n\");\n}\n";

	ast::function println_float_func;
	println_float_func.name = "println";
	println_float_func.ty = ast::type::builtin_type::void_type;
	println_float_func.args = {ast::function::argument{ast::type::builtin_type::float_type}};
	output += "void " + cCodeGen::mangleFuncName(println_float_func) + "(float val){\n\tprintf(\"%f\\n\", val);\n}\n";

	return output;
}


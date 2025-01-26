#include "cCodeGen.hpp"

std::string cCodeGen::genBuiltins(){
	std::string output;

	//prints
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

	//arithmatic
	ast::function add_int_func;
	add_int_func.name = "add";
	add_int_func.ty = ast::type::builtin_type::int_type;
	add_int_func.args = {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}};
	output += "int " + cCodeGen::mangleFuncName(add_int_func) + "(int val1, int val2){\n\treturn(val1+val2);\n}\n";

	ast::function sub_int_func{ast::type::builtin_type::int_type, "sub", {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}}};
	output += "int " + cCodeGen::mangleFuncName(sub_int_func) + "(int val1, int val2){\n\treturn(val1-val2);\n}\n";

	ast::function mul_int_func{ast::type::builtin_type::int_type, "mul", {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}}};
	output += "int " + cCodeGen::mangleFuncName(mul_int_func) + "(int val1, int val2){\n\treturn(val1*val2);\n}\n";

	ast::function div_int_func{ast::type::builtin_type::int_type, "div", {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}}};
	output += "int " + cCodeGen::mangleFuncName(div_int_func) + "(int val1, int val2){\n\treturn(val1/val2);\n}\n";

	ast::function mod_int_func{ast::type::builtin_type::int_type, "mod", {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}}};
	output += "int " + cCodeGen::mangleFuncName(mod_int_func) + "(int val1, int val2){\n\treturn(val1%val2);\n}\n";


	ast::function add_float_func{ast::type::builtin_type::float_type, "add", {ast::function::argument{ast::type::builtin_type::float_type}, ast::function::argument{ast::type::builtin_type::float_type}}};
	output += "float " + cCodeGen::mangleFuncName(add_float_func) + "(float val1, float val2){\n\treturn(val1+val2);\n}\n";

	ast::function sub_float_func{ast::type::builtin_type::float_type, "sub", {ast::function::argument{ast::type::builtin_type::float_type}, ast::function::argument{ast::type::builtin_type::float_type}}};
	output += "float " + cCodeGen::mangleFuncName(sub_float_func) + "(float val1, float val2){\n\treturn(val1-val2);\n}\n";

	ast::function mul_float_func{ast::type::builtin_type::float_type, "mul", {ast::function::argument{ast::type::builtin_type::float_type}, ast::function::argument{ast::type::builtin_type::float_type}}};
	output += "float " + cCodeGen::mangleFuncName(mul_float_func) + "(float val1, float val2){\n\treturn(val1*val2);\n}\n";

	ast::function div_float_func{ast::type::builtin_type::float_type, "div", {ast::function::argument{ast::type::builtin_type::float_type}, ast::function::argument{ast::type::builtin_type::float_type}}};
	output += "float " + cCodeGen::mangleFuncName(div_float_func) + "(float val1, float val2){\n\treturn(val1/val2);\n}\n";

	//logic
	ast::function equal_int_func{ast::type::builtin_type::bool_type, "equal", {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(equal_int_func) + "(int val1, int val2){\n\treturn(val1 == val2);\n}\n";

	ast::function equal_float_func{ast::type::builtin_type::bool_type, "equal", {ast::function::argument{ast::type::builtin_type::float_type}, ast::function::argument{ast::type::builtin_type::float_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(equal_float_func) + "(float val1, float val2){\n\treturn(val1 == val2);\n}\n";

	ast::function equal_bool_func{ast::type::builtin_type::bool_type, "equal", {ast::function::argument{ast::type::builtin_type::bool_type}, ast::function::argument{ast::type::builtin_type::bool_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(equal_bool_func) + "(unsigned char val1, unsigned char val2){\n\treturn(val1 == val2);\n}\n";

	ast::function greater_int_func{ast::type::builtin_type::bool_type, "greater", {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(greater_int_func) + "(int val1, int val2){\n\treturn(val1 > val2);\n}\n";

	ast::function greater_float_func{ast::type::builtin_type::bool_type, "greater", {ast::function::argument{ast::type::builtin_type::float_type}, ast::function::argument{ast::type::builtin_type::float_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(greater_float_func) + "(float val1, float val2){\n\treturn(val1 > val2);\n}\n";

	ast::function less_int_func{ast::type::builtin_type::bool_type, "less", {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(less_int_func) + "(int val1, int val2){\n\treturn(val1 < val2);\n}\n";

	ast::function less_float_func{ast::type::builtin_type::bool_type, "less", {ast::function::argument{ast::type::builtin_type::float_type}, ast::function::argument{ast::type::builtin_type::float_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(less_float_func) + "(float val1, float val2){\n\treturn(val1 < val2);\n}\n";

	ast::function greaterOrEqual_int_func{ast::type::builtin_type::bool_type, "greaterOrEqual", {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(greaterOrEqual_int_func) + "(int val1, int val2){\n\treturn(val1 >= val2);\n}\n";

	ast::function greaterOrEqual_float_func{ast::type::builtin_type::bool_type, "greaterOrEqual", {ast::function::argument{ast::type::builtin_type::float_type}, ast::function::argument{ast::type::builtin_type::float_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(greaterOrEqual_float_func) + "(float val1, float val2){\n\treturn(val1 >= val2);\n}\n";

	ast::function lessOrEqual_int_func{ast::type::builtin_type::bool_type, "lessOrEqual", {ast::function::argument{ast::type::builtin_type::int_type}, ast::function::argument{ast::type::builtin_type::int_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(lessOrEqual_int_func) + "(int val1, int val2){\n\treturn(val1 <= val2);\n}\n";

	ast::function lessOrEqual_float_func{ast::type::builtin_type::bool_type, "lessOrEqual", {ast::function::argument{ast::type::builtin_type::float_type}, ast::function::argument{ast::type::builtin_type::float_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(lessOrEqual_float_func) + "(float val1, float val2){\n\treturn(val1 <= val2);\n}\n";

	ast::function and_bool_func{ast::type::builtin_type::bool_type, "and", {ast::function::argument{ast::type::builtin_type::bool_type}, ast::function::argument{ast::type::builtin_type::bool_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(and_bool_func) + "(unsigned char val1, unsigned char val2){\n\treturn(val1 && val2);\n}\n";

	ast::function or_bool_func{ast::type::builtin_type::bool_type, "or", {ast::function::argument{ast::type::builtin_type::bool_type}, ast::function::argument{ast::type::builtin_type::bool_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(or_bool_func) + "(unsigned char val1, unsigned char val2){\n\treturn(val1 || val2);\n}\n";

	ast::function not_bool_func{ast::type::builtin_type::bool_type, "not", {ast::function::argument{ast::type::builtin_type::bool_type}}};
	output += "unsigned char " + cCodeGen::mangleFuncName(not_bool_func) + "(unsigned char val){\n\treturn(!val);\n}\n";

	return output;
}


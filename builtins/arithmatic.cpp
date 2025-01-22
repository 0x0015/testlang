#include "builtins.hpp"

void addArithmatic(ast::context& context){
	ast::function int_add;
	int_add.name = "add";
	int_add.ty = ast::type::int_type;
	int_add.args = {{ast::type::int_type, "val1"}, {ast::type::int_type, "val2"}};
	int_add.status = ast::function::positionStatus::builtin;
	addBuiltin(context, int_add);
	int_add.name = "sub";
	addBuiltin(context, int_add);
	int_add.name = "mul";
	addBuiltin(context, int_add);
	int_add.name = "div";
	addBuiltin(context, int_add);
	int_add.name = "mod";
	addBuiltin(context, int_add);

	ast::function float_add;
	float_add.name = "add";
	float_add.ty = ast::type::float_type;
	float_add.args = {{ast::type::float_type, "val1"}, {ast::type::float_type, "val2"}};
	float_add.status = ast::function::positionStatus::builtin;
	addBuiltin(context, float_add);
	float_add.name = "sub";
	addBuiltin(context, float_add);
	float_add.name = "mul";
	addBuiltin(context, float_add);
	float_add.name = "div";
	addBuiltin(context, float_add);
}


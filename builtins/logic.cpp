#include "builtins.hpp"

void addLogic(ast::context& context){
	ast::function int_greater;
	int_greater.name = "greater";
	int_greater.ty = ast::type::bool_type;
	int_greater.args = {{ast::type::int_type, "val1"}, {ast::type::int_type, "val2"}};
	int_greater.status = ast::function::positionStatus::builtin;
	addBuiltin(context, int_greater);
	int_greater.name = "less";
	addBuiltin(context, int_greater);
	int_greater.name = "greaterOrEqual";
	addBuiltin(context, int_greater);
	int_greater.name = "lessOrEqual";
	addBuiltin(context, int_greater);
	int_greater.name = "equal";
	addBuiltin(context, int_greater);

	ast::function float_greater;
	float_greater.name = "greater";
	float_greater.ty = ast::type::bool_type;
	float_greater.args = {{ast::type::float_type, "val1"}, {ast::type::float_type, "val2"}};
	float_greater.status = ast::function::positionStatus::builtin;
	addBuiltin(context, float_greater);
	float_greater.name = "less";
	addBuiltin(context, float_greater);
	float_greater.name = "greaterOrEqual";
	addBuiltin(context, float_greater);
	float_greater.name = "lessOrEqual";
	addBuiltin(context, float_greater);
	float_greater.name = "equal";
	addBuiltin(context, float_greater);

	ast::function bool_and;
	bool_and.name = "and";
	bool_and.ty = ast::type::bool_type;
	bool_and.args = {{ast::type::bool_type, "val1"}, {ast::type::bool_type, "val2"}};
	bool_and.status = ast::function::positionStatus::builtin;
	addBuiltin(context, bool_and);
	bool_and.name = "or";
	addBuiltin(context, bool_and);
	bool_and.name = "equal";
	addBuiltin(context, bool_and);

	ast::function bool_not;
	bool_not.name = "not";
	bool_not.ty = ast::type::bool_type;
	bool_not.args = {{ast::type::bool_type, "val"}};
	bool_not.status = ast::function::positionStatus::builtin;
	addBuiltin(context, bool_not);
}

#include "builtins.hpp"

void addAssignments(ast::context& context){
	//assign
	ast::function int_assign;
	int_assign.name = "assign";
	int_assign.ty = ast::type::void_type;
	int_assign.args = {{ast::type::int_type, "dest"}, {ast::type::int_type, "src"}};
	int_assign.builtin = true;
	addBuiltin(context, int_assign);

	ast::function float_assign;
	float_assign.name = "assign";
	float_assign.ty = ast::type::void_type;
	float_assign.args = {{ast::type::float_type, "dest"}, {ast::type::float_type, "src"}};
	float_assign.builtin = true;
	addBuiltin(context, float_assign);

	ast::function bool_assign;
	bool_assign.name = "assign";
	bool_assign.ty = ast::type::void_type;
	bool_assign.args = {{ast::type::bool_type, "dest"}, {ast::type::bool_type, "src"}};
	bool_assign.builtin = true;
	addBuiltin(context, bool_assign);
}

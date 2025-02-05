#include "builtins.hpp"

//Greater, Less, Greater/eq, Less/eq, Equal
void addBasicGLGeLeELogic(const ast::type& ty, ast::context& context){
	ast::function greater;
	greater.name = "greater";
	greater.ty = ast::type::bool_type;
	greater.args = {{ty, "val1"}, {ty, "val2"}};
	greater.status = ast::function::positionStatus::builtin;
	addBuiltin(context, greater);
	greater.name = "less";
	addBuiltin(context, greater);
	greater.name = "greaterOrEqual";
	addBuiltin(context, greater);
	greater.name = "lessOrEqual";
	addBuiltin(context, greater);
	greater.name = "equal";
	addBuiltin(context, greater);
}

void addLogic(ast::context& context){
	addBasicGLGeLeELogic(ast::type::int8_type, context);
	addBasicGLGeLeELogic(ast::type::int16_type, context);
	addBasicGLGeLeELogic(ast::type::int32_type, context);
	addBasicGLGeLeELogic(ast::type::int64_type, context);

	addBasicGLGeLeELogic(ast::type::uint8_type, context);
	addBasicGLGeLeELogic(ast::type::uint16_type, context);
	addBasicGLGeLeELogic(ast::type::uint32_type, context);
	addBasicGLGeLeELogic(ast::type::uint64_type, context);

	addBasicGLGeLeELogic(ast::type::float32_type, context);
	addBasicGLGeLeELogic(ast::type::float64_type, context);

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

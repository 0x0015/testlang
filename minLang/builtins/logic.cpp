#include "builtins.hpp"

//Greater, Less, Greater/eq, Less/eq, Equal
void addBasicGLGeLeELogic(const minLang::ast::type& ty, minLang::ast::context& context){
	minLang::ast::function greater;
	greater.name = "greater";
	greater.ty = minLang::ast::type::bool_type;
	greater.args = {{ty, "val1"}, {ty, "val2"}};
	greater.status = minLang::ast::function::positionStatus::builtin;
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

void addLogic(minLang::ast::context& context){
	addBasicGLGeLeELogic(minLang::ast::type::int8_type, context);
	addBasicGLGeLeELogic(minLang::ast::type::int16_type, context);
	addBasicGLGeLeELogic(minLang::ast::type::int32_type, context);
	addBasicGLGeLeELogic(minLang::ast::type::int64_type, context);

	addBasicGLGeLeELogic(minLang::ast::type::uint8_type, context);
	addBasicGLGeLeELogic(minLang::ast::type::uint16_type, context);
	addBasicGLGeLeELogic(minLang::ast::type::uint32_type, context);
	addBasicGLGeLeELogic(minLang::ast::type::uint64_type, context);

	addBasicGLGeLeELogic(minLang::ast::type::float32_type, context);
	addBasicGLGeLeELogic(minLang::ast::type::float64_type, context);

	minLang::ast::function bool_and;
	bool_and.name = "and";
	bool_and.ty = minLang::ast::type::bool_type;
	bool_and.args = {{minLang::ast::type::bool_type, "val1"}, {minLang::ast::type::bool_type, "val2"}};
	bool_and.status = minLang::ast::function::positionStatus::builtin;
	addBuiltin(context, bool_and);
	bool_and.name = "or";
	addBuiltin(context, bool_and);
	bool_and.name = "equal";
	addBuiltin(context, bool_and);

	minLang::ast::function bool_not;
	bool_not.name = "not";
	bool_not.ty = minLang::ast::type::bool_type;
	bool_not.args = {{minLang::ast::type::bool_type, "val"}};
	bool_not.status = minLang::ast::function::positionStatus::builtin;
	addBuiltin(context, bool_not);
}

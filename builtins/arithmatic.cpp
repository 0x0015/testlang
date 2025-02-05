#include "builtins.hpp"

void addBasicArithmatic(const ast::type& ty, ast::context& context){
	ast::function add;
	add.name = "add";
	add.ty = ty;
	add.args = {{ty, "val1"}, {ty, "val2"}};
	add.status = ast::function::positionStatus::builtin;
	addBuiltin(context, add);
	add.name = "sub";
	addBuiltin(context, add);
	add.name = "mul";
	addBuiltin(context, add);
	add.name = "div";
	addBuiltin(context, add);
	add.name = "mod";
	addBuiltin(context, add);
}

void addArithmatic(ast::context& context){
	addBasicArithmatic(ast::type::int8_type, context);
	addBasicArithmatic(ast::type::int16_type, context);
	addBasicArithmatic(ast::type::int32_type, context);
	addBasicArithmatic(ast::type::int64_type, context);

	addBasicArithmatic(ast::type::uint8_type, context);
	addBasicArithmatic(ast::type::uint16_type, context);
	addBasicArithmatic(ast::type::uint32_type, context);
	addBasicArithmatic(ast::type::uint64_type, context);

	addBasicArithmatic(ast::type::float32_type, context);
	addBasicArithmatic(ast::type::float64_type, context);
}


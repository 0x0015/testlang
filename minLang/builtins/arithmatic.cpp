#include "builtins.hpp"

void addBasicArithmatic(const minLang::ast::type& ty, minLang::ast::context& context){
	minLang::ast::function add;
	add.name = "add";
	add.ty = ty;
	add.args = {{ty, "val1"}, {ty, "val2"}};
	add.status = minLang::ast::function::positionStatus::builtin;
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

void addArithmatic(minLang::ast::context& context){
	addBasicArithmatic(minLang::ast::type::int8_type, context);
	addBasicArithmatic(minLang::ast::type::int16_type, context);
	addBasicArithmatic(minLang::ast::type::int32_type, context);
	addBasicArithmatic(minLang::ast::type::int64_type, context);

	addBasicArithmatic(minLang::ast::type::uint8_type, context);
	addBasicArithmatic(minLang::ast::type::uint16_type, context);
	addBasicArithmatic(minLang::ast::type::uint32_type, context);
	addBasicArithmatic(minLang::ast::type::uint64_type, context);

	addBasicArithmatic(minLang::ast::type::float32_type, context);
	addBasicArithmatic(minLang::ast::type::float64_type, context);
}


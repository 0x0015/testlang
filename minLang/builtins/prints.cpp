#include "builtins.hpp"

void addPrints(minLang::ast::context& context){
	//print
	minLang::ast::function int_print;
	int_print.name = "print";
	int_print.ty = minLang::ast::type::void_type;
	int_print.args = {{minLang::ast::type::int8_type, "val"}};
	int_print.status = minLang::ast::function::positionStatus::builtin;
	addBuiltin(context, int_print);
	int_print.args = {{minLang::ast::type::int16_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{minLang::ast::type::int32_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{minLang::ast::type::int64_type, "val"}};
	addBuiltin(context, int_print);

	int_print.args = {{minLang::ast::type::uint8_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{minLang::ast::type::uint16_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{minLang::ast::type::uint32_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{minLang::ast::type::uint64_type, "val"}};
	addBuiltin(context, int_print);

	minLang::ast::function float_print;
	float_print.name = "print";
	float_print.ty = minLang::ast::type::void_type;
	float_print.args = {{minLang::ast::type::float32_type, "val"}};
	float_print.status = minLang::ast::function::positionStatus::builtin;
	addBuiltin(context, float_print);
	float_print.args = {{minLang::ast::type::float64_type, "val"}};
	addBuiltin(context, float_print);

	minLang::ast::function bool_print;
	bool_print.name = "print";
	bool_print.ty = minLang::ast::type::void_type;
	bool_print.args = {{minLang::ast::type::bool_type, "val"}};
	bool_print.status = minLang::ast::function::positionStatus::builtin;
	addBuiltin(context, bool_print);

	//println
	minLang::ast::function int_println;
	int_println.name = "println";
	int_println.ty = minLang::ast::type::void_type;
	int_println.args = {{minLang::ast::type::int8_type, "val"}};
	int_println.status = minLang::ast::function::positionStatus::builtin;
	addBuiltin(context, int_println);
	int_println.args = {{minLang::ast::type::int16_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{minLang::ast::type::int32_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{minLang::ast::type::int64_type, "val"}};
	addBuiltin(context, int_println);

	int_println.args = {{minLang::ast::type::uint8_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{minLang::ast::type::uint16_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{minLang::ast::type::uint32_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{minLang::ast::type::uint64_type, "val"}};
	addBuiltin(context, int_println);

	minLang::ast::function float_println;
	float_println.name = "println";
	float_println.ty = minLang::ast::type::void_type;
	float_println.args = {{minLang::ast::type::float32_type, "val"}};
	float_println.status = minLang::ast::function::positionStatus::builtin;
	addBuiltin(context, float_println);
	float_println.args = {{minLang::ast::type::float64_type, "val"}};
	addBuiltin(context, float_println);

	minLang::ast::function bool_println;
	bool_println.name = "println";
	bool_println.ty = minLang::ast::type::void_type;
	bool_println.args = {{minLang::ast::type::bool_type, "val"}};
	bool_println.status = minLang::ast::function::positionStatus::builtin;
	addBuiltin(context, bool_println);
}

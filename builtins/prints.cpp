#include "builtins.hpp"

void addPrints(ast::context& context){
	//print
	ast::function int_print;
	int_print.name = "print";
	int_print.ty = ast::type::void_type;
	int_print.args = {{ast::type::int8_type, "val"}};
	int_print.status = ast::function::positionStatus::builtin;
	addBuiltin(context, int_print);
	int_print.args = {{ast::type::int16_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{ast::type::int32_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{ast::type::int64_type, "val"}};
	addBuiltin(context, int_print);

	int_print.args = {{ast::type::uint8_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{ast::type::uint16_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{ast::type::uint32_type, "val"}};
	addBuiltin(context, int_print);
	int_print.args = {{ast::type::uint64_type, "val"}};
	addBuiltin(context, int_print);

	ast::function float_print;
	float_print.name = "print";
	float_print.ty = ast::type::void_type;
	float_print.args = {{ast::type::float32_type, "val"}};
	float_print.status = ast::function::positionStatus::builtin;
	addBuiltin(context, float_print);
	float_print.args = {{ast::type::float64_type, "val"}};
	addBuiltin(context, float_print);

	ast::function bool_print;
	bool_print.name = "print";
	bool_print.ty = ast::type::void_type;
	bool_print.args = {{ast::type::bool_type, "val"}};
	bool_print.status = ast::function::positionStatus::builtin;
	addBuiltin(context, bool_print);

	//println
	ast::function int_println;
	int_println.name = "println";
	int_println.ty = ast::type::void_type;
	int_println.args = {{ast::type::int8_type, "val"}};
	int_println.status = ast::function::positionStatus::builtin;
	addBuiltin(context, int_println);
	int_println.args = {{ast::type::int16_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{ast::type::int32_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{ast::type::int64_type, "val"}};
	addBuiltin(context, int_println);

	int_println.args = {{ast::type::uint8_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{ast::type::uint16_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{ast::type::uint32_type, "val"}};
	addBuiltin(context, int_println);
	int_println.args = {{ast::type::uint64_type, "val"}};
	addBuiltin(context, int_println);

	ast::function float_println;
	float_println.name = "println";
	float_println.ty = ast::type::void_type;
	float_println.args = {{ast::type::float32_type, "val"}};
	float_println.status = ast::function::positionStatus::builtin;
	addBuiltin(context, float_println);
	float_println.args = {{ast::type::float64_type, "val"}};
	addBuiltin(context, float_println);

	ast::function bool_println;
	bool_println.name = "println";
	bool_println.ty = ast::type::void_type;
	bool_println.args = {{ast::type::bool_type, "val"}};
	bool_println.status = ast::function::positionStatus::builtin;
	addBuiltin(context, bool_println);
}

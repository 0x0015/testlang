#include "builtins.hpp"

void addConversion(ast::context& context, const ast::type& from, const ast::type& to){	
	ast::function convert;
	convert.name = "convert";
	convert.ty = to;
	convert.args = {{from, "val"}};
	convert.status = ast::function::positionStatus::builtin;
	addBuiltin(context, convert);
}

void addConversionTwoWay(ast::context& context, const ast::type& t1, const ast::type& t2){
	addConversion(context, t1, t2);
	addConversion(context, t2, t1);
}

void addConversions(ast::context& context){
	//int<->int conversions
	addConversionTwoWay(context, ast::type::int8_type, ast::type::int16_type);
	addConversionTwoWay(context, ast::type::int8_type, ast::type::int32_type);
	addConversionTwoWay(context, ast::type::int8_type, ast::type::int64_type);

	addConversionTwoWay(context, ast::type::int16_type, ast::type::int32_type);
	addConversionTwoWay(context, ast::type::int16_type, ast::type::int64_type);

	addConversionTwoWay(context, ast::type::int32_type, ast::type::int64_type);

	//uint<->uint conversions
	addConversionTwoWay(context, ast::type::uint8_type, ast::type::uint16_type);
	addConversionTwoWay(context, ast::type::uint8_type, ast::type::uint32_type);
	addConversionTwoWay(context, ast::type::uint8_type, ast::type::uint64_type);

	addConversionTwoWay(context, ast::type::uint16_type, ast::type::uint32_type);
	addConversionTwoWay(context, ast::type::uint16_type, ast::type::uint64_type);

	addConversionTwoWay(context, ast::type::uint32_type, ast::type::uint64_type);

	//int<->uint conversions
	addConversionTwoWay(context, ast::type::uint8_type, ast::type::int8_type);
	addConversionTwoWay(context, ast::type::uint8_type, ast::type::int16_type);
	addConversionTwoWay(context, ast::type::uint8_type, ast::type::int32_type);
	addConversionTwoWay(context, ast::type::uint8_type, ast::type::int64_type);

	addConversionTwoWay(context, ast::type::uint16_type, ast::type::int16_type);
	addConversionTwoWay(context, ast::type::uint16_type, ast::type::int32_type);
	addConversionTwoWay(context, ast::type::uint16_type, ast::type::int64_type);

	addConversionTwoWay(context, ast::type::uint32_type, ast::type::int32_type);
	addConversionTwoWay(context, ast::type::uint32_type, ast::type::int64_type);

	addConversionTwoWay(context, ast::type::uint64_type, ast::type::int64_type);

	//int<->float conversions
	addConversionTwoWay(context, ast::type::float32_type, ast::type::int8_type);
	addConversionTwoWay(context, ast::type::float32_type, ast::type::int16_type);
	addConversionTwoWay(context, ast::type::float32_type, ast::type::int32_type);
	addConversionTwoWay(context, ast::type::float32_type, ast::type::int64_type);
	addConversionTwoWay(context, ast::type::float32_type, ast::type::uint8_type);
	addConversionTwoWay(context, ast::type::float32_type, ast::type::uint16_type);
	addConversionTwoWay(context, ast::type::float32_type, ast::type::uint32_type);
	addConversionTwoWay(context, ast::type::float32_type, ast::type::uint64_type);

	addConversionTwoWay(context, ast::type::float64_type, ast::type::int8_type);
	addConversionTwoWay(context, ast::type::float64_type, ast::type::int16_type);
	addConversionTwoWay(context, ast::type::float64_type, ast::type::int32_type);
	addConversionTwoWay(context, ast::type::float64_type, ast::type::int64_type);
	addConversionTwoWay(context, ast::type::float64_type, ast::type::uint8_type);
	addConversionTwoWay(context, ast::type::float64_type, ast::type::uint16_type);
	addConversionTwoWay(context, ast::type::float64_type, ast::type::uint32_type);
	addConversionTwoWay(context, ast::type::float64_type, ast::type::uint64_type);
}


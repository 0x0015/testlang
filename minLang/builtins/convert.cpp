#include "builtins.hpp"

void addConversion(minLang::ast::context& context, const minLang::ast::type& from, const minLang::ast::type& to){	
	minLang::ast::function convert;
	convert.name = "convert";
	convert.ty = to;
	convert.args = {{from, "val"}};
	convert.status = minLang::ast::function::positionStatus::builtin;
	addBuiltin(context, convert);
}

void addConversionTwoWay(minLang::ast::context& context, const minLang::ast::type& t1, const minLang::ast::type& t2){
	addConversion(context, t1, t2);
	addConversion(context, t2, t1);
}

void addConversions(minLang::ast::context& context){
	//int<->int conversions
	addConversionTwoWay(context, minLang::ast::type::int8_type, minLang::ast::type::int16_type);
	addConversionTwoWay(context, minLang::ast::type::int8_type, minLang::ast::type::int32_type);
	addConversionTwoWay(context, minLang::ast::type::int8_type, minLang::ast::type::int64_type);

	addConversionTwoWay(context, minLang::ast::type::int16_type, minLang::ast::type::int32_type);
	addConversionTwoWay(context, minLang::ast::type::int16_type, minLang::ast::type::int64_type);

	addConversionTwoWay(context, minLang::ast::type::int32_type, minLang::ast::type::int64_type);

	//uint<->uint conversions
	addConversionTwoWay(context, minLang::ast::type::uint8_type, minLang::ast::type::uint16_type);
	addConversionTwoWay(context, minLang::ast::type::uint8_type, minLang::ast::type::uint32_type);
	addConversionTwoWay(context, minLang::ast::type::uint8_type, minLang::ast::type::uint64_type);

	addConversionTwoWay(context, minLang::ast::type::uint16_type, minLang::ast::type::uint32_type);
	addConversionTwoWay(context, minLang::ast::type::uint16_type, minLang::ast::type::uint64_type);

	addConversionTwoWay(context, minLang::ast::type::uint32_type, minLang::ast::type::uint64_type);

	//int<->uint conversions
	addConversionTwoWay(context, minLang::ast::type::uint8_type, minLang::ast::type::int8_type);
	addConversionTwoWay(context, minLang::ast::type::uint8_type, minLang::ast::type::int16_type);
	addConversionTwoWay(context, minLang::ast::type::uint8_type, minLang::ast::type::int32_type);
	addConversionTwoWay(context, minLang::ast::type::uint8_type, minLang::ast::type::int64_type);

	addConversionTwoWay(context, minLang::ast::type::uint16_type, minLang::ast::type::int16_type);
	addConversionTwoWay(context, minLang::ast::type::uint16_type, minLang::ast::type::int32_type);
	addConversionTwoWay(context, minLang::ast::type::uint16_type, minLang::ast::type::int64_type);

	addConversionTwoWay(context, minLang::ast::type::uint32_type, minLang::ast::type::int32_type);
	addConversionTwoWay(context, minLang::ast::type::uint32_type, minLang::ast::type::int64_type);

	addConversionTwoWay(context, minLang::ast::type::uint64_type, minLang::ast::type::int64_type);

	//int<->float conversions
	addConversionTwoWay(context, minLang::ast::type::float32_type, minLang::ast::type::int8_type);
	addConversionTwoWay(context, minLang::ast::type::float32_type, minLang::ast::type::int16_type);
	addConversionTwoWay(context, minLang::ast::type::float32_type, minLang::ast::type::int32_type);
	addConversionTwoWay(context, minLang::ast::type::float32_type, minLang::ast::type::int64_type);
	addConversionTwoWay(context, minLang::ast::type::float32_type, minLang::ast::type::uint8_type);
	addConversionTwoWay(context, minLang::ast::type::float32_type, minLang::ast::type::uint16_type);
	addConversionTwoWay(context, minLang::ast::type::float32_type, minLang::ast::type::uint32_type);
	addConversionTwoWay(context, minLang::ast::type::float32_type, minLang::ast::type::uint64_type);

	addConversionTwoWay(context, minLang::ast::type::float64_type, minLang::ast::type::int8_type);
	addConversionTwoWay(context, minLang::ast::type::float64_type, minLang::ast::type::int16_type);
	addConversionTwoWay(context, minLang::ast::type::float64_type, minLang::ast::type::int32_type);
	addConversionTwoWay(context, minLang::ast::type::float64_type, minLang::ast::type::int64_type);
	addConversionTwoWay(context, minLang::ast::type::float64_type, minLang::ast::type::uint8_type);
	addConversionTwoWay(context, minLang::ast::type::float64_type, minLang::ast::type::uint16_type);
	addConversionTwoWay(context, minLang::ast::type::float64_type, minLang::ast::type::uint32_type);
	addConversionTwoWay(context, minLang::ast::type::float64_type, minLang::ast::type::uint64_type);
}


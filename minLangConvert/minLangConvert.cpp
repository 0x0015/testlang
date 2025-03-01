#include "minLangConvert.hpp"

std::optional<minLang::ast::context> minLangConvert(const ast::context& context){
	minLang::ast::context output;
	for(const auto& func : context.funcs){
		const auto& convertedFunc = minLangConvertFunction(func);
		if(!convertedFunc)
			return std::nullopt;
		output.funcs.push_back(*convertedFunc);
	}

	return output;
}

std::optional<minLang::ast::function> minLangConvertFunction(const ast::function& func){
	minLang::ast::function output;
	const auto& convertedType = minLangConvertType(func.ty);
	if(!convertedType)
		return std::nullopt;
	output.ty = *convertedType;
	output.name = func.name;
	switch(func.status){
		case ast::function::normal:
			output.status = minLang::ast::function::normal;
		case ast::function::builtin:
			output.status = minLang::ast::function::builtin;
		case ast::function::external:
			output.status = minLang::ast::function::external;
	}
	const auto& convertedBody = minLangConvertBlock(func.body);
	if(!convertedBody)
		return std::nullopt;
	output.body = *convertedBody;
	for(const auto& arg : func.args){
		const auto& convertedArgTy = minLangConvertType(arg.ty);
		if(!convertedArgTy)
			return std::nullopt;
		output.args.push_back(minLang::ast::function::argument{*convertedArgTy, arg.name});
	}

	return output;
}

std::optional<minLang::ast::type> minLangConvertType(const ast::type& ty){
	minLang::ast::type output;
	if(std::holds_alternative<ast::type::builtin_type>(ty.ty)){
		const auto& builtinTy = std::get<ast::type::builtin_type>(ty.ty);
		switch(builtinTy){
			case ast::type::void_type:
				return minLang::ast::type{minLang::ast::type::void_type};
			case ast::type::uint8_type:
				return minLang::ast::type{minLang::ast::type::uint8_type};
			case ast::type::uint16_type:
				return minLang::ast::type{minLang::ast::type::uint16_type};
			case ast::type::uint32_type:
				return minLang::ast::type{minLang::ast::type::uint32_type};
			case ast::type::uint64_type:
				return minLang::ast::type{minLang::ast::type::uint64_type};
			case ast::type::int8_type:
				return minLang::ast::type{minLang::ast::type::int8_type};
			case ast::type::int16_type:
				return minLang::ast::type{minLang::ast::type::int16_type};
			case ast::type::int32_type:
				return minLang::ast::type{minLang::ast::type::int32_type};
			case ast::type::int64_type:
				return minLang::ast::type{minLang::ast::type::int64_type};
			case ast::type::float32_type:
				return minLang::ast::type{minLang::ast::type::float32_type};
			case ast::type::float64_type:
				return minLang::ast::type{minLang::ast::type::float64_type};
			case ast::type::bool_type:
				return minLang::ast::type{minLang::ast::type::bool_type};
			default:
				return std::nullopt;
		}
	}else if(std::holds_alternative<ast::type::array_type>(ty.ty)){
		const auto& arrayTy = std::get<ast::type::array_type>(ty.ty);
		const auto& derivedTy = minLangConvertType(*arrayTy.ty);
		if(!derivedTy)
			return std::nullopt;
		return minLang::ast::type::array_type{*derivedTy, arrayTy.length};
	}
	return std::nullopt;
}

std::optional<minLang::ast::block> minLangConvertBlock(const ast::block& block){
	return std::nullopt;
}


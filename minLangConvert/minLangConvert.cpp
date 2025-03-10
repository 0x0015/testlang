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
				std::cerr<<"Error: During minLang conversion found unknown basic type"<<std::endl;
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
	minLang::ast::block output;

	for(const auto& statement : block.statements){
		if(std::holds_alternative<ast::block::declaration>(statement)){
			const auto& decl = std::get<ast::block::declaration>(statement);
			const auto& convTy = minLangConvertType(decl.ty);
			if(!convTy)
				return std::nullopt;
			output.statements.push_back(minLang::ast::block::declaration{*convTy, decl.name});
		}else if(std::holds_alternative<ast::block::assignment>(statement)){
			const auto& asgn = std::get<ast::block::assignment>(statement);
			const auto& convExp = minLangConvertExpr(asgn.assignFrom);
			if(!convExp)
				return std::nullopt;
			output.statements.push_back(minLang::ast::block::assignment{asgn.assignTo, *convExp});
		}else if(std::holds_alternative<ast::block::ifStatement>(statement)){
			const auto& ifStat = std::get<ast::block::ifStatement>(statement);
			const auto& ifBlockConv = minLangConvertBlock(*ifStat.ifBody);
			if(!ifBlockConv)
				return std::nullopt;
			const auto& elseBlockConv = minLangConvertBlock(*ifStat.elseBody);
			if(!elseBlockConv)
				return std::nullopt;
			const auto& condConv = minLangConvertExpr(ifStat.condition);
			if(!condConv)
				return std::nullopt;
			output.statements.push_back(minLang::ast::block::ifStatement{*condConv, std::make_shared<minLang::ast::block>(*ifBlockConv), std::make_shared<minLang::ast::block>(*elseBlockConv)});
		}else if(std::holds_alternative<ast::block::forStatement_normal>(statement)){
			const auto& forStat = std::get<ast::block::forStatement_normal>(statement);
			std::cerr<<"Error: not implementing this right now.  I can't think welll enough to wade through it (shouldn't be too hard)"<<std::endl;
			return std::nullopt;
		}else if(std::holds_alternative<ast::block::forStatement_while>(statement)){
			const auto& whileStat = std::get<ast::block::forStatement_while>(statement);
			const auto& bodyConv = minLangConvertBlock(*whileStat.body);
			if(!bodyConv)
				return std::nullopt;
			const auto& condConv = minLangConvertExpr(whileStat.condition);
			if(!condConv)
				return std::nullopt;
			output.statements.push_back(minLang::ast::block::forStatement_while{*condConv, std::make_shared<minLang::ast::block>(*bodyConv)});
		}else if(std::holds_alternative<ast::block::returnStatement>(statement)){
			const auto& ret = std::get<ast::block::returnStatement>(statement);
			const auto& valConv = minLangConvertExpr(ret.val);
			if(!valConv)
				return std::nullopt;
			output.statements.push_back(minLang::ast::block::returnStatement{*valConv});
		}else if(std::holds_alternative<ast::expr>(statement)){
			const auto& exp = std::get<ast::expr>(statement);
			const auto& convExp = minLangConvertExpr(exp);
			if(!convExp)
				return std::nullopt;
			output.statements.push_back(*convExp);
		}else{
			std::cerr<<"Error: During minLang conversion, encountered block statement of unknown type"<<std::endl;
			return std::nullopt;
		}
	}

	return output;
}

std::optional<minLang::ast::expr> minLangConvertExpr(const ast::expr& exp){
	return std::nullopt;
}


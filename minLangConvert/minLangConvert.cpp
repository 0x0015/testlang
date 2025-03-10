#include "minLangConvert.hpp"

void fixValidatedDefsExp(minLang::ast::expr& exp, const toMinLangFuncMap& funcMap){
	if(std::holds_alternative<minLang::ast::call>(exp.value)){
		auto& call = std::get<minLang::ast::call>(exp.value);
		auto* defPtr = &call.validatedDef;
		auto* badPtr = (const ast::call**)defPtr; //ULTRA FORBIDDEN MEMORY HACK!!!!
		auto funcFrom = *(*badPtr)->validatedDef;
		auto mapTo = funcMap.at(funcFrom);
		call.validatedDef = mapTo;
	}
}

void fixValidatedDefsBlock(minLang::ast::block& block, const toMinLangFuncMap& funcMap){
	for(auto& statement : block.statements){
		if(std::holds_alternative<minLang::ast::block::assignment>(statement)){
			auto& asgn = std::get<minLang::ast::block::assignment>(statement);
			fixValidatedDefsExp(asgn.assignFrom, funcMap);
		}else if(std::holds_alternative<minLang::ast::block::ifStatement>(statement)){
			auto& ifStat = std::get<minLang::ast::block::ifStatement>(statement);
			fixValidatedDefsBlock(*ifStat.ifBody, funcMap);
			fixValidatedDefsBlock(*ifStat.elseBody, funcMap);
			fixValidatedDefsExp(ifStat.condition, funcMap);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_normal>(statement)){
			auto& forStat = std::get<minLang::ast::block::forStatement_normal>(statement);
			fixValidatedDefsBlock(*forStat.body, funcMap);
			fixValidatedDefsExp(forStat.breakCond, funcMap);
			fixValidatedDefsExp(forStat.initialDecl.assignFrom, funcMap);
			fixValidatedDefsExp(forStat.perLoopAsgn.assignFrom, funcMap);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_while>(statement)){
			auto& whileStat = std::get<minLang::ast::block::forStatement_while>(statement);
			fixValidatedDefsBlock(*whileStat.body, funcMap);
			fixValidatedDefsExp(whileStat.condition, funcMap);
		}else if(std::holds_alternative<minLang::ast::block::returnStatement>(statement)){
			auto& ret = std::get<minLang::ast::block::returnStatement>(statement);
			fixValidatedDefsExp(ret.val, funcMap);
		}else if(std::holds_alternative<minLang::ast::expr>(statement)){
			auto& exp = std::get<minLang::ast::expr>(statement);
			fixValidatedDefsExp(exp, funcMap);
		}
	}
}

void fixValidatedDefsFunc(minLang::ast::function& func, const toMinLangFuncMap& funcMap){
	fixValidatedDefsBlock(func.body, funcMap);
}

std::optional<minLang::ast::context> minLangConvert(const ast::context& context){
	minLang::ast::context output;
	toMinLangFuncMap funcMap;
	for(const auto& func : context.funcs){
		const auto& convertedFunc = minLangConvertFunction(func);
		if(!convertedFunc)
			return std::nullopt;
		output.funcs.push_back(*convertedFunc);
		funcMap.insert(std::make_pair<std::reference_wrapper<const ast::function>, std::reference_wrapper<const minLang::ast::function>>(std::cref(func), std::cref(output.funcs.back())));
	}

	//now loop over all exprs and fix validatedDefs
	for(auto& func : output.funcs)
		fixValidatedDefsFunc(func, funcMap);

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
			break;
		case ast::function::builtin:
			output.status = minLang::ast::function::builtin;
			break;
		case ast::function::external:
			output.status = minLang::ast::function::external;
			break;
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
	std::cerr<<"Error: During minLang conversion found unsupported type "<<ty.toString()<<std::endl;
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
			const auto& bodyConv = minLangConvertBlock(*forStat.body);
			if(!bodyConv)
				return std::nullopt;
			const auto& condConv = minLangConvertExpr(forStat.breakCond);
			if(!condConv)
				return std::nullopt;
			const auto& initialFromConv = minLangConvertExpr(forStat.initialDecl.assignFrom);
			if(!initialFromConv)
				return std::nullopt;
			const auto& perLoopFromConv = minLangConvertExpr(forStat.perLoopAsgn.assignFrom);
			if(!perLoopFromConv)
				return std::nullopt;
			output.statements.push_back(minLang::ast::block::forStatement_normal{minLang::ast::block::assignment{forStat.initialDecl.assignTo, *initialFromConv}, *condConv, minLang::ast::block::assignment{forStat.perLoopAsgn.assignTo, *perLoopFromConv}, std::make_shared<minLang::ast::block>(*bodyConv)});
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

std::optional<minLang::ast::literal> minLangConvertLiteral(const ast::literal& lit){
	const auto& convTy = minLangConvertType(lit.ty);
	if(!convTy)
		return std::nullopt;
	minLang::ast::literal output;
	output.ty = *convTy;
	if(std::holds_alternative<ast::literal::builtin_literal>(lit.value)){
		const auto& builtin = std::get<ast::literal::builtin_literal>(lit.value);
		std::visit([&](const auto& val){
			output.value = minLang::ast::literal::builtin_literal{val};
			}, builtin);
	}else if(std::holds_alternative<ast::literal::array_literal>(lit.value)){
		const auto& arr = std::get<ast::literal::array_literal>(lit.value);
		std::vector<minLang::ast::literal> convArr(arr.vals.size());
		for(unsigned int i=0;i<arr.vals.size();i++){
			const auto& convArrVal = minLangConvertLiteral(arr.vals[i]);
			if(!convArrVal)
				return std::nullopt;
			convArr[i] = *convArrVal;
		}
		output.value = minLang::ast::literal::array_literal{convArr};
	}else if(std::holds_alternative<ast::literal::tuple_literal>(lit.value)){
		std::cerr<<"Error: During minLang conversion, encountered tuple type literal (currently unsupported)"<<std::endl;
		return std::nullopt;
	}else{
		std::cerr<<"Error: During minLang conversion, encountered literal value of unknown type"<<std::endl;
		return std::nullopt;
	}
	return output;
}

std::optional<minLang::ast::expr> minLangConvertExpr(const ast::expr& exp){
	if(std::holds_alternative<ast::literal>(exp.value)){
		const auto& lit = std::get<ast::literal>(exp.value);
		const auto& convLit = minLangConvertLiteral(lit);
		if(!convLit)
			return std::nullopt;
		return *convLit;
	}else if(std::holds_alternative<ast::call>(exp.value)){
		const auto& call = std::get<ast::call>(exp.value);
		minLang::ast::call output{call.name};
		for(const auto& arg : call.args){
			const auto& argConv = minLangConvertExpr(arg);
			if(!argConv)
				return std::nullopt;
			output.args.push_back(*argConv);
		}
		//HOW TO DEAL WITH????
		if(call.validatedDef){
			output.validatedDef = {};
			auto* defPtr = &output.validatedDef;
			auto* badPtr = (const ast::call**)defPtr; //ULTRA FORBIDDEN MEMORY HACK!!!!
			*badPtr = &call;
			//save for later
			//THIS IS VERY BAD CODE
			//LIKE ACTUALLY VERY VERY BAD
		}
		return output;
	}else if(std::holds_alternative<ast::varName>(exp.value)){
		const auto& varNam = std::get<ast::varName>(exp.value);
		if(varNam.matchedType){
			const auto& tyConv = minLangConvertType(*varNam.matchedType);
			if(!tyConv)
				return std::nullopt;
			minLang::ast::expr output;
			output.value = minLang::ast::varName{varNam.name, *tyConv};
			return output;
		}
		minLang::ast::expr output;
		output.value = minLang::ast::varName{varNam.name, std::nullopt};
		return output;
	}
	std::cerr<<"Error: During minLang conversion, encountered expr of unknown type"<<std::endl;
	return std::nullopt;
}


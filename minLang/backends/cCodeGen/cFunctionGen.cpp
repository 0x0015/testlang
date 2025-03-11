#include "cCodeGen.hpp"

std::string genUsedFunctionForwarddef(const minLang::ast::function& func, const std::unordered_map<minLang::ast::type, minLang::backends::cCodeGen::cTypeInfo, minLang::backends::typeHasher>& types){
	std::string output;
	output += types.at(func.ty).cName;
	if(func.status == minLang::ast::function::positionStatus::normal){
		output += " " + minLang::backends::cCodeGen::mangleFuncName(func) + "(";
	}else if(func.status == minLang::ast::function::positionStatus::builtin){
		if(minLang::backends::cCodeGen::isSpecificBuiltinFuncCase(func))
			return "";
		output += " " + minLang::backends::cCodeGen::mangleFuncName(func) + "("; //if it's a specific case, custom code will be generated so skip over it
	}else if(func.status == minLang::ast::function::positionStatus::external){
		output += " " + func.name + "(";
	}

	for(unsigned int i=0;i<func.args.size();i++){
		output += types.at(func.args[i].ty).cName + " " + minLang::backends::cCodeGen::mangleName(func.args[i].name);
		if(i+1 < func.args.size())
			output += ", ";
	}
	output += ");";
	if(func.status == minLang::ast::function::positionStatus::external){
		output += "\n#define " + minLang::backends::cCodeGen::mangleFuncName(func) + " " + func.name;
	}
	return output;
}

std::string minLang::backends::cCodeGen::genUsedFunctionForwarddefs(const std::unordered_set<std::reference_wrapper<const minLang::ast::function>, funcSigHasher, funcSigComp>& funcs, const std::unordered_map<minLang::ast::type, cTypeInfo, typeHasher>& types){
	std::string output;
	for(const auto& func : funcs){
		output += genUsedFunctionForwarddef(func.get(), types);
		output += '\n';
	}
	return output;
}

std::string literalToCLit(const minLang::ast::literal& lit){
	if(std::holds_alternative<minLang::ast::literal::builtin_literal>(lit.value)){
		const auto& builtin = std::get<minLang::ast::literal::builtin_literal>(lit.value);
		if(builtin.valueless_by_exception()){
			std::cerr<<"Error: unable to cCodeGen literal (empty)"<<std::endl;
			return "<error literal>";
		}
		return std::visit([](const auto& val)->std::string{
			using builtinTy = std::decay_t<decltype(val)>;
			if constexpr(std::is_same_v<bool, builtinTy>){	
				return val ? "1" : "0";
			}else if constexpr(std::is_same_v<float, builtinTy>){
				return std::to_string(val) + "f";
			}else{
				return std::to_string(val);
			}
		}, builtin);
	}else if(std::holds_alternative<minLang::ast::literal::array_literal>(lit.value)){
		const auto& array = std::get<minLang::ast::literal::array_literal>(lit.value);
		std::string output = "{";
		for(unsigned int i=0;i<array.vals.size();i++){
			output += literalToCLit(array.vals[i]);
			if(i+1 < array.vals.size())
				output += ", ";
		}
		output += "}";
		return output;
	}else if(std::holds_alternative<minLang::ast::literal::tuple_literal>(lit.value)){
		const auto& tup = std::get<minLang::ast::literal::tuple_literal>(lit.value);
		std::string output = "{";
		for(unsigned int i=0;i<tup.vals.size();i++){
			output += literalToCLit(tup.vals[i]);
			if(i+1 < tup.vals.size())
				output += ", ";
		}
		output += "}";
		return output;
	}else{
		std::cerr<<"Error: unable to cCodeGen literal (empty)"<<std::endl;
		return "<error literal>";
	}
}

std::string genExprDef(const minLang::ast::expr& expr, const std::unordered_map<minLang::ast::type, minLang::backends::cCodeGen::cTypeInfo, minLang::backends::typeHasher>& types){
	if(std::holds_alternative<minLang::ast::literal>(expr.value)){
		const auto& lit = std::get<minLang::ast::literal>(expr.value);
		return literalToCLit(lit);
	}else if(std::holds_alternative<minLang::ast::call>(expr.value)){
		const auto& call = std::get<minLang::ast::call>(expr.value);
		if(call.validatedDef->get().status == minLang::ast::function::builtin && minLang::backends::cCodeGen::isSpecificBuiltinFuncCase(*call.validatedDef)){
			std::vector<std::string> generatedArgs(call.args.size());
			for(unsigned int i=0;i<call.args.size();i++)
				generatedArgs[i] = genExprDef(call.args[i], types);
			return minLang::backends::cCodeGen::genSpecificBuiltinFuncCase(call, generatedArgs);
		}else{
			std::string output = minLang::backends::cCodeGen::mangleFuncName(call.validatedDef->get()) + "(";
			for(unsigned int i=0;i<call.args.size();i++){
				output += genExprDef(call.args[i], types);
				if(i+1 < call.args.size())
					output += ", ";
			}
			output += ")";
			return output;
		}
	}else if(std::holds_alternative<minLang::ast::varName>(expr.value)){
		const auto& varName = std::get<minLang::ast::varName>(expr.value);
		return minLang::backends::cCodeGen::mangleName(varName.name);
	}else{
		std::cerr<<"Error: unable to cCodeGen expr : ";expr.dump();
		return "";
	}
}

std::string genBlockDef(const minLang::ast::block& block, const std::unordered_map<minLang::ast::type, minLang::backends::cCodeGen::cTypeInfo, minLang::backends::typeHasher>& types){
	std::string output = "{\n";
	for(const auto& statement : block.statements){
		if(std::holds_alternative<minLang::ast::block::declaration>(statement)){
			const auto& decl = std::get<minLang::ast::block::declaration>(statement);
			output += "\t" + types.at(decl.ty).cName + " " + minLang::backends::cCodeGen::mangleName(decl.name) + " = " + minLang::backends::cCodeGen::getDefaultTypeValue(decl.ty, types) + ";\n";
		}else if(std::holds_alternative<minLang::ast::block::assignment>(statement)){
			const auto& asgn = std::get<minLang::ast::block::assignment>(statement);
			output += "\t" + minLang::backends::cCodeGen::mangleName(asgn.assignTo) + " = " + genExprDef(asgn.assignFrom, types) + ";\n";
		}else if(std::holds_alternative<minLang::ast::block::ifStatement>(statement)){
			const auto& ifStat = std::get<minLang::ast::block::ifStatement>(statement);
			output += "\tif(" + genExprDef(ifStat.condition, types) + ")";
			output += genBlockDef(*ifStat.ifBody, types) + "else" + genBlockDef(*ifStat.elseBody, types);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_while>(statement)){
			const auto& whileStat = std::get<minLang::ast::block::forStatement_while>(statement);
			output += "\twhile(" + genExprDef(whileStat.condition, types) + ")";
			output += genBlockDef(*whileStat.body, types);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_normal>(statement)){
			const auto& forStat = std::get<minLang::ast::block::forStatement_normal>(statement);
			output += "\tfor(";
			output += types.at(*forStat.initialDecl.assignFrom.inferType()).cName + " " + minLang::backends::cCodeGen::mangleName(forStat.initialDecl.assignTo) + " = " + genExprDef(forStat.initialDecl.assignFrom, types) + ";";
			output += genExprDef(forStat.breakCond, types) + ";";
			output += minLang::backends::cCodeGen::mangleName(forStat.perLoopAsgn.assignTo) + " = " + genExprDef(forStat.perLoopAsgn.assignFrom, types) + ")";
			output += genBlockDef(*forStat.body, types);
		}else if(std::holds_alternative<minLang::ast::block::returnStatement>(statement)){
			const auto& retStat = std::get<minLang::ast::block::returnStatement>(statement);
			output += "return(" + genExprDef(retStat.val, types) + ");\n";
		}else if(std::holds_alternative<minLang::ast::expr>(statement)){
			output += "\t" + genExprDef(std::get<minLang::ast::expr>(statement), types) + ";\n";
		}else{
			std::cerr<<"cCodeGen Error: block statement of unknown type!"<<std::endl;
		}
	}
	output += "}";
	return output;
}

std::string genFunctionDef(const minLang::ast::function& func, const std::unordered_map<minLang::ast::type, minLang::backends::cCodeGen::cTypeInfo, minLang::backends::typeHasher>& types){
	if(func.status != minLang::ast::function::positionStatus::normal){
		//std::cout<<"ignoring function with non-normal status: "<<func.name<<std::endl;
		//no need to warn, as this is expected (as builtin and external functions should well not have a body generated for them)
		return {};
	}
	std::string output;
	output += types.at(func.ty).cName;
	output += " " + minLang::backends::cCodeGen::mangleFuncName(func) + "(";

	for(unsigned int i=0;i<func.args.size();i++){
		output += types.at(func.args[i].ty).cName + " " + minLang::backends::cCodeGen::mangleName(func.args[i].name);
		if(i+1 < func.args.size())
			output += ", ";
	}
	output += ")";
	output += genBlockDef(func.body, types);
	return output;
}

std::string minLang::backends::cCodeGen::genUsedFunctionDefs(const std::unordered_set<std::reference_wrapper<const minLang::ast::function>, funcSigHasher, funcSigComp>& funcs, const std::unordered_map<minLang::ast::type, cTypeInfo, typeHasher>& types){
	std::string output;
	for(const auto& func : funcs){
		output += genFunctionDef(func.get(), types);
		output += '\n';
	}
	return output;
}



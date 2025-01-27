#include "cCodeGen.hpp"

std::string genUsedFunctionForwarddef(const ast::function& func, const std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher>& types){
	std::string output;
	output += types.at(func.ty).cName;
	if(func.status == ast::function::positionStatus::normal || func.status == ast::function::positionStatus::builtin){
		output += " " + cCodeGen::mangleFuncName(func) + "(";
	}else if(func.status == ast::function::positionStatus::external){
		output += " " + func.name + "(";
	}

	for(unsigned int i=0;i<func.args.size();i++){
		output += types.at(func.args[i].ty).cName + " " + cCodeGen::mangleName(func.args[i].name);
		if(i+1 < func.args.size())
			output += ", ";
	}
	output += ");";
	if(func.status == ast::function::positionStatus::external){
		output += "\n#define " + cCodeGen::mangleFuncName(func) + " " + func.name;
	}
	return output;
}

std::string cCodeGen::genUsedFunctionForwarddefs(const std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types){
	std::string output;
	for(const auto& [name, func] : funcs){
		output += genUsedFunctionForwarddef(func.get(), types);
		output += '\n';
	}
	return output;
}

std::string literalToCLit(const ast::literal& lit){
	if(std::holds_alternative<ast::literal::builtin_literal>(lit.value)){
		const auto& builtin = std::get<ast::literal::builtin_literal>(lit.value);
		if(std::holds_alternative<int>(builtin)){
			return std::to_string(std::get<int>(builtin));
		}else if(std::holds_alternative<float>(builtin)){
			return std::to_string(std::get<float>(builtin));
		}else if(std::holds_alternative<bool>(builtin)){
			return std::get<bool>(builtin) ? "1" : "0";
		}else{
			std::cerr<<"Error: unable to cCodeGen literal (empty)"<<std::endl;
			return "<error literal>";
		}
	}else if(std::holds_alternative<ast::literal::array_literal>(lit.value)){
		const auto& array = std::get<ast::literal::array_literal>(lit.value);
		std::string output = "{";
		for(unsigned int i=0;i<array.vals.size();i++){
			output += literalToCLit(array.vals[i]);
			if(i+1 < array.vals.size())
				output += ", ";
		}
		output += "}";
		return output;
	}else if(std::holds_alternative<ast::literal::tuple_literal>(lit.value)){
		const auto& tup = std::get<ast::literal::tuple_literal>(lit.value);
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

std::string genExprDef(const ast::expr& expr, const std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher>& types){
	if(std::holds_alternative<ast::literal>(expr.value)){
		const auto& lit = std::get<ast::literal>(expr.value);
		return literalToCLit(lit);
	}else if(std::holds_alternative<ast::call>(expr.value)){
		const auto& call = std::get<ast::call>(expr.value);
		std::string output = cCodeGen::mangleFuncName(call.validatedDef->get()) + "(";
		for(unsigned int i=0;i<call.args.size();i++){
			output += genExprDef(call.args[i], types);
			if(i+1 < call.args.size())
				output += ", ";
		}
		output += ")";
		return output;
	}else if(std::holds_alternative<ast::varName>(expr.value)){
		const auto& varName = std::get<ast::varName>(expr.value);
		return cCodeGen::mangleName(varName.name);
	}else{
		std::cerr<<"Error: unable to cCodeGen expr : ";expr.dump();
		return "";
	}
}

std::string genBlockDef(const ast::block& block, const std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher>& types){
	std::string output = "{\n";
	for(const auto& statement : block.statements){
		if(std::holds_alternative<ast::block::declaration>(statement)){
			const auto& decl = std::get<ast::block::declaration>(statement);
			output += "\t" + types.at(decl.ty).cName + " " + cCodeGen::mangleName(decl.name) + ";\n";
		}else if(std::holds_alternative<ast::block::assignment>(statement)){
			const auto& asgn = std::get<ast::block::assignment>(statement);
			output += "\t" + cCodeGen::mangleName(asgn.assignTo) + " = " + genExprDef(asgn.assignFrom, types) + ";\n";
		}else if(std::holds_alternative<ast::block::ifStatement>(statement)){
			const auto& ifStat = std::get<ast::block::ifStatement>(statement);
			output += "\tif(" + genExprDef(ifStat.condition, types) + ")";
			output += genBlockDef(*ifStat.ifBody, types) + "else" + genBlockDef(*ifStat.elseBody, types);
		}else if(std::holds_alternative<ast::block::forStatement_while>(statement)){
			const auto& whileStat = std::get<ast::block::forStatement_while>(statement);
			output += "\twhile(" + genExprDef(whileStat.condition, types) + ")";
			output += genBlockDef(*whileStat.body, types);
		}else if(std::holds_alternative<ast::block::forStatement_normal>(statement)){
			const auto& forStat = std::get<ast::block::forStatement_normal>(statement);
			output += "\tfor(";
			output += types.at(*forStat.initialDecl.assignFrom.inferType()).cName + " " + cCodeGen::mangleName(forStat.initialDecl.assignTo) + " = " + genExprDef(forStat.initialDecl.assignFrom, types) + ";";
			output += genExprDef(forStat.breakCond, types) + ";";
			output += cCodeGen::mangleName(forStat.perLoopAsgn.assignTo) + " = " + genExprDef(forStat.perLoopAsgn.assignFrom, types) + ")";
			output += genBlockDef(*forStat.body, types);
		}else if(std::holds_alternative<ast::block::returnStatement>(statement)){
			const auto& retStat = std::get<ast::block::returnStatement>(statement);
			output += "return(" + genExprDef(retStat.val, types) + ");\n";
		}else if(std::holds_alternative<ast::expr>(statement)){
			output += "\t" + genExprDef(std::get<ast::expr>(statement), types) + ";\n";
		}else{
			std::cerr<<"cCodeGen Error: block statement of unknown type!"<<std::endl;
		}
	}
	output += "}";
	return output;
}

std::string genFunctionDef(const ast::function& func, const std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher>& types){
	if(func.status != ast::function::positionStatus::normal){
		//std::cout<<"ignoring function with non-normal status: "<<func.name<<std::endl;
		//no need to warn, as this is expected (as builtin and external functions should well not have a body generated for them)
		return {};
	}
	std::string output;
	output += types.at(func.ty).cName;
	output += " " + cCodeGen::mangleFuncName(func) + "(";

	for(unsigned int i=0;i<func.args.size();i++){
		output += types.at(func.args[i].ty).cName + " " + cCodeGen::mangleName(func.args[i].name);
		if(i+1 < func.args.size())
			output += ", ";
	}
	output += ")";
	output += genBlockDef(func.body, types);
	return output;
}

std::string cCodeGen::genUsedFunctionDefs(const std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types){
	std::string output;
	for(const auto& [name, func] : funcs){
		output += genFunctionDef(func.get(), types);
		output += '\n';
	}
	return output;
}



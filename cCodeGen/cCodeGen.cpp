#include "cCodeGen.hpp"
#include <cstring>

bool cCodeGen::genCCode(const ast::context& context, const std::string_view entryPoint, bool autoBuild, std::span<const std::string> linkLibs){
	std::optional<std::reference_wrapper<const ast::function>> entry;
	for(const auto& func : context.funcs){
		if(func.name == entryPoint && func.ty == ast::type::void_type && func.args.size() == 0){
			entry = std::cref(func);
			break;
		}
	}
	if(!entry){
		std::cerr<<"Error: Unable to find entry point function \""<<entryPoint<<"\" (must be void->void)"<<std::endl;
		return false;
	}
}

void findUsedFunctions_iter_addExpr(const ast::expr& expr, std::unordered_map<std::string, std::reference_wrapper<const ast::function>>& foundFuncs){
	if(std::holds_alternative<ast::call>(expr.value)){
		const auto& call = std::get<ast::call>(expr.value);
		if(!foundFuncs.contains(call.name))
			foundFuncs.emplace(std::make_pair(call.name, *call.validatedDef));
		for(const auto& arg : call.args)
			findUsedFunctions_iter_addExpr(arg, foundFuncs);
	}
}

void findUsedFunctions_iter(std::reference_wrapper<const ast::function> funcToSearch, std::unordered_map<std::string, std::reference_wrapper<const ast::function>>& foundFuncs){
	foundFuncs.emplace(std::make_pair(funcToSearch.get().name, funcToSearch));
	for(const auto& state : funcToSearch.get().body.statements){
		if(std::holds_alternative<ast::expr>(state)){
			const auto& expr = std::get<ast::expr>(state);
			findUsedFunctions_iter_addExpr(expr, foundFuncs);
		}
	}
}

std::unordered_map<std::string, std::reference_wrapper<const ast::function>> cCodeGen::findUsedFunctions(std::reference_wrapper<const ast::function> entrypoint){
	std::unordered_map<std::string, std::reference_wrapper<const ast::function>> output;
	findUsedFunctions_iter(entrypoint, output);
	return output;
}

void findUsedTypes_iter_addExpr(const ast::expr& expr, std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher>& foundTypes){
	if(std::holds_alternative<ast::literal>(expr.value)){
		const auto& lit = std::get<ast::literal>(expr.value);
		foundTypes[lit.ty] = {};
	}else if(std::holds_alternative<ast::call>(expr.value)){
		const auto& call = std::get<ast::call>(expr.value);
		foundTypes[call.validatedDef->get().ty] = {};
		for(const auto& arg : call.args)
			findUsedTypes_iter_addExpr(arg, foundTypes);
	}else if(std::holds_alternative<ast::varName>(expr.value)){
		const auto& varName = std::get<ast::varName>(expr.value);
		foundTypes[*varName.matchedType] = {};
	}
}

void findUsedTypes_iter(std::reference_wrapper<const ast::function> funcToSearch, std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher>& foundTypes){
	for(const auto& state : funcToSearch.get().body.statements){
		if(std::holds_alternative<ast::expr>(state)){
			const auto& expr = std::get<ast::expr>(state);
			findUsedTypes_iter_addExpr(expr, foundTypes);
		}else if(std::holds_alternative<ast::block::declaration>(state)){
			const auto& decl = std::get<ast::block::declaration>(state);
			foundTypes[decl.ty] = {};
		}
	}
}

std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher> cCodeGen::findUsedTypes(std::reference_wrapper<const ast::function> entrypoint){
	std::unordered_map<ast::type, cTypeInfo, cCodeGen::typeHasher> output;
	findUsedTypes_iter(entrypoint, output);
	return output;
}

void cCodeGen::genUsedCTypes(std::unordered_map<ast::type, cTypeInfo, typeHasher>& usedTypes){
	for(auto& [type, cInfo] : usedTypes){
		cInfo = genCTypeInfo(type);
	}
}

cCodeGen::cTypeInfo cCodeGen::genCTypeInfo(const ast::type& ty){
	
}


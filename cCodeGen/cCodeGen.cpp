#include "cCodeGen.hpp"
#include <cstring>

std::string cCodeGen::genCCode(const ast::context& context, const std::string_view entryPoint, bool autoBuild, std::span<const std::string> linkLibs){
	std::optional<std::reference_wrapper<const ast::function>> entry;
	for(const auto& func : context.funcs){
		if(func.name == entryPoint && func.ty == ast::type::void_type && func.args.size() == 0){
			entry = std::cref(func);
			break;
		}
	}
	if(!entry){
		std::cerr<<"Error: Unable to find entry point function \""<<entryPoint<<"\" (must be void->void)"<<std::endl;
		return "";
	}

	std::string code;
	auto types = findUsedTypes(*entry);
	code += genUsedCTypes(types);

	//then generate function forward defs
	//and finally generate the actual function definitons themselves
	
	std::cout<<code<<std::endl;
	return code;
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


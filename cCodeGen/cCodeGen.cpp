#include "cCodeGen.hpp"
#include <cstring>
#include "../hashCombine.hpp"

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

	std::string code = "#include <stdio.h>\n\n";
	auto types = findUsedTypes(*entry);
	auto funcs = findUsedFunctions(*entry);
	code += genUsedCTypes(types);
	code += genBuiltins();
	code += genUsedFunctionForwarddefs(funcs, types);
	code += genUsedFunctionDefs(funcs, types);

	//quick main stub
	code += "int main(int argc, char** argv){\n\t" + mangleFuncName(entry->get()) + "();\n\treturn(0);\n}\n";
	
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

std::string cCodeGen::mangleName(const std::string& name){
	return "cCodeGen_" + name.substr(0, 5) + std::to_string(hashing::hashValues(name));//for now just do this;
	       //later check for forbidden characters (eg template < & >)
}

std::string cCodeGen::mangleFuncName(const ast::function& func){
	std::size_t hash = hashing::hashValues(func.name, func.ty.hash());
	for(const auto& arg : func.args)
		hash = hashing::hashValues(hash, arg.ty.hash());
	return "cCodeGen_" + func.name.substr(0, 5) + std::to_string(hash);//for now just do this;
	       //later check for forbidden characters (eg template < & >)
}

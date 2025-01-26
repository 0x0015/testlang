#pragma once
#include "../ast/ast.hpp"
#include <unordered_map>
#include <span>

namespace cCodeGen{
	struct typeHasher{
		size_t operator()(const ast::type& p) const{
       			return p.hash();
		}
	};
	struct cTypeInfo{
		std::string cName;
		int isPointer = 0;
	};
	std::string mangleName(const std::string& name);
	std::string mangleFuncName(const ast::function& func);
	std::string genCCode(const ast::context& context, const std::string_view entryPoint);
	bool compileCCode(const std::string_view code, const std::string_view outputFilename, std::span<const std::string> linkLibs = {}, const std::string& farceCompiler = "");
	std::unordered_map<std::string, std::reference_wrapper<const ast::function>> findUsedFunctions(std::reference_wrapper<const ast::function> entrypoint);
	std::unordered_map<ast::type, cTypeInfo, typeHasher> findUsedTypes(std::reference_wrapper<const ast::function> entrypoint);
	std::string genUsedCTypes(std::unordered_map<ast::type, cTypeInfo, typeHasher>& usedTypes);

	std::string genUsedFunctionForwarddefs(const std::unordered_map<std::string, std::reference_wrapper<const ast::function>>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);
	std::string genUsedFunctionDefs(const std::unordered_map<std::string, std::reference_wrapper<const ast::function>>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);

	std::string genBuiltins();
}

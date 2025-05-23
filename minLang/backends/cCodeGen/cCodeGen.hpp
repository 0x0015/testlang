#pragma once
#include "../../ast/ast.hpp"
#include <unordered_map>
#include <unordered_set>
#include <span>
#include "../util.hpp"

namespace minLang{
namespace backends{
namespace cCodeGen{
	struct cTypeInfo{
		std::string cName;
		int isPointer = 0;
	};
	std::string mangleName(const std::string& name);
	std::string mangleFuncName(const ast::function& func);
	std::string genCCode(const ast::context& context, const std::string_view entryPoint);
	bool compileCCode(const std::string_view code, const std::string_view outputFilename, std::span<const std::string> linkLibs = {}, const std::string& farceCompiler = "");
	std::unordered_map<ast::type, cTypeInfo, typeHasher> findUsedTypes(std::reference_wrapper<const ast::function> entrypoint);
	std::string genUsedCTypes(std::unordered_map<ast::type, cTypeInfo, typeHasher>& usedTypes);
	std::string getDefaultTypeValue(const ast::type& ty, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);

	std::string genUsedFunctionForwarddefs(const std::unordered_set<std::reference_wrapper<const ast::function>, funcSigHasher, funcSigComp>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);
	std::string genUsedFunctionDefs(const std::unordered_set<std::reference_wrapper<const ast::function>, funcSigHasher, funcSigComp>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);

	std::string genBuiltins();
	std::string genTemplateFuncBuiltins(const std::unordered_set<std::reference_wrapper<const ast::function>, funcSigHasher, funcSigComp>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);
	bool isSpecificBuiltinFuncCase(const ast::function& func);
	std::string genSpecificBuiltinFuncCase(const ast::call& call, const std::vector<std::string>& generatedArgs);
}
}
}

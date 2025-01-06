#pragma once
#include "../ast/ast.hpp"
#include <unordered_map>
#include <map>
#include <span>

namespace cCodeGen{
	struct typeHasher{
		size_t operator()(const ast::type& p) const{
       			return std::hash<std::string>{}(p.toString());
		}
	};
	struct cTypeInfo{
		std::string cName;
		int isPointer = 0;
	};
	bool genCCode(const ast::context& context, const std::string_view entryPoint, bool autoBuild = false, std::span<const std::string> linkLibs = {});
	std::unordered_map<std::string, std::reference_wrapper<const ast::function>> findUsedFunctions(std::reference_wrapper<const ast::function> entrypoint);
	std::unordered_map<ast::type, cTypeInfo, typeHasher> findUsedTypes(std::reference_wrapper<const ast::function> entrypoint);
	cTypeInfo genCTypeInfo(const ast::type& ty);
	void genUsedCTypes(std::unordered_map<ast::type, cTypeInfo, typeHasher>& usedTypes);
}

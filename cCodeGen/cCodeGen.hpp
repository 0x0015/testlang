#pragma once
#include "../ast/ast.hpp"
#include <unordered_map>
#include <unordered_set>
#include <span>
#include "../hashCombine.hpp"

namespace cCodeGen{
	struct typeHasher{
		size_t operator()(const ast::type& p) const{
       			return p.hash();
		}
	};
	struct funcSigHasher{
		size_t operator()(const std::reference_wrapper<const ast::function>& func) const{
			std::size_t output = hashing::hashValues(func.get().name, func.get().ty.hash());
			for(const auto& arg : func.get().args)
				output = hashing::hashValues(output, arg.ty.hash());
			return output;
		}
	};
	struct funcSigComp{
		bool operator()(const std::reference_wrapper<const ast::function>& l, const std::reference_wrapper<const ast::function>& r) const{
			if(!(l.get().name == r.get().name && l.get().ty == r.get().ty))
				return false;
			if(l.get().args.size() != r.get().args.size())
				return false;
			for(unsigned int i=0;i<l.get().args.size();i++)
				if(l.get().args[i].ty != r.get().args[i].ty)
					return false;
			return true;
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
	std::unordered_set<std::reference_wrapper<const ast::function>, funcSigHasher, funcSigComp> findUsedFunctions(std::reference_wrapper<const ast::function> entrypoint);
	std::unordered_map<ast::type, cTypeInfo, typeHasher> findUsedTypes(std::reference_wrapper<const ast::function> entrypoint);
	std::string genUsedCTypes(std::unordered_map<ast::type, cTypeInfo, typeHasher>& usedTypes);
	std::string getDefaultTypeValue(const ast::type& ty, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);

	std::string genUsedFunctionForwarddefs(const std::unordered_set<std::reference_wrapper<const ast::function>, funcSigHasher, funcSigComp>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);
	std::string genUsedFunctionDefs(const std::unordered_set<std::reference_wrapper<const ast::function>, funcSigHasher, funcSigComp>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);

	std::string genBuiltins();
	std::string genTemplateFuncBuiltins(const std::unordered_set<std::reference_wrapper<const ast::function>, funcSigHasher, funcSigComp>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types);
}

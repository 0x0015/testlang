#pragma once
#include "../ast/ast.hpp"
#include "../hashCombine.hpp"
#include <unordered_map>
#include <unordered_set>

namespace minLang{
namespace backends{
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

std::unordered_set<std::reference_wrapper<const minLang::ast::function>, funcSigHasher, funcSigComp> findUsedFunctions(const ast::function& entrypoint);
std::unordered_set<ast::type, typeHasher> findUsedTypes(const ast::function& entrypoint);
bool checkAllCallsValidated(const ast::function& entrypoint);
bool checkAllCallsValidated_nonRecursive(const ast::function& func);

}
}

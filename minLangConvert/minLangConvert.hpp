#pragma once
#include <optional>
#include <unordered_map>
#include "../minLang/ast/ast.hpp"
#include "../ast/ast.hpp"
#include "../hashCombine.hpp"

using funcPtrMapkeyTy = std::reference_wrapper<const ast::function>;
struct funcPtrHasher{
	std::size_t operator()(const funcPtrMapkeyTy& funcPtr) const{
		std::size_t output = hashing::hashValues(funcPtr.get().ty.hash(), funcPtr.get().name, funcPtr.get().status);
		for(const auto& arg : funcPtr.get().args){
			output = hashing::hashValues(output, arg.name, arg.ty.hash());
		}
		//this in theory should be enough
		return output;
	}
};
struct funcPtrEqualityChecker{
	bool operator()(const funcPtrMapkeyTy& k1, const funcPtrMapkeyTy& k2) const{
		if(k1.get().ty != k2.get().ty)
			return false;
		if(k1.get().name != k2.get().name)
			return false;
		if(k1.get().args.size() != k2.get().args.size())
			return false;
		for(unsigned int i=0;i<k1.get().args.size();i++){
			if(!(k1.get().args[i].name == k2.get().args[i].name && k1.get().args[i].ty == k2.get().args[i].ty))
				return false;
		}
		return true;//not checking body!
	}
};

using toMinLangFuncMap = std::unordered_map<funcPtrMapkeyTy, std::reference_wrapper<const minLang::ast::function>, funcPtrHasher, funcPtrEqualityChecker>;

std::optional<minLang::ast::expr> minLangConvertExpr(const ast::expr& exp);
std::optional<minLang::ast::block> minLangConvertBlock(const ast::block& block);
std::optional<minLang::ast::type> minLangConvertType(const ast::type& ty);
std::optional<minLang::ast::function> minLangConvertFunction(const ast::function& func);
std::optional<minLang::ast::context> minLangConvert(const ast::context& context);


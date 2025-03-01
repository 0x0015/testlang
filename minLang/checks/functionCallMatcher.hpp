#pragma once
#include "../ast/ast.hpp"
#include <unordered_map>
#include <list>

namespace minLang{
struct multiContextDefinedVars_t{
private:
	static inline ast::type defaultReturn = ast::type::none_type;
public:
	std::list<std::reference_wrapper<const std::unordered_map<std::string, ast::type>>> upperDefinedVars;
	bool contains(const std::string& str) const;
	const ast::type& at(const std::string& str) const;
	multiContextDefinedVars_t(const multiContextDefinedVars_t& other, const std::unordered_map<std::string, ast::type>& definedVars){
		upperDefinedVars = other.upperDefinedVars;
		upperDefinedVars.push_front(std::cref(definedVars));//so newer(lower; more specific) contexts get searched first
	}
	multiContextDefinedVars_t(const std::unordered_map<std::string, ast::type>& definedVars){
		upperDefinedVars = {std::cref(definedVars)};
	}
};

struct functionCallMatcher{
	ast::context& context;
	std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>> allFunctions;
	struct conversionTypePairHasher{
		std::size_t operator()(const std::pair<ast::type, ast::type>& p) const;
	};
	// {to_ty, from_ty}->func
	std::unordered_map<std::pair<ast::type, ast::type>, std::reference_wrapper<const ast::function>, conversionTypePairHasher> conversionFunctions;
	std::optional<std::reference_wrapper<const ast::function>> matchCall(ast::call& call, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars);
};
}

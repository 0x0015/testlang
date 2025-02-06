#pragma once
#include "../ast/ast.hpp"
#include <unordered_map>
#include <list>

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
	std::unordered_multimap<std::string, std::reference_wrapper<const ast::functionTemplate>> allTemplateFunctions;
	struct conversionTypePairHasher{
		std::size_t operator()(const std::pair<ast::type, ast::type>& p) const;
	};
	// {to_ty, from_ty}->func
	std::unordered_map<std::pair<ast::type, ast::type>, std::reference_wrapper<const ast::function>, conversionTypePairHasher> conversionFunctions;
	struct templateInstantiationHasher{
		std::size_t operator()(const std::pair<std::string, std::vector<ast::type>>& p) const;
	};
	std::unordered_map<std::pair<std::string, std::vector<ast::type>>, std::reference_wrapper<const ast::function>, templateInstantiationHasher> templateInstantiations;
	std::optional<std::reference_wrapper<const ast::function>> matchCall(ast::call& call, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars);
	std::optional<std::reference_wrapper<const ast::function>> matchCallTryTemplateFallback(ast::call& call, const std::vector<ast::type>& callArgTypes, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars);
	std::optional<std::reference_wrapper<const ast::function>> matchTemplateCall(ast::templateCall& templCall, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars, bool ShowErrors = true);
};

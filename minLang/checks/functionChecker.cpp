#include "functionChecker.hpp"
#include <unordered_set>
#include <unordered_map>

bool minLang::checkFunctionsDefined(const minLang::ast::context& context){
	std::unordered_set<std::string> definedFunctions;
	for(const auto& func : context.funcs){
		definedFunctions.insert(func.name);
	}

	bool errored = false;
	for(const auto& func : context.funcs){
		for(const auto& state : func.body.statements){
			if(std::holds_alternative<minLang::ast::expr>(state)){
				const auto& expr = std::get<minLang::ast::expr>(state);
				if(std::holds_alternative<minLang::ast::call>(expr.value)){
					const auto& call = std::get<minLang::ast::call>(expr.value);
					if(!definedFunctions.contains(call.name)){
						std::cerr<<"Error: call to undefined function \""<<call.name<<"\" in function \""<<func.name<<"\""<<std::endl;
						errored = true;
					}
				}
			}
		}
	}

	return !errored;
}

bool minLang::checkConflictingFunctionDefinitions(const minLang::ast::context& context){
	std::unordered_multimap<std::string, std::reference_wrapper<const minLang::ast::function>> function_defs;
	std::unordered_set<std::string> funcNames;
	for(const auto& func : context.funcs){
		function_defs.insert({func.name, std::cref(func)});
		funcNames.insert(func.name);
	}

	bool errored = false;
	//try really hard to make this not O(n^2) (as for all the functions, that's REALLY not okay)
	for(const auto& funcName : funcNames){
		const auto& funcs = function_defs.equal_range(funcName);
		for(auto it1 = funcs.first; it1 != funcs.second; ++it1){
			for(auto it2 = it1; it2 != funcs.second; ++it2){
				if(it1 == it2)
					continue;
				if(it1->second.get().args.size() != it2->second.get().args.size())
					continue;
				bool allMatch = true;
				for(unsigned int i=0;i<it1->second.get().args.size();i++){
					if(it1->second.get().args[i].ty != it2->second.get().args[i].ty){
						allMatch = false;
						break;
					}
				}
				if(allMatch){
					if(it1->second.get().ty != it2->second.get().ty){
						if(funcName != "convert"){//convert is the AND ONLY ONE permissable case.  For that reason, cannot be called manually
							std::cerr<<"Error: Found functions with same name \""<<funcName<<"\" and arg types, but differing return types"<<std::endl;
							errored = true;
						}
						break;
					}
				}
				if(allMatch){
					std::cerr<<"Error: Found matching definitons for function \""<<funcName<<"\""<<std::endl;
					errored = true;
					break;
				}
			}
		}
	}

	return !errored;
}


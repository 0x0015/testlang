#include "functionCallMatcher.hpp"
#include "../hashCombine.hpp"
#include "typeChecker.hpp"
#include <set>


bool multiContextDefinedVars_t::contains(const std::string& str) const{
	for(const auto& definedVars : upperDefinedVars){
		if(definedVars.get().contains(str))
			return true;
	}
	return false;
}

const ast::type& multiContextDefinedVars_t::at(const std::string& str) const{
	for(const auto& definedVars : upperDefinedVars){
		if(definedVars.get().contains(str))
			return definedVars.get().at(str);
	}
	return defaultReturn;
}



size_t functionCallMatcher::templateInstantiationHasher::operator()(const std::pair<std::string, std::vector<ast::type>>& p) const{
	std::size_t output = hashing::hashValues(p.first);
	for(const auto& templateSubTy : p.second)
		output = hashing::hashValues(output, templateSubTy.hash());
	return output;
}

std::optional<std::reference_wrapper<const ast::function>> functionCallMatcher::matchCall(ast::call& call, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars){
	const auto& possibleMatches = allFunctions.equal_range(call.name);

	std::vector<ast::type> callArgTypes(call.args.size());
	bool unableToDetermineArgTypes = false;
	for(unsigned int i=0;i<call.args.size();i++){
		auto argType = deriveExprTypeAndFill(call.args[i], definedVars, parentFunction, *this);
		if(argType){
			callArgTypes[i] = *argType;
		}else{
			unableToDetermineArgTypes = true;
		}
	}
	if(unableToDetermineArgTypes)
		return std::nullopt;//error somewhere inside the arg type finding

	std::optional<std::reference_wrapper<const ast::function>> matchingFunc;
	unsigned int matchesFound = 0;
	unsigned int numPossibleMatches = 0;
	for(auto it = possibleMatches.first; it != possibleMatches.second; ++it){
		numPossibleMatches++;
		const auto& matchTry = it->second.get();
		bool argsMatch = true;
		for(unsigned int i=0;i<matchTry.args.size();i++){
			if(matchTry.args[i].ty != callArgTypes[i]){
				argsMatch = false;
				break;
			}
		}
		if(!argsMatch)
			continue;

		//if you got here, all the args both exist and match
		matchingFunc = std::cref(matchTry);
		matchesFound++;
	}

	if(numPossibleMatches == 0){
		const auto& templateTry = matchCallTryTemplateFallback(call, callArgTypes, parentFunction, definedVars);
		if(!templateTry){
			std::cerr<<"Error: call to unknown function \""<<call.name<<"\""<<std::endl;
			return std::nullopt;
		}
		return templateTry;
	}

	if(!matchingFunc || matchesFound == 0 /*should be equivelent*/){
		const auto& templateTry = matchCallTryTemplateFallback(call, callArgTypes, parentFunction, definedVars);
		if(templateTry)
			return templateTry;

		std::cerr<<"Error: no match found for call: ";
		ast::expr(call).dump();
		std::cout<<"Candidates ("<<numPossibleMatches<<"): "<<std::endl;
		for(auto it = possibleMatches.first; it != possibleMatches.second; ++it){
			const auto& possibleMatch = it->second.get();
			std::cout<<"\t";
			possibleMatch.dump();
		}
		return std::nullopt;
	}
	if(matchesFound != 1){
		std::cerr<<"Error: found multiple ("<<matchesFound<<") functions matching call to ";
		ast::expr(call).dump();
		return std::nullopt;
	}

	return *matchingFunc;
}

//from here: https://stackoverflow.com/a/15761097
namespace uniqueify_impl{
bool operator<(const ast::type& a, const ast::type& b){
	return a.toString() < b.toString();
}
struct target_less
{
    template<class It>
    bool operator()(It const &a, It const &b) const { return *a < *b; }
};
struct target_equal
{
    template<class It>
    bool operator()(It const &a, It const &b) const { return *a == *b; }
};
template<class It> It uniquify(It begin, It const end)
{
    std::vector<It> v;
    v.reserve(static_cast<size_t>(std::distance(begin, end)));
    for (It i = begin; i != end; ++i)
    { v.push_back(i); }
    std::stable_sort(v.begin(), v.end(), target_less());
    v.erase(std::unique(v.begin(), v.end(), target_equal()), v.end());
    std::sort(v.begin(), v.end());
    size_t j = 0;
    for (It i = begin; i != end && j != v.size(); ++i)
    {
        if (i == v[j])
        {
            using std::iter_swap; iter_swap(i, begin);
            ++j;
            ++begin;
        }
    }
    return begin;
}
}

std::optional<std::reference_wrapper<const ast::function>> functionCallMatcher::matchCallTryTemplateFallback(ast::call& call, const std::vector<ast::type>& callArgTypes, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars){
	//maybe not the most efficient, but definitely the easiest
	std::vector<ast::type> uniqueTypes = callArgTypes;
	uniqueTypes.erase(uniqueify_impl::uniquify(uniqueTypes.begin(),uniqueTypes.end()), uniqueTypes.end());

	ast::templateCall templEquiv{call.name, call.args, uniqueTypes};
	auto ret = matchTemplateCall(templEquiv, parentFunction, definedVars, true);
	if(ret){
		//call name; call arg types don't need to be updated
		call.args = templEquiv.args;
	}
	return ret;
}

std::optional<std::reference_wrapper<const ast::function>> functionCallMatcher::matchTemplateCall(ast::templateCall& templCall, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars, bool ShowErrors){
	const auto& possibleMatches = allTemplateFunctions.equal_range(templCall.name);

	std::vector<ast::type> callArgTypes(templCall.args.size());
	bool unableToDetermineArgTypes = false;
	for(unsigned int i=0;i<templCall.args.size();i++){
		auto argType = deriveExprTypeAndFill(templCall.args[i], definedVars, parentFunction, *this);
		if(argType){
			callArgTypes[i] = *argType;
		}else{
			unableToDetermineArgTypes = true;
		}
	}
	if(unableToDetermineArgTypes)
		return std::nullopt;//error somewhere inside the arg type finding

	std::optional<std::reference_wrapper<const ast::functionTemplate>> matchingTemplateFunc;
	unsigned int matchesFound = 0;
	unsigned int numPossibleMatches = 0;
	for(auto it = possibleMatches.first; it != possibleMatches.second; ++it){
		numPossibleMatches++;
		const auto& matchTry = it->second.get();
		if(matchTry.func.args.size() != templCall.args.size()){
			continue;
		}
		if(matchTry.numTemplateArgs != templCall.templateArgs.size()){
			continue;
		}
		bool argsMatch = true;
		for(unsigned int i=0;i<matchTry.func.args.size();i++){
			const auto& arg = matchTry.func.args[i];
			ast::type argType;
			if(std::holds_alternative<ast::type::template_type>(arg.ty.ty)){
				argType = templCall.templateArgs[std::get<ast::type::template_type>(arg.ty.ty).templateParamNum];
			}else{
				argType = arg.ty;
			}
			if(argType != callArgTypes[i]){
				argsMatch = false;
				break;
			}
		}
		if(!argsMatch)
			continue;

		//if you got here, all the args both exist and match
		matchingTemplateFunc = std::cref(matchTry);
		matchesFound++;
	}

	if(numPossibleMatches == 0){
		if(ShowErrors) std::cerr<<"Error: call to unknown template function \""<<templCall.name<<"\""<<std::endl;
		return std::nullopt;
	}

	if(!matchingTemplateFunc || matchesFound == 0 /*should be equivelent*/){
		if(!ShowErrors){
			return std::nullopt;
		}
		std::cerr<<"Error: no match found for template call: ";
		ast::expr(templCall).dump();
		std::cout<<"Candidates ("<<numPossibleMatches<<"): "<<std::endl;
		for(auto it = possibleMatches.first; it != possibleMatches.second; ++it){
			const auto& possibleMatch = it->second.get();
			std::cout<<"\t";
			possibleMatch.func.dump();
		}
		return std::nullopt;
	}
	if(matchesFound != 1){
		if(!ShowErrors)
			return std::nullopt;
		std::cerr<<"Error: found multiple ("<<matchesFound<<") template functions matching call to ";
		ast::expr(templCall).dump();
		return std::nullopt;
	}

	//we know what template function is being matched.  Now check if it's been instantiated already (and use that) or instantiate it.
	const auto& match = matchingTemplateFunc->get();
	std::pair<std::string, std::vector<ast::type>> instantiationInfo;
	instantiationInfo.first = match.func.name;
	instantiationInfo.second = templCall.templateArgs;

	if(templateInstantiations.contains(instantiationInfo)){
		return std::cref(templateInstantiations.at(instantiationInfo));
	}else{
		const auto& instantiation_obj = match.instantiate(templCall.templateArgs);
		if(instantiation_obj.status == ast::function::positionStatus::builtin){
			context.builtinFuncTemplateInstantiations.push_back({templCall.name, instantiation_obj});
		}else{
			context.funcs.push_back(instantiation_obj);
		}
		auto& instantiation = (instantiation_obj.status == ast::function::positionStatus::builtin) ? context.builtinFuncTemplateInstantiations.back().second : context.funcs.back();
		templateInstantiations.insert({instantiationInfo, instantiation});
		return instantiation;
	}
}


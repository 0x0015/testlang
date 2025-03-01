#include "functionCallMatcher.hpp"
#include "../hashCombine.hpp"
#include "typeChecker.hpp"

bool minLang::multiContextDefinedVars_t::contains(const std::string& str) const{
	for(const auto& definedVars : upperDefinedVars){
		if(definedVars.get().contains(str))
			return true;
	}
	return false;
}

const minLang::ast::type& minLang::multiContextDefinedVars_t::at(const std::string& str) const{
	for(const auto& definedVars : upperDefinedVars){
		if(definedVars.get().contains(str))
			return definedVars.get().at(str);
	}
	return defaultReturn;
}

std::size_t minLang::functionCallMatcher::conversionTypePairHasher::operator()(const std::pair<ast::type, ast::type>& p) const{
	return hashing::hashValues(p.first.hash(), p.second.hash());
}

std::optional<std::reference_wrapper<const minLang::ast::function>> minLang::functionCallMatcher::matchCall(ast::call& call, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars){
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
		std::cerr<<"Error: call to unknown function \""<<call.name<<"\""<<std::endl;
		return std::nullopt;
	}

	if(!matchingFunc || matchesFound == 0 /*should be equivelent*/){
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
bool operator<(const minLang::ast::type& a, const minLang::ast::type& b){
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


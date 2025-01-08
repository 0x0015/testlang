#include "typeChecker.hpp"
#include <unordered_map>
#include <optional>
#include <list>

struct multiContextDefinedVars_t{
private:
	static inline ast::type defaultReturn = ast::type::none_type;
public:
	std::list<std::reference_wrapper<const std::unordered_map<std::string, ast::type>>> upperDefinedVars;
	bool contains(const std::string& str) const{
		for(const auto& definedVars : upperDefinedVars){
			if(definedVars.get().contains(str))
				return true;
		}
		return false;
	}
	const ast::type& at(const std::string& str) const{
		for(const auto& definedVars : upperDefinedVars){
			if(definedVars.get().contains(str))
				return definedVars.get().at(str);
		}
		return defaultReturn;
	}
	multiContextDefinedVars_t(const multiContextDefinedVars_t& other, const std::unordered_map<std::string, ast::type>& definedVars){
		upperDefinedVars = other.upperDefinedVars;
		upperDefinedVars.push_front(std::cref(definedVars));//so newer(lower; more specific) contexts get searched first
	}
	multiContextDefinedVars_t(const std::unordered_map<std::string, ast::type>& definedVars){
		upperDefinedVars = {std::cref(definedVars)};
	}
};

std::optional<ast::type> deriveExprTypeAndFill(ast::expr& expr, const multiContextDefinedVars_t& definedVars, const ast::function& func, const std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>>& allFunctions){
	if(std::holds_alternative<ast::varName>(expr.value)){
		auto& varName = std::get<ast::varName>(expr.value);
		if(definedVars.contains(varName.name)){
			varName.matchedType = definedVars.at(varName.name);
			return *varName.matchedType;
		}else{
			std::cerr<<"Error: reference to unknown variable \""<<varName.name<<"\" in function \""<<func.name<<"\""<<std::endl;
		}
	}else if(std::holds_alternative<ast::literal>(expr.value)){
		auto& literal = std::get<ast::literal>(expr.value);
		return literal.ty;
	}else if(std::holds_alternative<ast::call>(expr.value)){
		auto& call = std::get<ast::call>(expr.value);
		const auto& possibleMatches = allFunctions.equal_range(call.name);

		std::vector<ast::type> callArgTypes(call.args.size());
		bool unableToDetermineArgTypes = false;
		for(unsigned int i=0;i<call.args.size();i++){
			auto argType = deriveExprTypeAndFill(call.args[i], definedVars, func, allFunctions);
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
			if(matchTry.args.size() != call.args.size()){
				continue;
			}
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
			expr.dump();
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
			expr.dump();
			return std::nullopt;
		}

		call.validatedDef = *matchingFunc;
		return matchingFunc->get().ty;
	}

	std::cerr<<"Error: expr of unknown type in function \""<<func.name<<"\""<<std::endl;
	return std::nullopt;
}

bool checkBlockTypeUsesValid(ast::block& block, const multiContextDefinedVars_t& definedVars_up, const ast::function& func, const std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>>& allFunctions){
	std::unordered_map<std::string, ast::type> definedVars_current;
	multiContextDefinedVars_t definedVars(definedVars_up, definedVars_current);
	bool errored = false;

	for(unsigned int i=0;i<block.statements.size();i++){
		auto& statement = block.statements[i];
		if(std::holds_alternative<ast::block::declaration>(statement)){
			const auto& decl = std::get<ast::block::declaration>(statement);
			if(definedVars.contains(decl.name)){
				std::cerr<<"Error: Redefintion of variable \""<<decl.name<<"\" in function \""<<func.name<<"\""<<std::endl;
				errored = true;
			}else if(decl.ty == ast::type::void_type){
				std::cerr<<"Error: Cannot declare variable \""<<decl.name<<"\" in function \""<<func.name<<"\" with type void"<<std::endl;
				errored = true;
			}else{
				definedVars_current[decl.name] = decl.ty;
			}
		}else if(std::holds_alternative<ast::block::assignment>(statement)){
			auto& assign = std::get<ast::block::assignment>(statement);
			const auto& assignFromType = deriveExprTypeAndFill(assign.assignFrom, definedVars, func, allFunctions);
			if(!assignFromType){
				errored = true;
				continue;
			}
			if(definedVars.contains(assign.assignTo)){
				if(definedVars.at(assign.assignTo) != *assignFromType){
					std::cerr<<"Error: assigning variable of type "<<assignFromType->toString()<<" to variable \""<<assign.assignTo<<"\" of type "<<definedVars.at(assign.assignTo).toString()<<std::endl;
					errored = true;
				}
			}else{
				//add a new definition here for the variable
				block.statements.insert(block.statements.begin() + i, ast::block::declaration{*assignFromType, assign.assignTo});
				definedVars_current[assign.assignTo] = *assignFromType;
				i++;
			}
		}else if(std::holds_alternative<ast::expr>(statement)){
			auto& expr = std::get<ast::expr>(statement);
			const auto& exprType = deriveExprTypeAndFill(expr, definedVars, func, allFunctions);
			if(!exprType){
				errored = true;
				continue;
			}
		}else if(std::holds_alternative<ast::block::ifStatement>(statement)){
			auto& ifStatement = std::get<ast::block::ifStatement>(statement);
			const auto& condExprType = deriveExprTypeAndFill(ifStatement.condition, definedVars, func, allFunctions);
			if(!condExprType){
				errored = true;
				continue;
			}
			if(*condExprType != ast::type(ast::type::bool_type)){
				std::cerr<<"Error: if statement condition must be a boolean (bool) value rather than "<<condExprType->toString()<<" in function \""<<func.name<<"\""<<std::endl;
				errored = true;
				continue;
			}
			if(!checkBlockTypeUsesValid(*ifStatement.ifBody, definedVars, func, allFunctions)){
				errored = true;
			}
			if(!checkBlockTypeUsesValid(*ifStatement.elseBody, definedVars, func, allFunctions)){
				errored = true;
			}
		}else if(std::holds_alternative<ast::block::returnStatement>(statement)){
			auto& retStatement = std::get<ast::block::returnStatement>(statement);
			const auto& retValType = deriveExprTypeAndFill(retStatement.val, definedVars, func, allFunctions);
			if(!retValType){
				errored = true;
				continue;
			}
			if(*retValType != func.ty){
				std::cerr<<"Error: return statement returns value of type "<<retValType->toString()<<" rather than function \""<<func.name<<"\"'s return type of "<<func.ty.toString()<<std::endl;
				errored = true;
				continue;
			}
		}else{
			//should never happen
			std::cerr<<"Error: encountered statement of unknown type! (should never happen)"<<std::endl;
			errored = true;
		}
	}

	return !errored;
}

bool checkFunctionTypeUsesValid(ast::function& func, const std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>>& allFunctions){
	bool errored = false;
	//add arguments as vars
	std::unordered_map<std::string, ast::type> definedVars_current;
	for(const auto& arg : func.args){
		if(definedVars_current.contains(arg.name)){
			std::cerr<<"Error: Multiple arguments of same name \""<<arg.name<<"\" in function \""<<func.name<<"\""<<std::endl;
			errored = true;
		}else{
			definedVars_current[arg.name] = arg.ty;
		}
	}

	if(!checkBlockTypeUsesValid(func.body, definedVars_current, func, allFunctions))
		errored = true;

	return !errored;
}

bool checkTypeUsesValid(ast::context& context){
	std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>> allFunctions;
	for(auto& func : context.funcs){
		allFunctions.insert({func.name, std::cref(func)});
	}

	bool errored = false;
	for(auto& func : context.funcs){
		errored = !checkFunctionTypeUsesValid(func, allFunctions) || errored; //note: needs this order otherwise the call is culled as errored is (usually) false
	}
	return !errored;
}



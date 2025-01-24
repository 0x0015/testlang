#include "typeChecker.hpp"
#include <unordered_map>
#include <optional>
#include <list>
#include "../hashCombine.hpp"

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

//quick forward defs
struct functionCallMatcher;
std::optional<ast::type> deriveExprTypeAndFill(ast::expr& expr, const multiContextDefinedVars_t& definedVars, const ast::function& func, functionCallMatcher& funcCallMatcher);

//one day seperate this into it's own file (with header, etc)
struct functionCallMatcher{
	ast::context& context;
	std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>> allFunctions;
	std::unordered_multimap<std::string, std::reference_wrapper<const ast::functionTemplate>> allTemplateFunctions;
	struct templateInstantiationHasher{
		size_t operator()(const std::pair<std::string, std::vector<ast::type>>& p) const{
			std::size_t output = hashing::hashValues(p.first);
			for(const auto& templateSubTy : p.second)
				output = hashing::hashValues(output, templateSubTy.hash());
			return output;
		}
	};
	std::unordered_map<std::pair<std::string, std::vector<ast::type>>, std::reference_wrapper<const ast::function>, templateInstantiationHasher> templateInstantiations;
	std::optional<std::reference_wrapper<const ast::function>> matchCall(ast::call& call, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars){
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
	std::optional<std::reference_wrapper<const ast::function>> matchTemplateCall(ast::templateCall& templCall, const ast::function& parentFunction, const multiContextDefinedVars_t& definedVars){
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
			std::cerr<<"Error: call to unknown function \""<<templCall.name<<"\""<<std::endl;
			return std::nullopt;
		}

		if(!matchingTemplateFunc || matchesFound == 0 /*should be equivelent*/){
			std::cerr<<"Error: no match found for call: ";
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
			std::cerr<<"Error: found multiple ("<<matchesFound<<") functions matching call to ";
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
			context.funcs.push_back(match.instantiate(templCall.templateArgs));
			auto& instantiation = context.funcs.back();
			templateInstantiations.insert({instantiationInfo, instantiation});
			return instantiation;
		}
	}
};

std::optional<ast::type> deriveExprTypeAndFill(ast::expr& expr, const multiContextDefinedVars_t& definedVars, const ast::function& func, functionCallMatcher& funcCallMatcher){
	if(std::holds_alternative<ast::varName>(expr.value)){
		auto& varName = std::get<ast::varName>(expr.value);
		if(definedVars.contains(varName.name)){
			varName.matchedType = definedVars.at(varName.name);
			return *varName.matchedType;
		}else{
			std::cerr<<"Error: reference to unknown variable \""<<varName.name<<"\" in function \""<<func.name<<"\""<<std::endl;
			return std::nullopt;
		}
	}else if(std::holds_alternative<ast::literal>(expr.value)){
		auto& literal = std::get<ast::literal>(expr.value);
		return literal.ty;
	}else if(std::holds_alternative<ast::call>(expr.value)){
		auto& call = std::get<ast::call>(expr.value);

		auto tryMatch = funcCallMatcher.matchCall(call, func, definedVars);
		if(!tryMatch)
			return std::nullopt;

		call.validatedDef = *tryMatch;
		return tryMatch->get().ty;
	}else if(std::holds_alternative<ast::templateCall>(expr.value)){
		auto& templCall = std::get<ast::templateCall>(expr.value);

		auto tryMatch = funcCallMatcher.matchTemplateCall(templCall, func, definedVars);
		if(!tryMatch)
			return std::nullopt;

		ast::call instantiationCall{templCall.name, templCall.args, *tryMatch};
		expr = instantiationCall;
		return tryMatch->get().ty;
	}

	std::cerr<<"Error: expr of unknown type in function \""<<func.name<<"\": "<<std::endl;
	expr.dump();//may crash if the memory is too messed up; but if it does it's entitled to do so
	return std::nullopt;
}

bool checkBlockTypeUsesValid(ast::block& block, const multiContextDefinedVars_t& definedVars_up, const ast::function& func, functionCallMatcher& funcCallMatcher){
	std::unordered_map<std::string, ast::type> definedVars_current;
	multiContextDefinedVars_t definedVars(definedVars_up, definedVars_current);
	bool errored = false;

	for(unsigned int i=0;i<block.statements.size();i++){
		auto& statement = block.statements[i];
		//declaration
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

		//assignment
		}else if(std::holds_alternative<ast::block::assignment>(statement)){
			auto& assign = std::get<ast::block::assignment>(statement);
			const auto& assignFromType = deriveExprTypeAndFill(assign.assignFrom, definedVars, func, funcCallMatcher);
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
				definedVars_current[assign.assignTo] = *assignFromType;
				block.statements.insert(block.statements.begin() + i, ast::block::declaration{*assignFromType, assign.assignTo});//need's to be inserted last before the loop goes again as the reference is destroyed and then when the loop goes again it's updated to a correct value
				i++;
			}

		//expr
		}else if(std::holds_alternative<ast::expr>(statement)){
			auto& expr = std::get<ast::expr>(statement);
			const auto& exprType = deriveExprTypeAndFill(expr, definedVars, func, funcCallMatcher);
			if(!exprType){
				errored = true;
				continue;
			}

		//if statement
		}else if(std::holds_alternative<ast::block::ifStatement>(statement)){
			auto& ifStatement = std::get<ast::block::ifStatement>(statement);
			const auto& condExprType = deriveExprTypeAndFill(ifStatement.condition, definedVars, func, funcCallMatcher);
			if(!condExprType){
				errored = true;
				continue;
			}
			if(*condExprType != ast::type(ast::type::bool_type)){
				std::cerr<<"Error: if statement condition must be a boolean (bool) value rather than "<<condExprType->toString()<<" in function \""<<func.name<<"\""<<std::endl;
				errored = true;
				continue;
			}
			if(!checkBlockTypeUsesValid(*ifStatement.ifBody, definedVars, func, funcCallMatcher)){
				errored = true;
			}
			if(!checkBlockTypeUsesValid(*ifStatement.elseBody, definedVars, func, funcCallMatcher)){
				errored = true;
			}

		//for statement (while mode)
		}else if(std::holds_alternative<ast::block::forStatement_while>(statement)){
			auto& whileStatement = std::get<ast::block::forStatement_while>(statement);
			const auto& condExprType = deriveExprTypeAndFill(whileStatement.condition, definedVars, func, funcCallMatcher);
			if(!condExprType){
				errored = true;
				continue;
			}
			if(*condExprType != ast::type(ast::type::bool_type)){
				std::cerr<<"Error: for statement (single argument mode) condition must be a boolean (bool) value rather than "<<condExprType->toString()<<" in function \""<<func.name<<"\""<<std::endl;
				errored = true;
				continue;
			}
			if(!checkBlockTypeUsesValid(*whileStatement.body, definedVars, func, funcCallMatcher)){
				errored = true;
			}

		//for statement (normal mode)
		}else if(std::holds_alternative<ast::block::forStatement_normal>(statement)){
			//TODO: clean this up!  IT IS A MESS.  probably take each of the block statement types and put them each into their own functions
			auto& forStatement = std::get<ast::block::forStatement_normal>(statement);

			/*begin copied and pasted from above in the asignment statement code*/
			const auto& assignFromType = deriveExprTypeAndFill(forStatement.initialDecl.assignFrom, definedVars, func, funcCallMatcher);
			if(!assignFromType){
				errored = true;
				continue;
			}
			bool addBlockDefStatement = false;
			if(definedVars.contains(forStatement.initialDecl.assignTo)){
				if(definedVars.at(forStatement.initialDecl.assignTo) != *assignFromType){
					std::cerr<<"Error: assigning variable of type "<<assignFromType->toString()<<" to variable \""<<forStatement.initialDecl.assignTo<<"\" of type "<<definedVars.at(forStatement.initialDecl.assignTo).toString()<<std::endl;
					errored = true;
				}
			}else{
				//add a new definition here for the variable
				addBlockDefStatement = true;
				definedVars_current[forStatement.initialDecl.assignTo] = *assignFromType;
				i++;
			}

			const auto& condExprType = deriveExprTypeAndFill(forStatement.breakCond, definedVars, func, funcCallMatcher);
			if(!condExprType){
				errored = true;
				continue;
			}
			if(*condExprType != ast::type(ast::type::bool_type)){
				std::cerr<<"Error: for statement (single argument mode) condition must be a boolean (bool) value rather than "<<condExprType->toString()<<" in function \""<<func.name<<"\""<<std::endl;
				errored = true;
				continue;
			}

			const auto& perLoopAsgnType = deriveExprTypeAndFill(forStatement.perLoopAsgn.assignFrom, definedVars, func, funcCallMatcher);
			if(!perLoopAsgnType){
				errored = true;
				continue;
			}
			if(definedVars.contains(forStatement.perLoopAsgn.assignTo)){
				if(definedVars.at(forStatement.perLoopAsgn.assignTo) != *perLoopAsgnType){
					std::cerr<<"Error: assigning variable of type "<<perLoopAsgnType->toString()<<" to variable \""<<forStatement.initialDecl.assignTo<<"\" of type "<<definedVars.at(forStatement.initialDecl.assignTo).toString()<<std::endl;
					errored = true;
				}else{
					definedVars_current[forStatement.initialDecl.assignTo] = *perLoopAsgnType;
				}
			}else{
				std::cerr<<"Error: cannot declare variable in third operand of for loop"<<std::endl;
			}

			/*end copied section*/
			if(!checkBlockTypeUsesValid(*forStatement.body, definedVars, func, funcCallMatcher)){
				errored = true;
			}
			if(addBlockDefStatement){
				//want this to happen last as to not invalidate all of memory
				block.statements.insert(block.statements.begin() + i, ast::block::declaration{*assignFromType, forStatement.initialDecl.assignTo});
			}

		//return statement
		}else if(std::holds_alternative<ast::block::returnStatement>(statement)){
			auto& retStatement = std::get<ast::block::returnStatement>(statement);
			const auto& retValType = deriveExprTypeAndFill(retStatement.val, definedVars, func, funcCallMatcher);
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

bool checkFunctionTypeUsesValid(ast::function& func, functionCallMatcher& funcCallMatcher){
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

	if(!checkBlockTypeUsesValid(func.body, definedVars_current, func, funcCallMatcher))
		errored = true;

	return !errored;
}

bool checkTypeUsesValid(ast::context& context){
	functionCallMatcher funcCallMatcher{context};
	for(const auto& func : context.funcs){
		funcCallMatcher.allFunctions.insert({func.name, std::cref(func)});
	}
	for(const auto& funcTempl : context.funcTemplates){
		funcCallMatcher.allTemplateFunctions.insert({funcTempl.func.name, std::cref(funcTempl)});
	}

	bool errored = false;
	//note as we're adding more functions (as template instantiations come in) this cannot be range based
	//actually can now as to solve this problem (as it comes up later) we moved from vector -> list (as we don't really need random access anyway)
	for(auto& func : context.funcs){
		errored = !checkFunctionTypeUsesValid(func, funcCallMatcher) || errored; //note: needs this order otherwise the call is culled as errored is (usually) false
	}

	return !errored;
}



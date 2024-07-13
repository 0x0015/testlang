#include "typeChecker.hpp"
#include <unordered_map>
#include <optional>

ast::type getArgumentType(std::unordered_map<std::string, ast::type>& definedVars, const ast::function::call::argument& arg){
	if(std::holds_alternative<ast::function::call::varNameArg>(arg)){
		return definedVars[std::get<ast::function::call::varNameArg>(arg)];
	}else if(std::holds_alternative<ast::literal>(arg)){
		return std::get<ast::literal>(arg).ty;
	}else{
		std::cerr<<"Error: unknown internal argument type error"<<std::endl;
		return ast::type::none_type;
	}
}

bool checkTypeUsesValid(ast::context& context){
	std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>> functions;
	for(auto& func : context.funcs){
		functions.insert({func.name, std::cref(func)});
	}
	
	bool errored = false;
	for(auto& func : context.funcs){
		std::unordered_map<std::string, ast::type> definedVars;
		for(const auto& arg : func.args){
			if(definedVars.contains(arg.name)){
				std::cerr<<"Error: Multiple arguments of same name \""<<arg.name<<"\" in function \""<<func.name<<"\""<<std::endl;
				errored = true;
			}else{
				definedVars[arg.name] = arg.ty;
			}
		}
		for(unsigned int i=0;i<func.body.size();i++){
			auto& state = func.body[i];
			if(std::holds_alternative<ast::function::declaration>(state)){
				const auto& decl = std::get<ast::function::declaration>(state);
				if(definedVars.contains(decl.name)){
					std::cerr<<"Error: Redefintion of variable \""<<decl.name<<"\" in function \""<<func.name<<"\""<<std::endl;
					errored = true;
				}else if(decl.ty == ast::type::void_type){
					std::cerr<<"Error: Cannot declare variable \""<<decl.name<<"\" in function \""<<func.name<<"\" with type void"<<std::endl;
					errored = true;
				}else{
					definedVars[decl.name] = decl.ty;
				}
			}else if(std::holds_alternative<ast::function::assignment>(state)){
				const auto& asgn = std::get<ast::function::assignment>(state);
				ast::type asgnType;
				if(std::holds_alternative<std::string>(asgn.assignFrom)){
					const auto& fromName = std::get<std::string>(asgn.assignFrom);
					if(!definedVars.contains(fromName)){	
						std::cerr<<"Error: unable to assign from unknown variable \""<<fromName<<"\""<<std::endl;
						errored = true;
					}else{
						asgnType = definedVars[fromName];
					}
				}else if(std::holds_alternative<ast::literal>(asgn.assignFrom)){	
					const auto& fromLit = std::get<ast::literal>(asgn.assignFrom);
					asgnType = fromLit.ty;
				}else{
					std::cerr<<"Error: assignment is not assigning from any target"<<std::endl;
					errored = true;
				}
				if(definedVars.contains(asgn.assignTo)){
					if(definedVars[asgn.assignTo] != asgnType){
						std::cerr<<"Error: assigning variable of type "<<asgnType.toString()<<" to variable \""<<asgn.assignTo<<"\" of type "<<definedVars[asgn.assignTo].toString()<<std::endl;
						errored = true;
					}
				}else{
					//add a new definition here for the variable
					func.body.insert(func.body.begin() + i, ast::function::declaration{asgnType, asgn.assignTo});
					definedVars[asgn.assignTo] = asgnType;
					i++;
				}
			}else if(std::holds_alternative<ast::function::call>(state)){
				auto& call = std::get<ast::function::call>(state);
				const auto& funcs = functions.equal_range(call.name);

				bool unableToDecernCallargTypes = false;
				for(unsigned int i=0;i<call.args.size();i++){
					if(std::holds_alternative<ast::function::call::varNameArg>(call.args[i]) && !definedVars.contains(std::get<ast::function::call::varNameArg>(call.args[i]))){
						std::cerr<<"Error: Use of undefined variable \""<<std::get<ast::function::call::varNameArg>(call.args[i])<<"\" in call to \""<<call.name<<"\", in function \""<<func.name<<"\""<<std::endl;
						errored = true;
						unableToDecernCallargTypes = true;
						break;
					}
				}
				if(unableToDecernCallargTypes)
					continue;

				std::optional<std::reference_wrapper<const ast::function>> matchingFunc;
				for(auto it = funcs.first; it != funcs.second; ++it){
					const auto& func = it->second.get();
					if(func.args.size() != call.args.size()){
						continue;
					}
					bool argsMatch = true;
					for(unsigned int i=0;i<func.args.size();i++){
						ast::type varType = getArgumentType(definedVars, call.args[i]);
						if(func.args[i].ty != varType){
							argsMatch = false;
							break;
						}
					}
					if(!argsMatch)
						continue;

					//if you got here, all the args both exist and match
					matchingFunc = std::cref(func);
				}
				if(!matchingFunc){
					//there was no matching function! figure out why
					std::cerr<<"Error: No function definition for \""<<call.name<<"\" matching "<<call.name<<"(";
					for(unsigned int i=0;i<call.args.size();i++){
						ast::type varType = getArgumentType(definedVars, call.args[i]);
						std::cerr<<varType.toString();
						if(i+1 < call.args.size())
							std::cerr<<", ";
					}
					std::cerr<<")"<<std::endl;
					std::cerr<<"Candidates:"<<std::endl;
					for(auto it = funcs.first; it != funcs.second; ++it){
						const auto& func = it->second.get();
						std::cerr<<"\t"<<func.ty.toString()<<" "<<func.name<<"(";
						for(unsigned int i=0;i<func.args.size();i++){
							std::cerr<<func.args[i].ty.toString();
							if(i+1 < func.args.size())
								std::cerr<<", ";
						}
						std::cerr<<")"<<std::endl;
					}

					errored = true;
				}
				//realistically here I should do something with the matched functions (as I'm definitely going to need that)
				call.validatedDef = matchingFunc;
			}
		}
	}

	return !errored;
}


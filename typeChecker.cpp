#include "typeChecker.hpp"
#include <unordered_map>
#include <optional>

ast::type getArgumentType(std::unordered_map<std::string, ast::type>& definedVars, const ast::function::call::argument& arg){
	if(std::holds_alternative<ast::function::call::varNameArg>(arg)){
		return definedVars[std::get<ast::function::call::varNameArg>(arg)];
	}else if(std::holds_alternative<ast::function::call::literalArg>(arg)){
		return ast::function::call::getLiteralType(std::get<ast::function::call::literalArg>(arg));
	}else{
		std::cerr<<"Error: unknown internal argument type error"<<std::endl;
		return ast::none_type;
	}
}

bool checkTypeUsesValid(ast::context& context){
	std::unordered_multimap<std::string, std::reference_wrapper<const ast::function>> functions;
	for(const auto& func : context.funcs){
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
		for(auto& state : func.body){
			if(std::holds_alternative<ast::function::declaration>(state)){
				const auto& decl = std::get<ast::function::declaration>(state);
				if(definedVars.contains(decl.name)){
					std::cerr<<"Error: Redefintion of variable \""<<decl.name<<"\" in function \""<<func.name<<"\""<<std::endl;
					errored = true;
				}else if(decl.ty == ast::void_type){
					std::cerr<<"Error: Cannot declare variable \""<<decl.name<<"\" in function \""<<func.name<<"\" with type void"<<std::endl;
					errored = true;
				}else{
					definedVars[decl.name] = decl.ty;
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
						std::cerr<<ast::type_rmap(varType);
						if(i+1 < call.args.size())
							std::cerr<<", ";
					}
					std::cerr<<")"<<std::endl;
					std::cerr<<"Candidates:"<<std::endl;
					for(auto it = funcs.first; it != funcs.second; ++it){
						const auto& func = it->second.get();
						std::cerr<<"\t"<<ast::type_rmap(func.ty)<<" "<<func.name<<"(";
						for(unsigned int i=0;i<func.args.size();i++){
							std::cerr<<ast::type_rmap(func.args[i].ty);
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


#include "typeChecker.hpp"
#include <unordered_map>
#include <optional>
#include "functionCallMatcher.hpp"

std::optional<minLang::ast::type> minLang::deriveExprTypeAndFill(minLang::ast::expr& expr, const multiContextDefinedVars_t& definedVars, const minLang::ast::function& func, functionCallMatcher& funcCallMatcher){
	if(std::holds_alternative<minLang::ast::varName>(expr.value)){
		auto& varName = std::get<minLang::ast::varName>(expr.value);
		if(definedVars.contains(varName.name)){
			varName.matchedType = definedVars.at(varName.name);
			return *varName.matchedType;
		}else{
			std::cerr<<"Error: reference to unknown variable \""<<varName.name<<"\" in expr ";expr.dump();std::cout<<" in function \""<<func.name<<"\""<<std::endl;
			return std::nullopt;
		}
	}else if(std::holds_alternative<minLang::ast::literal>(expr.value)){
		auto& literal = std::get<minLang::ast::literal>(expr.value);
		return literal.ty;
	}else if(std::holds_alternative<minLang::ast::call>(expr.value)){
		auto& call = std::get<minLang::ast::call>(expr.value);

		auto tryMatch = funcCallMatcher.matchCall(call, func, definedVars);
		if(!tryMatch)
			return std::nullopt;

		call.validatedDef = *tryMatch;
		return tryMatch->get().ty;
	}

	std::cerr<<"Error: expr of unknown type in function \""<<func.name<<"\": "<<std::endl;
	expr.dump();//may crash if the memory is too messed up; but if it does it's entitled to do so
	return std::nullopt;
}

bool checkBlockTypeUsesValid(minLang::ast::block& block, const minLang::multiContextDefinedVars_t& definedVars_up, const minLang::ast::function& func, minLang::functionCallMatcher& funcCallMatcher){
	std::unordered_map<std::string, minLang::ast::type> definedVars_current;
	minLang::multiContextDefinedVars_t definedVars(definedVars_up, definedVars_current);
	bool errored = false;

	for(unsigned int i=0;i<block.statements.size();i++){
		auto& statement = block.statements[i];
		//declaration
		if(std::holds_alternative<minLang::ast::block::declaration>(statement)){
			const auto& decl = std::get<minLang::ast::block::declaration>(statement);
			if(definedVars.contains(decl.name)){
				std::cerr<<"Error: Redefintion of variable \""<<decl.name<<"\" in function \""<<func.name<<"\""<<std::endl;
				errored = true;
			}else if(decl.ty == minLang::ast::type::void_type){
				std::cerr<<"Error: Cannot declare variable \""<<decl.name<<"\" in function \""<<func.name<<"\" with type void"<<std::endl;
				errored = true;
			}else{
				definedVars_current[decl.name] = decl.ty;
			}

		//assignment
		}else if(std::holds_alternative<minLang::ast::block::assignment>(statement)){
			auto& assign = std::get<minLang::ast::block::assignment>(statement);
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
				std::cerr<<"Error: assigning to undefined variable \""<<assign.assignTo<<"\""<<std::endl;
				errored = true;
				continue;
			}

		//expr
		}else if(std::holds_alternative<minLang::ast::expr>(statement)){
			auto& expr = std::get<minLang::ast::expr>(statement);
			const auto& exprType = deriveExprTypeAndFill(expr, definedVars, func, funcCallMatcher);
			if(!exprType){
				errored = true;
				continue;
			}

		//if statement
		}else if(std::holds_alternative<minLang::ast::block::ifStatement>(statement)){
			auto& ifStatement = std::get<minLang::ast::block::ifStatement>(statement);
			const auto& condExprType = deriveExprTypeAndFill(ifStatement.condition, definedVars, func, funcCallMatcher);
			if(!condExprType){
				errored = true;
				continue;
			}
			if(*condExprType != minLang::ast::type(minLang::ast::type::bool_type)){
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
		}else if(std::holds_alternative<minLang::ast::block::forStatement_while>(statement)){
			auto& whileStatement = std::get<minLang::ast::block::forStatement_while>(statement);
			const auto& condExprType = deriveExprTypeAndFill(whileStatement.condition, definedVars, func, funcCallMatcher);
			if(!condExprType){
				errored = true;
				continue;
			}
			if(*condExprType != minLang::ast::type(minLang::ast::type::bool_type)){
				std::cerr<<"Error: for statement (single argument mode) condition must be a boolean (bool) value rather than "<<condExprType->toString()<<" in function \""<<func.name<<"\""<<std::endl;
				errored = true;
				continue;
			}
			if(!checkBlockTypeUsesValid(*whileStatement.body, definedVars, func, funcCallMatcher)){
				errored = true;
			}

		//for statement (normal mode)
		}else if(std::holds_alternative<minLang::ast::block::forStatement_normal>(statement)){
			//TODO: clean this up!  IT IS A MESS.  probably take each of the block statement types and put them each into their own functions
			auto& forStatement = std::get<minLang::ast::block::forStatement_normal>(statement);

			/*begin copied and pasted from above in the asignment statement code*/
			const auto& initialDeclType = deriveExprTypeAndFill(forStatement.initialDecl.assignFrom, definedVars, func, funcCallMatcher);
			if(!initialDeclType){
				errored = true;
				continue;
			}
			bool addBlockDefStatement = false;
			if(definedVars.contains(forStatement.initialDecl.assignTo)){
				if(definedVars.at(forStatement.initialDecl.assignTo) != *initialDeclType){
					std::cerr<<"Error: assigning variable of type "<<initialDeclType->toString()<<" to variable \""<<forStatement.initialDecl.assignTo<<"\" of type "<<definedVars.at(forStatement.initialDecl.assignTo).toString()<<std::endl;
					errored = true;
					continue;
				}
			}else{
				//add a new definition here for the variable
				addBlockDefStatement = true;
				definedVars_current[forStatement.initialDecl.assignTo] = *initialDeclType;
			}

			const auto& condExprType = deriveExprTypeAndFill(forStatement.breakCond, definedVars, func, funcCallMatcher);
			if(!condExprType){
				errored = true;
				continue;
			}
			if(*condExprType != minLang::ast::type(minLang::ast::type::bool_type)){
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
				errored = true;
			}

			/*end copied section*/
			if(!checkBlockTypeUsesValid(*forStatement.body, definedVars, func, funcCallMatcher)){
				errored = true;
			}
			if(addBlockDefStatement){
				//want this to happen last as to not invalidate all of memory
				block.statements.insert(block.statements.begin() + i, minLang::ast::block::declaration{*initialDeclType, forStatement.initialDecl.assignTo});
				i++;
			}

		//return statement
		}else if(std::holds_alternative<minLang::ast::block::returnStatement>(statement)){
			auto& retStatement = std::get<minLang::ast::block::returnStatement>(statement);
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

bool checkFunctionTypeUsesValid(minLang::ast::function& func, minLang::functionCallMatcher& funcCallMatcher){
	bool errored = false;
	//add arguments as vars
	std::unordered_map<std::string, minLang::ast::type> definedVars_current;
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

bool minLang::checkTypeUsesValid(minLang::ast::context& context){
	functionCallMatcher funcCallMatcher{context};
	for(const auto& func : context.funcs){
		if(func.name == "convert"){
			funcCallMatcher.conversionFunctions.insert({{func.ty, func.args[0].ty}, std::cref(func)});
		}
		funcCallMatcher.allFunctions.insert({func.name, std::cref(func)});
	}

	bool errored = false;
	//note as we're adding more functions (as template instantiations come in) this cannot be range based
	//actually can now as to solve this problem (as it comes up later) we moved from vector -> list (as we don't really need random access anyway)
	for(auto& func : context.funcs){
		errored = !checkFunctionTypeUsesValid(func, funcCallMatcher) || errored; //note: needs this order otherwise the call is culled as errored is (usually) false
	}

	return !errored;
}



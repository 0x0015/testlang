#include "interpreter.hpp"

void addVariableToInterpreterStack(interpreter::interpreter& M, const std::string& name, unsigned int size){
	M.functionExecutions.back().variablePtrs[name] = M.stackPtr;
	M.stackPtr += size;
	M.functionExecutions.back().usedStack += size;
}

void interpretFunction(interpreter::interpreter& M, const ast::function& func){
	for(const auto& state : func.body){
		if(std::holds_alternative<ast::function::declaration>(state)){
			const auto& decl = std::get<ast::function::declaration>(state);
			addVariableToInterpreterStack(M, decl.name, decl.ty.getSize());
		}else if(std::holds_alternative<ast::function::call>(state)){
			const auto& call = std::get<ast::function::call>(state);
			if(call.validatedDef.value().get().builtin){
				interpreter::handleBulitin(call.validatedDef.value().get(), call, M);
			}else{
				auto& currentFunc = M.functionExecutions.back();
				M.functionExecutions.push_back({});
				for(auto& arg : call.args){
					//TODO: generalize calling to allow literals
					//really though, I should do more compil-y memory management and put all the literals in a buffer, etc...
					if(std::holds_alternative<std::string>(arg))
						M.functionExecutions.back().variablePtrs[std::get<std::string>(arg)] = currentFunc.variablePtrs[std::get<std::string>(arg)];
				}
				interpretFunction(M, call.validatedDef.value().get());
			}
		}
	}
	M.stackPtr -= M.functionExecutions.back().usedStack;
	M.functionExecutions.pop_back();
}

void interpreter::interpret(const ast::context& context, const std::string_view entryPoint){
	std::optional<std::reference_wrapper<const ast::function>> entry;
	for(const auto& func : context.funcs){
		if(func.name == entryPoint && func.ty == ast::type::void_type && func.args.size() == 0){
			entry = std::cref(func);
			break;
		}
	}
	if(!entry){
		std::cerr<<"Error: Unable to find entry point function \""<<entryPoint<<"\" (must be void->void)"<<std::endl;
		return;
	}
	interpreter M;
	M.stack.resize(interpreter::stackSize);
	M.stackPtr = 0;

	M.functionExecutions.push_back({});
	interpretFunction(M, entry.value().get());
}


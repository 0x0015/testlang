#include "interpreter.hpp"
#include <cstring>

void addVariableToInterpreterStack(interpreter::interpreter& M, const std::string& name, unsigned int size){
	M.functionExecutions.back().variablePtrs[name] = M.stackPtr;
	M.functionExecutions.back().variableSizes[name] = size;
	M.stackPtr += size;
	M.functionExecutions.back().usedStack += size;
}

void interpretFunction(interpreter::interpreter& M, const ast::function& func){
	for(const auto& state : func.body.statements){
		if(std::holds_alternative<ast::block::declaration>(state)){
			const auto& decl = std::get<ast::block::declaration>(state);
			addVariableToInterpreterStack(M, decl.name, decl.ty.getSize());
		}else if(std::holds_alternative<ast::block::assignment>(state)){
			const auto& asgn = std::get<ast::block::assignment>(state);
			if(std::holds_alternative<ast::varName>(asgn.assignFrom.value)){
				const auto& from = std::get<ast::varName>(asgn.assignFrom.value).name;
				std::memcpy(M.stack.data() + M.functionExecutions.back().variablePtrs[asgn.assignTo], M.stack.data() + M.functionExecutions.back().variablePtrs[from], M.functionExecutions.back().variableSizes[from]);
			}else if(std::holds_alternative<ast::literal>(asgn.assignFrom.value)){
				const auto& lit = std::get<ast::literal>(asgn.assignFrom.value);
				const void* ptr;
				std::visit([&ptr](auto& x){ptr = &x;}, lit.value);
				std::memcpy(M.stack.data() + M.functionExecutions.back().variablePtrs[asgn.assignTo], ptr, lit.ty.getSize());
			}//no third case
		}else if(std::holds_alternative<ast::expr>(state)){
			const auto& exp = std::get<ast::expr>(state);
			if(std::holds_alternative<ast::call>(exp.value)){
				const auto& call = std::get<ast::call>(exp.value);
				if(call.validatedDef.value().get().status == ast::function::positionStatus::builtin){
					interpreter::handleBulitin(call.validatedDef.value().get(), call, M);
				}else if(call.validatedDef.value().get().status == ast::function::positionStatus::external){
					M.externalHandler.handleExternal(call.validatedDef.value().get(), call, M);
				}else{
					auto& currentFunc = M.functionExecutions.back();
					M.functionExecutions.push_back({});
					for(auto& arg : call.args){
						//TODO: generalize calling to allow literals
						//really though, I should do more compil-y memory management and put all the literals in a buffer, etc...
						if(std::holds_alternative<ast::varName>(arg.value))
							M.functionExecutions.back().variablePtrs[std::get<ast::varName>(arg.value).name] = currentFunc.variablePtrs[std::get<ast::varName>(arg.value).name];
						else
							std::cerr<<"Currently unable to evaluate function args of non-variable expressions.  Coming soon!"<<std::endl;
					}
					interpretFunction(M, call.validatedDef.value().get());
				}
			}
		}
	}
	M.stackPtr -= M.functionExecutions.back().usedStack;
	M.functionExecutions.pop_back();
}

bool interpreter::interpret(const ast::context& context, const std::string_view entryPoint, std::span<const std::string> linkLibs){
	auto externalHandler = loadExternalFunctions(context.funcs, linkLibs);
	if(!externalHandler)
		return false;

	std::optional<std::reference_wrapper<const ast::function>> entry;
	for(const auto& func : context.funcs){
		if(func.name == entryPoint && func.ty == ast::type::void_type && func.args.size() == 0){
			entry = std::cref(func);
			break;
		}
	}
	if(!entry){
		std::cerr<<"Error: Unable to find entry point function \""<<entryPoint<<"\" (must be void->void)"<<std::endl;
		return false;
	}
	interpreter M;
	M.stack.resize(interpreter::stackSize);
	M.stackPtr = 0;
	M.externalHandler = *externalHandler;

	M.functionExecutions.push_back({});
	interpretFunction(M, entry.value().get());

	return true;
}


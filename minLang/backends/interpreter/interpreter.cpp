#include "interpreter.hpp"
#include <cstring>
#include "../util.hpp"

bool minLang::backends::interpreter::interpret(const minLang::ast::context& context, const std::string_view entryPoint, std::span<const std::string> linkLibs){
	std::optional<std::reference_wrapper<const minLang::ast::function>> entry;
	for(const auto& func : context.funcs){
		if(func.name == entryPoint && func.ty == minLang::ast::type::void_type && func.args.size() == 0){
			entry = std::cref(func);
			break;
		}
	}
	if(!entry){
		std::cerr<<"Error: Unable to find entry point function \""<<entryPoint<<"\" (must be void->void)"<<std::endl;
		return false;
	}
	if(!checkAllCallsValidated(*entry)){
		std::cerr<<"Error: Non-valid calls were found.  This should never happen.  Aborting"<<std::endl;
		return false;
	}
	interpreter M;

	M.initializeBuiltinLibrary();
	if(!M.loadExternalFunctions(context.funcs, linkLibs))
		return false;
	
	minLang::ast::call mainCall{std::string(entryPoint), {}, *entry};
	M.interpretCall(mainCall);

	return true;
}


std::vector<uint8_t> getLiteralValue(const minLang::ast::literal& lit){
	if(std::holds_alternative<minLang::ast::literal::builtin_literal>(lit.value)){
		const auto& builtin = std::get<minLang::ast::literal::builtin_literal>(lit.value);
		if(builtin.valueless_by_exception()){
			std::cerr<<"Error: got builtin literal with no value"<<std::endl;
			return {};
		}
		const auto& output = std::visit([](const auto& arg){
			using arg_t = std::decay_t<decltype(arg)>;
			std::vector<uint8_t> output(sizeof(arg_t));
			std::memcpy(output.data(), &arg, sizeof(arg_t));
			return output;
		}, builtin);
		if(output.size() != lit.ty.getSize()){
			std::cerr<<"Interpreter error: Got builtin literal value of incorrect size (was "<<output.size()<<", should be "<<lit.ty.getSize()<<")"<<std::endl;
		}
		return output;
	}else if(std::holds_alternative<minLang::ast::literal::array_literal>(lit.value)){
		const auto& array = std::get<minLang::ast::literal::array_literal>(lit.value);
		std::vector<uint8_t> output;
		for(const auto& elm : array.vals){
			const auto& val = getLiteralValue(elm);
			unsigned int oldOutputSize = output.size();
			output.resize(output.size() + val.size());
			std::memcpy(output.data() + oldOutputSize, val.data(), val.size());
		}
		return output;
	}else if(std::holds_alternative<minLang::ast::literal::tuple_literal>(lit.value)){
		const auto& tuple = std::get<minLang::ast::literal::tuple_literal>(lit.value);
		std::vector<uint8_t> output;
		for(const auto& elm : tuple.vals){
			const auto& val = getLiteralValue(elm);
			unsigned int oldOutputSize = output.size();
			output.resize(output.size() + val.size());
			std::memcpy(output.data() + oldOutputSize, val.data(), val.size());
		}
		return output;
	}else{
		std::cerr<<"Error: got literal object with no value"<<std::endl;
		return {};
	}
}

bool minLang::backends::interpreter::interpreter::interpretStatement(const minLang::ast::block::statement& state){
	if(std::holds_alternative<minLang::ast::block::declaration>(state)){
		const auto& decl = std::get<minLang::ast::block::declaration>(state);
		varEntries[decl.name] = std::vector<uint8_t>(decl.ty.getSize());
	}else if(std::holds_alternative<minLang::ast::block::assignment>(state)){
		const auto& assignment = std::get<minLang::ast::block::assignment>(state);
		varEntries[assignment.assignTo] = interpretExpr(assignment.assignFrom);
	}else if(std::holds_alternative<minLang::ast::expr>(state)){
		const auto& expr = std::get<minLang::ast::expr>(state);
		interpretExpr(expr);
	}else if(std::holds_alternative<minLang::ast::block::ifStatement>(state)){
		const auto& ifState = std::get<minLang::ast::block::ifStatement>(state);
		if(interpretExpr(ifState.condition).front() != 0){//blindly treat the value as a bool.  What could go wrong (should be already type checked, so should be fine)
			interpretBlock(*ifState.ifBody);
		}else{
			interpretBlock(*ifState.elseBody);
		}
	}else if(std::holds_alternative<minLang::ast::block::forStatement_while>(state)){
		const auto& whileState = std::get<minLang::ast::block::forStatement_while>(state);
		bool cond = interpretExpr(whileState.condition).front() != 0;
		while(cond){
			interpretBlock(*whileState.body);	
			cond = interpretExpr(whileState.condition).front() != 0;
		}
	}else if(std::holds_alternative<minLang::ast::block::forStatement_normal>(state)){
		const auto& forState = std::get<minLang::ast::block::forStatement_normal>(state);
		varEntries[forState.initialDecl.assignTo] = interpretExpr(forState.initialDecl.assignFrom);
		bool cond = interpretExpr(forState.breakCond).front() != 0;
		while(cond){
			interpretBlock(*forState.body);
			varEntries[forState.perLoopAsgn.assignTo] = interpretExpr(forState.perLoopAsgn.assignFrom);
			cond = interpretExpr(forState.breakCond).front() != 0;
		}
	}else if(std::holds_alternative<minLang::ast::block::returnStatement>(state)){
		const auto& ret = std::get<minLang::ast::block::returnStatement>(state);
		returnBuffer = interpretExpr(ret.val);
	}else{
		std::cerr<<"Error: unable to interpret statement with no value"<<std::endl;
		return false;
	}
	return true;
}

std::vector<uint8_t> minLang::backends::interpreter::interpreter::interpretExpr(const minLang::ast::expr& expr){
	if(std::holds_alternative<minLang::ast::literal>(expr.value)){
		const auto& lit = std::get<minLang::ast::literal>(expr.value);
		return getLiteralValue(lit);
	}else if(std::holds_alternative<minLang::ast::call>(expr.value)){
		const auto& call = std::get<minLang::ast::call>(expr.value);
		//std::cout<<"Interpreting call: "<<call.name<<std::endl;
		return interpretCall(call);
	}else if(std::holds_alternative<minLang::ast::varName>(expr.value)){
		const auto& varName = std::get<minLang::ast::varName>(expr.value);
		return varEntries[varName.name];
	}else{
		std::cerr<<"Error: Unable to interpret blank expr!"<<std::endl;
		return {};
	}
}

std::vector<uint8_t> minLang::backends::interpreter::interpreter::interpretCall(const minLang::ast::call& call){
	if(!call.validatedDef){
		std::cerr<<"Interpreter Error: unable to call non validated function"<<std::endl;
		return {};
	}
	const minLang::ast::function& func = call.validatedDef->get();

	switch(func.status){
		case minLang::ast::function::positionStatus::normal:
			break;
		case minLang::ast::function::positionStatus::builtin:
			return handleBuiltinCall(call);
		case minLang::ast::function::positionStatus::external:
			return handleExternalCall(call);
	}

	for(unsigned int i=0;i<call.args.size();i++){
		const auto& arg = interpretExpr(call.args[i]);
		if(arg.size() != call.validatedDef->get().args[i].ty.getSize()){
			std::cerr<<"Interpreter error: got arg with incorrect size wrt type"<<std::endl;
			return {};
		}
		varEntries[func.args[i].name] = arg;
	}
	
	interpretBlock(func.body);

	//cleanup local vars after the function call is over
	for(unsigned int i=0;i<call.args.size();i++){
		varEntries.erase(func.args[i].name);
	}

	return returnBuffer;//change once the return function is implemented
}

void minLang::backends::interpreter::interpreter::interpretBlock(const minLang::ast::block& block){
	for(const auto& state : block.statements){
		interpretStatement(state);
	}
}


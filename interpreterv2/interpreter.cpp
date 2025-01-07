#include "interpreter.hpp"
#include <cstring>

bool interpreterv2::interpret(const ast::context& context, const std::string_view entryPoint, std::span<const std::string> linkLibs){
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

	if(!M.loadExternalFunctions(context.funcs, linkLibs))
		return false;
	
	ast::call mainCall{std::string(entryPoint), {}, *entry};
	M.interpretCall(mainCall);

	return true;
}


std::vector<uint8_t> getLiteralValue(const ast::literal& lit){
	if(std::holds_alternative<ast::literal::builtin_literal>(lit.value)){
		const auto& builtin = std::get<ast::literal::builtin_literal>(lit.value);
		if(std::holds_alternative<int>(builtin)){
			int val = std::get<int>(builtin);
			std::vector<uint8_t> output(sizeof(int));
			std::memcpy(output.data(), &val, sizeof(int));
			return output;
		}else if(std::holds_alternative<float>(builtin)){	
			float val = std::get<float>(builtin);
			std::vector<uint8_t> output(sizeof(float));
			std::memcpy(output.data(), &val, sizeof(float));
			return output;
		}else if(std::holds_alternative<bool>(builtin)){
			bool val = std::get<bool>(builtin);
			std::vector<uint8_t> output(sizeof(bool));
			std::memcpy(output.data(), &val, sizeof(bool));
			return output;
		}else{
			std::cerr<<"Error: got builtin literal with no value"<<std::endl;
			return {};
		}
	}else if(std::holds_alternative<ast::literal::array_literal>(lit.value)){
		const auto& array = std::get<ast::literal::array_literal>(lit.value);
		std::vector<uint8_t> output;
		for(const auto& elm : array.vals){
			const auto& val = getLiteralValue(elm);
			unsigned int oldOutputSize = output.size();
			output.resize(output.size() + val.size());
			std::memcpy(output.data() + oldOutputSize, val.data(), val.size());
		}
		return output;
	}else if(std::holds_alternative<ast::literal::tuple_literal>(lit.value)){
		const auto& tuple = std::get<ast::literal::tuple_literal>(lit.value);
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

bool interpreterv2::interpreter::interpretStatement(const ast::block::statement& state){
	if(std::holds_alternative<ast::block::declaration>(state)){
		const auto& decl = std::get<ast::block::declaration>(state);
		varEntries[decl.name] = std::vector<uint8_t>(decl.ty.getSize());
	}else if(std::holds_alternative<ast::block::assignment>(state)){
		const auto& assignment = std::get<ast::block::assignment>(state);
		varEntries[assignment.assignTo] = interpretExpr(assignment.assignFrom);
	}else if(std::holds_alternative<ast::expr>(state)){
		const auto& expr = std::get<ast::expr>(state);
		interpretExpr(expr);
	}else if(std::holds_alternative<ast::block::ifStatement>(state)){
		const auto& ifState = std::get<ast::block::ifStatement>(state);
		if(interpretExpr(ifState.condition).front() != 0){//blindly treat the value as a bool.  What could go wrong (should be already type checked, so should be fine)
			interpretBlock(*ifState.ifBody);
		}else{
			interpretBlock(*ifState.elseBody);
		}
	}else{
		std::cerr<<"Error: unable to interpret statement with no value"<<std::endl;
		return false;
	}
	return true;
}

std::vector<uint8_t> interpreterv2::interpreter::interpretExpr(const ast::expr& expr){
	if(std::holds_alternative<ast::literal>(expr.value)){
		const auto& lit = std::get<ast::literal>(expr.value);
		return getLiteralValue(lit);
	}else if(std::holds_alternative<ast::call>(expr.value)){
		const auto& call = std::get<ast::call>(expr.value);
		return interpretCall(call);
	}else if(std::holds_alternative<ast::varName>(expr.value)){
		const auto& varName = std::get<ast::varName>(expr.value);
		return varEntries[varName.name];
	}else{
		std::cerr<<"Error: Unable to interpret blank expr!"<<std::endl;
		return {};
	}
}

std::vector<uint8_t> interpreterv2::interpreter::interpretCall(const ast::call& call){
	const ast::function& func = call.validatedDef->get();

	switch(func.status){
		case ast::function::positionStatus::normal:
			break;
		case ast::function::positionStatus::builtin:
			return handleBuiltinCall(call);
		case ast::function::positionStatus::external:
			return handleExternalCall(call);
	}

	for(unsigned int i=0;i<call.args.size();i++){
		varEntries[func.args[i].name] = interpretExpr(call.args[i]);
	}
	
	interpretBlock(func.body);

	//cleanup local vars after the function call is over
	for(unsigned int i=0;i<call.args.size();i++){
		varEntries.erase(func.args[i].name);
	} 

	return {};//change once the return function is implemented
}

void interpreterv2::interpreter::interpretBlock(const ast::block& block){
	for(const auto& state : block.statements){
		interpretStatement(state);
	}
}


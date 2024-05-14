#include "interpreter.hpp"

namespace interpreter{
namespace impl{
	void print(int val){
		std::cout<<val;
	}
	void print(float val){
		std::cout<<val;
	}
	void print(bool val){
		std::cout<<(val ? "true" : "false");
	}

	void println(int val){
		std::cout<<val<<std::endl;
	}
	void println(float val){
		std::cout<<val<<std::endl;
	}
	void println(bool val){
		std::cout<<(val ? "true" : "false")<<std::endl;
	}

	void assign(int* dst, int src){
		*dst = src;
	}
	void assign(float* dst, float src){
		*dst = src;
	}
	void assign(bool* dst, bool src){
		*dst = src;
	}
}
}


void interpreter::handleBulitin(const ast::function& func, const ast::function::call& call, interpreter& M){

	auto functionMatches = [&](const std::string_view name, const std::vector<ast::type> CAT){
		if(func.name != name)
			return false;
		if(func.args.size() != CAT.size())
			return false;
		for(unsigned int i=0;i<CAT.size();i++){
			if(func.args[i].ty != CAT[i])
				return false;
		}
		return true;
	};

	auto getArg = [&]<typename T>(unsigned int argNum) -> T{
		if(std::holds_alternative<ast::function::call::varNameArg>(call.args[argNum])){
			auto carg = std::get<ast::function::call::varNameArg>(call.args[argNum]);
			T* arg = (T*) (M.stack.data() + (M.functionExecutions.back().variablePtrs[carg]));
			return *arg;
		}else if(std::holds_alternative<ast::literal>(call.args[argNum])){	
			auto& larg = std::get<ast::literal>(call.args[argNum]);
			//TODO: make this handle more than builtin types
			return std::get<T>(std::get<ast::literal::builtin_literal>(larg.value));
		}else{
			std::cerr<<"Unknown interpreter type error"<<std::endl;
			return T{};
		}
	};

	auto getArgPtr = [&]<typename T>(unsigned int argNum) -> T*{
		if(std::holds_alternative<ast::function::call::varNameArg>(call.args[argNum])){
			auto carg = std::get<ast::function::call::varNameArg>(call.args[argNum]);
			T* arg = (T*) (M.stack.data() + (M.functionExecutions.back().variablePtrs[carg]));
			return arg;
		}else{
			//TODO: make this a compiletime error, NOT RUNTIME!
			//probably add to functions check
			//realyl should be generalized though (const time?)
			std::cerr<<"Unknown interpreter type error"<<std::endl;
			return nullptr;
		}
	};

	if(functionMatches("print", {ast::type::int_type})){
		impl::print(getArg.operator()<int>(0));
	}else if(functionMatches("print", {ast::type::float_type})){
		impl::print(getArg.operator()<float>(0));
	}else if(functionMatches("print", {ast::type::bool_type})){
		impl::print(getArg.operator()<bool>(0));
	}else if(functionMatches("println", {ast::type::int_type})){
		impl::println(getArg.operator()<int>(0));
	}else if(functionMatches("println", {ast::type::float_type})){
		impl::println(getArg.operator()<float>(0));
	}else if(functionMatches("println", {ast::type::bool_type})){
		impl::println(getArg.operator()<bool>(0));
	}else if(functionMatches("assign", {ast::type::int_type, ast::type::int_type})){
		impl::assign(getArgPtr.operator()<int>(0), getArg.operator()<int>(1));
	}else if(functionMatches("assign", {ast::type::float_type, ast::type::float_type})){
		impl::assign(getArgPtr.operator()<float>(0), getArg.operator()<float>(1));
	}else if(functionMatches("assign", {ast::type::bool_type, ast::type::bool_type})){
		impl::assign(getArgPtr.operator()<bool>(0), getArg.operator()<bool>(1));
	}else{
		std::cerr<<"Error: Call to unknown builtin \""<<call.name<<"\""<<std::endl;
	}
}


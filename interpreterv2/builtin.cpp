#include "interpreter.hpp"
#include <cstring>

namespace interpreterv2{
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
}
}


std::vector<uint8_t> interpreterv2::interpreter::handleBuiltinCall(const ast::call& call){
	const auto& func = call.validatedDef->get();
	const auto functionMatches = [&](const std::string_view name, const std::vector<ast::type> CAT){
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

	const auto getArg = [&]<typename T>(unsigned int argNum) -> T{
		std::vector<uint8_t> val = interpretExpr(call.args[argNum]);
		T output;
		if(val.size() != sizeof(T)){
			std::cerr<<"Error: unmatched var size in builtin!"<<std::endl;
			return {};
		}
		std::memcpy(&output, val.data(), sizeof(T));
		return output;
	};

	if(functionMatches("print", {ast::type::int_type})){
		impl::print(getArg.operator()<int>(0));
		return {};
	}else if(functionMatches("print", {ast::type::float_type})){
		impl::print(getArg.operator()<float>(0));
		return {};
	}else if(functionMatches("print", {ast::type::bool_type})){
		impl::print(getArg.operator()<bool>(0));
		return {};
	}else if(functionMatches("println", {ast::type::int_type})){
		impl::println(getArg.operator()<int>(0));
		return {};
	}else if(functionMatches("println", {ast::type::float_type})){
		impl::println(getArg.operator()<float>(0));
		return {};
	}else if(functionMatches("println", {ast::type::bool_type})){
		impl::println(getArg.operator()<bool>(0));
		return {};
	}else{
		std::cerr<<"Error: Call to unknown builtin \""<<call.name<<"\""<<std::endl;
		return {};
	}
}


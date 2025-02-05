#include "interpreter.hpp"
#include <cstring>
#include "../hashCombine.hpp"
#include <unordered_map>
#include <tuple>
#include <cmath>

namespace interpreterv2{
namespace impl{
	template<typename T> void print(T val){
		std::cout<<+val;
	}
	void print_bool(bool val){
		std::cout<<(val ? "true" : "false");
	}
	template<typename T> void println(T val){
		std::cout<<+val<<std::endl;
	}
	void println_bool(bool val){
		std::cout<<(val ? "true" : "false")<<std::endl;
	}
	template<typename T> T add(T val1, T val2){
		return val1 + val2;
	}
	template<typename T> T sub(T val1, T val2){
		return val1 - val2;
	}
	template<typename T> T mul(T val1, T val2){
		return val1 * val2;
	}
	template<typename T> T div(T val1, T val2){
		return val1 / val2;
	}
	template<typename T> T mod(T val1, T val2){
		return val1 % val2;
	}
	template<> float mod<float>(float val1, float val2){
		return std::fmod(val1, val2);
	}
	template<> double mod<double>(double val1, double val2){
		return std::fmod(val1, val2);
	}
	template<typename T> bool greater(T val1, T val2){
		return val1 > val2;
	}
	template<typename T> bool less(T val1, T val2){
		return val1 < val2;
	}
	template<typename T> bool greaterOrEqual(T val1, T val2){
		return val1 >= val2;
	}
	template<typename T> bool lessOrEqual(T val1, T val2){
		return val1 <= val2;
	}
	template<typename T> bool equal(T val1, T val2){
		return val1 == val2;
	}
	bool and_bool(bool val1, bool val2){
		return val1 && val2;
	}
	bool or_bool(bool val1, bool val2){
		return val1 || val2;
	}
	bool not_bool(bool val){
		return !val;
	}
}
}

template<typename Sig>struct signature;

template<typename R, typename ...Args>struct signature<R(Args...)>{
    using type = std::tuple<Args...>;
};
template<typename F> concept is_fun = std::is_function_v<F>;
template<is_fun F> auto arguments(const F&) -> typename signature<F>::type;

template<int N, typename... Ts> using NthTypeOf = typename std::tuple_element<N, std::tuple<Ts...>>::type;


struct builtinFuncDetail{
	std::string name;
	ast::type ty;
	std::vector<ast::type> argTypes;
};
bool operator==(const builtinFuncDetail& b1, const builtinFuncDetail& b2){
	return b1.name == b2.name && b1.ty == b2.ty && b1.argTypes == b2.argTypes;
}
struct builtinFuncDetail_hasher{
	size_t operator()(const builtinFuncDetail& p) const{
		std::size_t baseHash = hashing::hashValues(p.name, COMPILE_TIME_CRC32_STR("ty"), p.ty.hash(), COMPILE_TIME_CRC32_STR("arg"));
		for(const auto& argTy : p.argTypes)
			baseHash = hashing::hashValues(baseHash, argTy.hash());
		return baseHash;
	}
};

struct builtinFuncLibrary{
	struct funcWrapper_base{
		virtual std::vector<uint8_t> call(const std::vector<std::vector<uint8_t>>& args) = 0;
		template<typename T> static const T& convertSimpleArg(const std::vector<uint8_t>& arg){
			return *((T*)arg.data());
		}
		template<typename T> static std::vector<uint8_t> convertSimpleArgBack(const T& arg){
			std::vector<uint8_t> output(sizeof(T));
			*((T*)output.data()) = arg;
			return output;
		}
		template <typename... Result, std::size_t... Indices> static auto vec_to_tup_helper(const std::vector<std::vector<uint8_t>>& args, std::index_sequence<Indices...>) {
		    return std::make_tuple(
			    convertSimpleArg<NthTypeOf<Indices, Result...>>(args[Indices])...
		    );
		}
		template<typename... Types> struct empty_vpack{
			empty_vpack(const std::tuple<Types...>& tp){}
			empty_vpack() = delete;
		};
		template <typename ...Result> std::tuple<Result...> vec_to_tup(const std::vector<std::vector<uint8_t>>& args, empty_vpack<Result...>)
		{
		    return vec_to_tup_helper<Result...>(args, std::make_index_sequence<sizeof...(Result)>());
		}
	};
	template<is_fun func> struct funcWrapper : public funcWrapper_base{
		std::conditional_t<std::is_function<func>::value, std::add_pointer_t<func>, func> f;
		funcWrapper(const func& _f) : f(_f){}
		using funcArgsTuple = std::conditional_t<std::is_function<func>::value, decltype(arguments(*f)), decltype(arguments(*f))>;
		std::vector<uint8_t> call(const std::vector<std::vector<uint8_t>>& args){
			if(std::tuple_size<funcArgsTuple>::value != args.size()){
				std::cout<<"Error: wrong number of func args for builtin function call!"<<std::endl;
				return {};
			}
			funcArgsTuple argsTuple = vec_to_tup(args, empty_vpack(funcArgsTuple{}));
			if constexpr(std::is_same_v<decltype(std::apply(f, argsTuple)), void>){
				std::apply(f, argsTuple);
				return {};
			}else{
				auto res = std::apply(f, argsTuple);
				return convertSimpleArgBack(res);
			}
		}
	};
	template<typename F> static std::shared_ptr<funcWrapper_base> wrapFunc(const F& func){
		return std::make_shared<funcWrapper<F>>(func);
	}
	std::unordered_map<builtinFuncDetail, std::shared_ptr<funcWrapper_base>, builtinFuncDetail_hasher> bulitinFuncs;
	bool containsCall(const ast::call& call) const{
		builtinFuncDetail funcDetail{call.name, call.validatedDef->get().ty};
		funcDetail.argTypes.resize(call.args.size());
		for(unsigned int i=0;i<call.args.size();i++)
			funcDetail.argTypes[i] = *call.args[i].inferType();
		return bulitinFuncs.contains(funcDetail);
	}
	std::vector<uint8_t> makeCall(const ast::call& call, const std::vector<std::vector<uint8_t>>& args){
		builtinFuncDetail funcDetail{call.name, call.validatedDef->get().ty};
		funcDetail.argTypes.resize(call.args.size());
		for(unsigned int i=0;i<call.args.size();i++)
			funcDetail.argTypes[i] = *call.args[i].inferType();
		auto& funcWrapper = bulitinFuncs.at(funcDetail);

		return funcWrapper->call(args);
	}
};

builtinFuncLibrary builtinLibrary;

template<typename T> void addBasicPrintBuiltins(const ast::type& ty){
	builtinLibrary.bulitinFuncs[{"print", ast::type::void_type, {ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::print<T>);
	builtinLibrary.bulitinFuncs[{"println", ast::type::void_type, {ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::println<T>);
}

template<typename T> void addBasicArithmaticBuiltins(const ast::type& ty){
	builtinLibrary.bulitinFuncs[{"add", ty, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::add<T>);
	builtinLibrary.bulitinFuncs[{"mul", ty, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::mul<T>);
	builtinLibrary.bulitinFuncs[{"sub", ty, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::sub<T>);
	builtinLibrary.bulitinFuncs[{"div", ty, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::div<T>);
	builtinLibrary.bulitinFuncs[{"mod", ty, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::mod<T>);
}

template<typename T> void addBasicLogicBuiltins(const ast::type& ty){
	builtinLibrary.bulitinFuncs[{"greater", ast::type::bool_type, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::greater<int>);
	builtinLibrary.bulitinFuncs[{"less", ast::type::bool_type, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::less<int>);
	builtinLibrary.bulitinFuncs[{"greaterOrEqual", ast::type::bool_type, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::greaterOrEqual<int>);
	builtinLibrary.bulitinFuncs[{"lessOrEqual", ast::type::bool_type, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::lessOrEqual<int>);
	builtinLibrary.bulitinFuncs[{"equal", ast::type::bool_type, {ty, ty}}] = builtinFuncLibrary::wrapFunc(interpreterv2::impl::equal<int>);
}

void interpreterv2::interpreter::initializeBuiltinLibrary(){
	builtinLibrary.bulitinFuncs.clear();//make sure it's empty as to prevent double filling (theoretical)
	
	//prints
	addBasicPrintBuiltins<int8_t>(ast::type::int8_type);
	addBasicPrintBuiltins<int16_t>(ast::type::int16_type);
	addBasicPrintBuiltins<int32_t>(ast::type::int32_type);
	addBasicPrintBuiltins<int64_t>(ast::type::int64_type);

	addBasicPrintBuiltins<uint8_t>(ast::type::uint8_type);
	addBasicPrintBuiltins<uint16_t>(ast::type::uint16_type);
	addBasicPrintBuiltins<uint32_t>(ast::type::uint32_type);
	addBasicPrintBuiltins<uint64_t>(ast::type::uint64_type);

	addBasicPrintBuiltins<float>(ast::type::float32_type);
	addBasicPrintBuiltins<double>(ast::type::float64_type);

	builtinLibrary.bulitinFuncs[{"print", ast::type::void_type, {ast::type::bool_type}}] = builtinFuncLibrary::wrapFunc(impl::print_bool);
	builtinLibrary.bulitinFuncs[{"println", ast::type::void_type, {ast::type::bool_type}}] = builtinFuncLibrary::wrapFunc(impl::println_bool);

	//arithmatic
	addBasicArithmaticBuiltins<int8_t>(ast::type::int8_type);
	addBasicArithmaticBuiltins<int16_t>(ast::type::int16_type);
	addBasicArithmaticBuiltins<int32_t>(ast::type::int32_type);
	addBasicArithmaticBuiltins<int64_t>(ast::type::int64_type);

	addBasicArithmaticBuiltins<uint8_t>(ast::type::uint8_type);
	addBasicArithmaticBuiltins<uint16_t>(ast::type::uint16_type);
	addBasicArithmaticBuiltins<uint32_t>(ast::type::uint32_type);
	addBasicArithmaticBuiltins<uint64_t>(ast::type::uint64_type);

	addBasicArithmaticBuiltins<float>(ast::type::float32_type);
	addBasicArithmaticBuiltins<double>(ast::type::float64_type);

	//logic
	addBasicLogicBuiltins<int8_t>(ast::type::int8_type);
	addBasicLogicBuiltins<int16_t>(ast::type::int16_type);
	addBasicLogicBuiltins<int32_t>(ast::type::int32_type);
	addBasicLogicBuiltins<int64_t>(ast::type::int64_type);

	addBasicLogicBuiltins<uint8_t>(ast::type::uint8_type);
	addBasicLogicBuiltins<uint16_t>(ast::type::uint16_type);
	addBasicLogicBuiltins<uint32_t>(ast::type::uint32_type);
	addBasicLogicBuiltins<uint64_t>(ast::type::uint64_type);

	addBasicLogicBuiltins<float>(ast::type::float32_type);
	addBasicLogicBuiltins<double>(ast::type::float64_type);

	builtinLibrary.bulitinFuncs[{"equal", ast::type::bool_type, {ast::type::bool_type, ast::type::bool_type}}] = builtinFuncLibrary::wrapFunc(impl::equal<bool>);
	builtinLibrary.bulitinFuncs[{"and", ast::type::bool_type, {ast::type::bool_type, ast::type::bool_type}}] = builtinFuncLibrary::wrapFunc(impl::and_bool);
	builtinLibrary.bulitinFuncs[{"or", ast::type::bool_type, {ast::type::bool_type, ast::type::bool_type}}] = builtinFuncLibrary::wrapFunc(impl::or_bool);
	builtinLibrary.bulitinFuncs[{"not", ast::type::bool_type, {ast::type::bool_type}}] = builtinFuncLibrary::wrapFunc(impl::not_bool);
}

std::vector<uint8_t> interpreterv2::interpreter::handleBuiltinCall(const ast::call& call){
	//const auto& func = call.validatedDef->get();

	if(!(builtinLibrary.containsCall(call) || call.name == ("get") || call.name == ("set"))){
		std::cerr<<"Error: Call to unknown builtin \""<<call.name<<"\""<<std::endl;
		return {};
	}

	std::vector<std::vector<uint8_t>> callArgs(call.args.size());
	for(unsigned int i=0;i<callArgs.size();i++){
		callArgs[i] = interpretExpr(call.args[i]);
	}
	
	if(call.name == ("get")){
		const auto& callTy = call.validatedDef->get().ty;
		
		if(!std::holds_alternative<ast::type::array_type>(call.args[0].inferType()->ty)){
			std::cout<<"Error: call to unknown get function builtin (type is not an array, but"<<callTy.toString()<<")"<<std::endl;
			return {};
		}
		
		const auto& arrTySize = callTy.getSize();
		const int& arrIndex = *((int*)callArgs[1].data());
		std::vector<uint8_t> output(arrTySize);
		std::memcpy(output.data(), callArgs[0].data()+(arrTySize*arrIndex), arrTySize);
		//std::cout<<"Getting "<<arrIndex<<"th element of array with size "<<callArgs[0].size()<<" bytes"<<std::endl;
		return output;
	}else if(call.name == ("set")){
		const auto& callTy = call.validatedDef->get().ty;
		
		if(!std::holds_alternative<ast::type::array_type>(call.args[0].inferType()->ty)){
			std::cout<<"Error: call to unknown get function builtin (type is not an array, but "<<callTy.toString()<<")"<<std::endl;
			return {};
		}

		//func order: array (0), val (1), index (2)
		
		const auto& arrTySize = call.validatedDef->get().args[1].ty.getSize();
		const int& arrIndex = *((int*)callArgs[2].data());
		const auto& value = callArgs[1];
		std::vector<uint8_t> output = callArgs[0];
		std::memcpy(output.data()+(arrTySize*arrIndex), value.data(), std::min((std::size_t)arrTySize, value.size()));
		//std::cout<<"Setting "<<arrIndex<<"th element of array with size "<<callArgs[0].size()<<" bytes"<<std::endl;
		return output;
	}else
		return builtinLibrary.makeCall(call, callArgs);
	

	/*
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
	*/
}


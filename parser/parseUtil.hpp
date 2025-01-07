#pragma once
#include <vector>
#include <string_view>
#include <optional>

template<class T, class Arr> bool isInList(const T& val, const Arr& arr){
	for(const auto& o : arr)
		if(o == val)
			return true;
	return false;
}

template<class T, class Arr> std::optional<unsigned int> findInList(const T& val, const Arr& arr){
	unsigned int i=0;
	for(const auto& o : arr){
		if(o == val)
			return i;
		i++;
	}
	return std::nullopt;
}

std::vector<std::string_view> strSplit(std::string_view s, std::string_view delimiter);

namespace PARSE_INTERNAL__{
	template<class T> struct parseRes{
		T val;
		int toksConsumed;
		bool encounteredError = false; //used if an error can be potentially recovered from, but it should be propegated that an error occurred
	};
}
template<class T> using parseRes = std::optional<PARSE_INTERNAL__::parseRes<T>>;
template<class T> constexpr parseRes<T> makeParseRes(const T& val, int consumed){
	return PARSE_INTERNAL__::parseRes<T>{val, consumed};
}
template<class T> constexpr parseRes<T> makeParseRes(const T& val, int consumed, bool encounteredError){
	return PARSE_INTERNAL__::parseRes<T>{val, consumed, encounteredError};
}

inline bool doDebugParse = false;

#ifndef DEBUG_PARSE_DISABLED
#include <iostream>
inline void parse_debug_print(const std::string_view str){
	if(doDebugParse)
		std::cout<<"parse debug:  "<<str<<std::endl;
}
#else
constexpr void parse_debug_print(const std::string_view str){
}
#endif

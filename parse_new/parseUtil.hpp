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
#define DEBUG_PARSE
#ifdef DEBUG_PARSE
#include <iostream>
inline void parse_debug_print(const std::string_view str){
	std::cout<<"parse debug:  "<<str<<std::endl;
}
#else
constexpr void parse_debug_print(const std::string_view str){
}
#endif

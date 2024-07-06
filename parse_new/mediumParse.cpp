#include "mediumParse.hpp"
#include "parseUtil.hpp"
#include <iostream>

void mediumToken::print() const{
	if(auto* v = std::get_if<mediumToken::stringToken>(&value)){
		std::cout<<"string: \""<<v->value<<"\"";
	}else if(auto* v = std::get_if<mediumToken::intToken>(&value)){
		std::cout<<"int: "<<v->value;
	}else if(auto* v = std::get_if<mediumToken::uintToken>(&value)){	
		std::cout<<"uint: "<<v->value;
	}else if(auto* v = std::get_if<mediumToken::doubleToken>(&value)){	
		std::cout<<"double: "<<v->value;
	}else if(auto* v = std::get_if<mediumToken::floatToken>(&value)){	
		std::cout<<"float: "<<v->value;
	}else if(auto* v = std::get_if<mediumToken::boolToken>(&value)){	
		std::cout<<"bool: "<<(v->value ? "true" : "false");
	}else if(auto* v = std::get_if<std::string>(&value)){
		std::cout<<*v;
	}
	std::cout<<" @ ";
	file.print();
}

bool isBool(const std::string_view str){
	return (str == "true" || str == "false");
}

bool isInt(const std::string_view str){
	for(char c : str){
		if(!isdigit(c))
			return false;
	}
	return true;
}

bool isUint(const std::string_view str){
	return (str.back() == 'u' && isInt(str.substr(0, str.size()-1)));
}

bool isDouble(const std::string_view str){
	auto parts = strSplit(str, ".");
	if(parts.size() != 2)
		return false;
	return (isInt(parts[0]) && isInt(parts[1]));
}

bool isFloat(const std::string_view str){
	return (str.back() == 'f' && isDouble(str.substr(0, str.size()-1)));
}

std::optional<std::vector<mediumToken>> parseBasicLiterals(const std::vector<basicToken>& toks){
	std::vector<mediumToken> output;
	for(unsigned int i=0;i<toks.size();i++){
		if(isBool(toks[i].val))
			output.push_back(mediumToken{mediumToken::boolToken{toks[i].val == "true"}, toks[i].file});
		else if(isInt(toks[i].val))
			output.push_back(mediumToken{mediumToken::intToken{strtoll(toks[i].val.c_str(), nullptr, 10)}, toks[i].file});
		else if(isUint(toks[i].val))
			output.push_back(mediumToken{mediumToken::uintToken{strtoull(toks[i].val.c_str(), nullptr, 10)}, toks[i].file});
		else if(isDouble(toks[i].val))
			output.push_back(mediumToken{mediumToken::doubleToken{std::stod(toks[i].val)}, toks[i].file});
		else if(isFloat(toks[i].val))
			output.push_back(mediumToken{mediumToken::floatToken{std::stof(toks[i].val)}, toks[i].file});
		else{
			
		}
	}

	return output;
}


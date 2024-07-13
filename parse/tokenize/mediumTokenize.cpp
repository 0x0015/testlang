#include "mediumTokenize.hpp"
#include <iostream>
#include "../parseUtil.hpp"

std::string_view mediumToken::tokList::type_t_to_string(const type_t& ty){
	switch(ty){
		case PAREN:
			return "PAREN";
		case CURL_BRACK:
			return "CURL_BRACK";
		case SQUARE_BRACK:
			return "SQUARE_BRACK";
	}
}

void mediumToken::print() const{
	currentRecur = -1;
	print_internal();
}

void mediumToken::print_internal() const{
	currentRecur++;

	if(std::holds_alternative<mediumToken::tokList>(value)){
		const auto& list = std::get<mediumToken::tokList>(value);
		for(unsigned int i=0;i<currentRecur;i++)
			std::cout<<"\t";
		std::cout<<"Token list: "<<mediumToken::tokList::type_t_to_string(list.type);
		if(!list.value.empty())
			std::cout<<std::endl;
		for(unsigned int i=0;i<list.value.size();i++){
			list.value[i].print_internal();
			if(i+1<list.value.size())
				std::cout<<std::endl;
		}
	}else if(std::holds_alternative<basicToken>(value)){
		for(unsigned int i=0;i<currentRecur;i++)
			std::cout<<"\t";
		std::get<basicToken>(value).print();
	}else{
		std::cerr<<"Unknown variant error (mediumToken)"<<std::endl;
	}

	currentRecur--;
}

std::optional<mediumToken::tokList> parseUntilEndbrack(const std::vector<basicToken>& toks, unsigned int& i, char closing);

std::optional<mediumToken> parseMediumToken(const std::vector<basicToken>& toks, unsigned int& i){
	if(toks[i].val == "("){
		auto parseRes = parseUntilEndbrack(toks, i, ')');
		if(!parseRes)
			return std::nullopt;
		parseRes->type = mediumToken::tokList::PAREN;
		return mediumToken{*parseRes};
	}else if(toks[i].val == "{"){
		auto parseRes = parseUntilEndbrack(toks, i, '}');
		if(!parseRes)
			return std::nullopt;
		parseRes->type = mediumToken::tokList::CURL_BRACK;
		return mediumToken{*parseRes};
	}else if(toks[i].val == "["){
		auto parseRes = parseUntilEndbrack(toks, i, ']');
		if(!parseRes)
			return std::nullopt;
		parseRes->type = mediumToken::tokList::SQUARE_BRACK;
		return mediumToken{*parseRes};
	}else{
		return mediumToken{toks[i]};
	}
}

std::optional<mediumToken::tokList> parseUntilEndbrack(const std::vector<basicToken>& toks, unsigned int& i, char closing){
	i++;
	mediumToken::tokList output;
	bool foundClosing = false;
	for(;i<toks.size();i++){
		if(toks[i].val.size() == 1 && toks[i].val[0] == closing){
			foundClosing = true;
			break;
		}
		auto parsed = parseMediumToken(toks, i);
		if(parsed)
			output.value.push_back(*parsed);
	}

	if(!foundClosing){
		std::cout<<"Error: did not find closing \""<<closing<<"\"";
		return std::nullopt;
	}

	return output;
}

std::optional<std::vector<mediumToken>> parseBrackets(const std::vector<basicToken>& toks){
	std::vector<mediumToken> output;

	for(unsigned int i=0;i<toks.size();i++){
		auto parsed = parseMediumToken(toks, i);
		if(parsed)
			output.push_back(*parsed);
	}

	return output;
}


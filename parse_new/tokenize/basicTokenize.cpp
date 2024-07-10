#include "basicTokenize.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "../parseUtil.hpp"

void basicToken::print() const{
	std::cout<<val<<" @ ";
	file.print();
}

std::optional<std::string> readFile(const std::string_view filename){
	if(!std::filesystem::exists(filename)){
		std::cerr<<"Unable to open file \""<<filename<<"\""<<std::endl;
		return std::nullopt;
	}
	auto size = std::filesystem::file_size(filename);
	std::ifstream file;
	file.open(std::string(filename)); //no direct call, so cast to std::string
	std::string output;
	output.resize(size);
	file.read(output.data(), size);
	return output;
}

std::optional<std::vector<basicToken>> parseStringsAndComments(const std::string_view file){
	std::vector<basicToken> output;
	output.push_back({});
	output.back().file.fileLocation = 0;
	output.back().file.fileSize = 0;
	//ignore filename for now
	for(unsigned int i=0;i<file.size();i++){
		if(file[i] == '\"'){
			output.push_back({});
			output.back().file.fileLocation = i;
			output.back().file.fileSize = 0;

			output.back().val += file[i];
			output.back().file.fileSize++;
			i++;
			bool foundEnd = false;
			for(;i<file.size();i++){
				output.back().val += file[i];
				output.back().file.fileSize++;
				if(file[i] == '\"'){
					foundEnd = true;
					i++;
					break;
				}
			}
			if(!foundEnd){
				std::cerr<<"Did not find end of string literal"<<std::endl;
				return std::nullopt;
			}
			output.push_back({});
			output.back().file.fileLocation = i;
			output.back().file.fileSize = 0;
		}else if(i+1 < file.size() && file.substr(i, 2) == "//"){
			i+=2;
			bool foundEnd = false;
			for(;i<file.size();i++){
				if(file[i] == '\n'){
					foundEnd = true;
					i++;
					break;
				}
			}
			if(foundEnd){
				output.push_back({});
				output.back().file.fileLocation = i;
				output.back().file.fileSize = 0;
			}
		}else if(i+1 < file.size() && file.substr(i, 2) == "/*"){
			i+=2;
			bool foundEnd = false;
			for(;i<file.size();i++){
				if(i+1 < file.size() && file.substr(i, 2) == "*/"){
					foundEnd = true;
					i+=2;
					break;
				}
			}
			if(!foundEnd){
				std::cerr<<"Did not find end of bounded comment"<<std::endl;
				return std::nullopt;
			}
			output.push_back({});
			output.back().file.fileLocation = i;
			output.back().file.fileSize = 0;
		}
		output.back().val += file[i];
		output.back().file.fileSize++;
	}

	if(!output.empty() && output.back().val.empty())
		output.pop_back();

	return output;
}

std::optional<std::vector<basicToken>> basicTokenizeFile(const std::string_view filename){
	auto contents = readFile(filename);
	if(!contents)
		return std::nullopt;
	return basicTokenizeString(*contents, filename);
}

std::optional<std::vector<basicToken>> basicTokenizeString(const std::string_view contents, const std::string_view filename){
	auto commentAndStringed = parseStringsAndComments(contents);

	if(!commentAndStringed)
		return std::nullopt;
	

	std::vector<basicToken> output;

	for(auto& part : *commentAndStringed){
		if(part.val[0] == '\"'){
			part.file.filename = filename;
			output.push_back(part);
			continue;
		}

		for(auto& i : part.val){
			constexpr char whitespace[] = {' ', '\t', '\n', '\r'};
			if(isInList(i, whitespace))
				i = ' ';
		}

		auto& contents_val = part.val;
		for(unsigned int i=0;i<contents_val.size();i++){
			if(contents_val[i] != ' '){
				if(i == 0 || contents_val[i-1] == ' '){
					if(!(!output.empty() && output.back().val.size() == 0)){
						output.push_back({});
						output.back().file.filename = filename;
						output.back().file.fileLocation = i+part.file.fileLocation;
						output.back().file.fileSize = 0;
					}
				}
				constexpr char singleCharToks[] = {'(', ')', '{', '}', '[', ']', ';', ','};
				if(isInList(contents_val[i], singleCharToks)){
					if(output.back().val.size() == 0){
						output.back().val += contents_val[i];
						output.back().file.fileSize++;
						if(i+1 < contents_val.size()){
							output.push_back({});
							output.back().file.filename = filename;
							output.back().file.fileLocation = i+1+part.file.fileLocation;
							output.back().file.fileSize = 0;
						}
					}else{	
						output.push_back({});
						output.back().file.filename = filename;
						output.back().file.fileLocation = i+1+part.file.fileLocation;
						output.back().file.fileSize = 1;
						output.back().val += contents_val[i];
						if(i+1 < contents_val.size()){
						output.push_back({});
							output.back().file.filename = filename;
							output.back().file.fileLocation = i+1+part.file.fileLocation;
							output.back().file.fileSize = 0;
						}
					}
					continue;
				}
				output.back().val += contents_val[i];
				output.back().file.fileSize++;
			}
		}

		if(!output.empty() && output.back().val.empty())
			output.pop_back();

	}

	return output;
}


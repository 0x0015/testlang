#include "errorUtil.hpp"
#include <iostream>
#include "parseUtil.hpp"
#include "fileIO.hpp"

struct fileErrorDetails{
	int line;
	int column;
	std::string_view fullLine;
	std::optional<std::string_view> prevLine;
	std::optional<std::string_view> nextLine;
};

//we shall get guranteed that file.size() < errorLoc
fileErrorDetails getFileErrorDetails(const std::string_view file, int errorLoc){
	int line = 1;
	int column = 1;
	for(unsigned int i=0;i<errorLoc;i++){
		if(file[i] == '\n'){
			line++;
			column = 1;
		}else{
			column++;
		}
	}
	auto lines = strSplit(file, "\n");
	auto output = fileErrorDetails{line, column, lines[line-1]};
	if(line-1 > 0)
		output.prevLine = lines[line-2];
	if(line-1 < lines.size())
		output.nextLine = lines[line];
	return output;
}

void printLine(int lineNum, const std::string_view line){
	constexpr int tabIndent = 8;
	std::string lineStr = std::to_string(lineNum);
	std::string extraIndent(lineStr.size() < 8 ? tabIndent-lineStr.size()-1 : 0, ' ');
	std::cout<<lineStr<<extraIndent<<"|"<<line<<std::endl;
}

void printErrorFileSpot(const basicToken& tok){
	auto file = readFile(tok.file.filename);
	if(!file){
		std::cout<<"Error occurred at (unable to find) "<<tok.file.filename<<" at char "<<tok.file.fileLocation<<std::endl;
		return;
	}
	if(tok.file.fileLocation >= file->size()){
		std::cerr<<"Error occurred past end of file? (at "<<tok.file.fileLocation<<" of "<<tok.file.fileSize<<" while current size is "<<file->size()<<")"<<std::endl;
		return;
	}
	auto details = getFileErrorDetails(*file, tok.file.fileLocation);
	std::cout<<"Error occurred at "<<tok.file.filename<<" at line "<<details.line<<" and column "<<details.column<<std::endl;
	if(details.prevLine)
		printLine(details.line-1, *details.prevLine);
	printLine(details.line, details.fullLine);
	if(details.nextLine)
		printLine(details.line+1, *details.nextLine);
}

void printErrorFileSpot(const mediumToken& tok){
	if(std::holds_alternative<basicToken>(tok.value)){
		printErrorFileSpot(std::get<basicToken>(tok.value));
	}else if(std::holds_alternative<mediumToken::tokList>(tok.value)){
		const auto& tList = std::get<mediumToken::tokList>(tok.value);
		if(tList.value.empty())
			return;
		printErrorFileSpot(tList.value.front());
	}else{
		std::cerr<<"Error: Trying to print error file location of null token"<<std::endl;
	}
}


#include <iostream>
#include "parse/parse.hpp"
#include "checks/functionChecker.hpp"
#include "checks/typeChecker.hpp"
#include "builtins/builtins.hpp"

#include "interpreter/interpreter.hpp"

int main(int argc, char** argv){
	parser parse;
	if(argc == 2){	
		parse.files.push_back(std::string(argv[1]));
	}else{
		parse.files.push_back("simpleTest.txt");
	}
	//parse.files.push_back("test.txt");

	auto parseRes = parse.parseAll();
	if(parseRes) addBuiltins(*parseRes);

	bool errored = false;
	
	if(!parseRes) errored = true;
	errored = errored || !checkFunctionsDefined(*parseRes);
	errored = errored || !checkConflictingFunctionDefinitions(*parseRes);
	errored = errored || !checkTypeUsesValid(*parseRes);

	parseRes->dump();
	if(errored){
		std::cout<<"An error has occurred.  Aborting."<<std::endl;
		return -1;
	}

	interpreter::interpret(*parseRes, "main");
	/*

	auto val = result.value();
	addBuiltins(val);


	if(!errored){
		val.dump();
		interpreter::interpret(val, "main");
	}
	

	return errored;
	*/
}

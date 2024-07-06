#include <iostream>
#include "parse_new/parse.hpp"
#include "functionChecker.hpp"
#include "typeChecker.hpp"
#include "builtins/builtins.hpp"

#include "interpreter/interpreter.hpp"

int main(){
	parser parse;
	parse.files.push_back("simpleTest.txt");
	//parse.files.push_back("test.txt");

	auto parseRes = parse.parseAll();
	if(parseRes) addBuiltins(*parseRes);

	bool errored = false;
	
	if(!parseRes) errored = true;
	errored = errored || !checkFunctionsDefined(*parseRes);
	errored = errored || !checkConflictingFunctionDefinitions(*parseRes);
	errored = errored || !checkTypeUsesValid(*parseRes);

	if(errored){
		std::cout<<"An error has occurred.  Aborting."<<std::endl;
		return -1;
	}

	parseRes->dump();
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

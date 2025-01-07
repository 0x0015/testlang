#include <iostream>
#include "parser/parse.hpp"
#include "checks/functionChecker.hpp"
#include "checks/typeChecker.hpp"
#include "builtins/builtins.hpp"
#include "parser/parseUtil.hpp"
#include "argParse.hpp"
#include "interpreter/interpreter.hpp"
#include "interpreterv2/interpreter.hpp"

int main(int argc, char** argv){
	auto args = argVals::parse(argc, argv);
	if(!args)
		return 0;

	parser parse;
	parse.files.push_back(args->input);
	bool verbose = args->verbose;
	doDebugParse = verbose;
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
	if(verbose)
		parseRes->dump();

	if(args->oldInterpreter){
		//old is faster but less featured (may catch up slowly)
		interpreter::interpret(*parseRes, "main", args->links);
	}else{
		//new is slower, but should be more fleshed out
		interpreterv2::interpret(*parseRes, "main", args->links);
	}
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

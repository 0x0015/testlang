#include <iostream>
#include "parser/parse.hpp"
#include "checks/functionChecker.hpp"
#include "checks/typeChecker.hpp"
#include "builtins/builtins.hpp"
#include "parser/parseUtil.hpp"
#include "argParse.hpp"
#include "minLangConvert/minLangConvert.hpp"
#include "minLang/backends/cCodeGen/cCodeGen.hpp"
#include "minLang/backends/interpreter/interpreter.hpp"

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
	//errored = errored || !checkFunctionsDefined(*parseRes); //problematic and not needed (as this is checked when types are matched in checkTypeUsesValid)
	errored = errored || !checkConflictingFunctionDefinitions(*parseRes);
	errored = errored || !checkTypeUsesValid(*parseRes);

	if(errored){
		std::cout<<"An error has occurred.  Aborting."<<std::endl;
		return -1;
	}
	if(verbose)
		parseRes->dump();

	const auto& minLangContext = minLangConvert(*parseRes);
	if(!minLangContext){
		std::cout<<"An error occurred while downconverting to minLang.  Aborting."<<std::endl;
		return -1;
	}

	if(args->interpreter){
		//new is slower, but should be more fleshed out
		minLang::backends::interpreter::interpret(*minLangContext, "main", args->links);
	}else{
		auto code = minLang::backends::cCodeGen::genCCode(*minLangContext, "main");
		if(args->printCCode){
			std::cout<<code<<std::endl;
		}else{
			std::string outputFilename = args->outputFn.empty() ? "a.out" : args->outputFn;
			minLang::backends::cCodeGen::compileCCode(code, outputFilename, args->links, args->forceCCompiler);
		}
	}

	return 0;
}

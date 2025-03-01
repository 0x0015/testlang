#include "argParse.hpp"
#include <iostream>

std::optional<minLang::argVals> minLang::argVals::parse(int argc, char** argv){
	argVals output{};

	if(argc == 1){
		std::cout<<"No command line arguments given, quitting. (use \"-h\" or \"--help\" for help)"<<std::endl;
		return std::nullopt;
	}

	for(int i=1;i<argc;i++){
		auto arg = std::string_view(argv[i]);
		if(arg == "-h" || arg == "--help"){
			std::cout<<"this is the help screen!"<<std::endl;
			std::cout<<"\t-v | --verbose for verbose"<<std::endl;
			std::cout<<"\t-l{lib} for linking to a library"<<std::endl;
			std::cout<<"\t-i for the interpreter"<<std::endl;
			std::cout<<"\t-o {output filename} to specify the output filename"<<std::endl;
			std::cout<<"\t-C_dump to print the cCodeGen generated C code and exit"<<std::endl;
			std::cout<<"\t-C_use_compiler {compiler command} to force use of a given C compiler"<<std::endl;
			return std::nullopt;
		};
		if(arg == "-v" || arg == "--verbose"){
			output.verbose = true;
			continue;
		}
		if(arg.size() > 2 && arg.substr(0, 2) == "-l"){
			output.links.push_back(std::string(arg.substr(2)));
			continue;
		}
		if(arg == "-i"){
			output.interpreter = true;
			continue;
		}
		if(arg == "-o"){
			if(i+1<argc){
				output.outputFn = std::string_view(argv[i+1]);
				i++;
				continue;
			}else{
				std::cerr<<"Argument error: output argument \"-o\" used with no output file supplied"<<std::endl;
				return std::nullopt;
			}
		}
		if(arg == "-C_dump"){
			output.printCCode = true;
			continue;
		}
		if(arg == "-C_use_compiler"){
			if(i+1<argc){
				output.forceCCompiler = std::string_view(argv[i+1]);
				i++;
				continue;
			}else{
				std::cerr<<"Argument error: force C compiler argument \"-C_use_compiler\" used with no compiler supplied"<<std::endl;
				return std::nullopt;
			}
		}
		if(output.input.empty() && arg.size() > 3 && arg[0] != '-'){
			output.input = arg;
			continue;
		}

		std::cerr<<"Unknown command line argument: \""<<arg<<"\""<<std::endl;
		std::cout<<"Exiting."<<std::endl;
		return std::nullopt;
	}

	//sanity checks
	if(output.input.empty()){
		std::cerr<<"Argument error: no file input supplied"<<std::endl;
		return std::nullopt;
	}
	if(output.interpreter && (!output.outputFn.empty() || output.printCCode)){
		std::cerr<<"Argument error: interpreter and cCodeGen are mutually exclusive (must choose one)"<<std::endl;
		return std::nullopt;
	}

	return output;
}


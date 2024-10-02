#include "argParse.hpp"
#include <iostream>

std::optional<argVals> argVals::parse(int argc, char** argv){
	argVals output{};

	if(argc == 1){
		std::cout<<"No command line arguments given, quitting."<<std::endl;
		return std::nullopt;
	}

	for(int i=1;i<argc;i++){
		auto arg = std::string_view(argv[i]);
		if(arg == "-h" || arg == "--help"){
			std::cout<<"this is the help screen!"<<std::endl;
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
		if(arg == "-i_old"){
			output.oldInterpreter = true;
			continue;
		}
		if(output.input.empty() && arg.size() > 3 && arg[0] != '-'){
			output.input = arg;
			continue;
		}

		std::cerr<<"Unknown command line argument: \""<<arg<<"\""<<std::endl;
		std::cout<<"Exiting."<<std::endl;
	}

	return output;
}


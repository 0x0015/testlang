#pragma once
#include <vector>
#include <string>
#include <optional>

struct argVals{
	bool verbose = false;
	std::string input;
	std::vector<std::string> links;
	static std::optional<argVals> parse(int argc, char** argv);
	bool interpreter = false;
	bool printCCode = false;
	std::string outputFn;
	std::string forceCCompiler;
};


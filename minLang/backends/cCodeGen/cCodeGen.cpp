#include "cCodeGen.hpp"
#include <cstring>
#include <filesystem>
#include "../../hashCombine.hpp"
#include "../../parser/fileIO.hpp"

std::string minLang::backends::cCodeGen::genCCode(const minLang::ast::context& context, const std::string_view entryPoint){
	std::optional<std::reference_wrapper<const minLang::ast::function>> entry;
	for(const auto& func : context.funcs){
		if(func.name == entryPoint && func.ty == minLang::ast::type::void_type && func.args.size() == 0){
			entry = std::cref(func);
			break;
		}
	}
	if(!entry){
		std::cerr<<"Error: Unable to find entry point function \""<<entryPoint<<"\" (must be void->void)"<<std::endl;
		return "";
	}

	std::string code = "#include <stdio.h>\n#include<stdint.h>\n\n";
	auto types = findUsedTypes(*entry);
	auto funcs = findUsedFunctions(*entry);
	code += genUsedCTypes(types);
	code += genBuiltins();
	code += genTemplateFuncBuiltins(funcs, types);
	code += genUsedFunctionForwarddefs(funcs, types);
	code += genUsedFunctionDefs(funcs, types);

	//quick main stub
	code += "int main(int argc, char** argv){\n\t" + mangleFuncName(entry->get()) + "();\n\treturn(0);\n}\n";
	
	return code;
}

bool minLang::backends::cCodeGen::compileCCode(const std::string_view code, const std::string_view outputFilename, std::span<const std::string> linkLibs, const std::string& forceCompiler){
	std::string compiler;
	std::string compilerSpecificOpts;
	if(forceCompiler == "gcc" || (forceCompiler.empty() && system("gcc -v > /dev/null 2>&1") == 0)){
		compiler = "gcc";
		compilerSpecificOpts = "-O2 -ffunction-sections -fdata-sections -Wl,--gc-sections -flto -ffast-math"; //when compiling an executable we can cull unused sections
	}else if(forceCompiler == "clang" || (forceCompiler.empty() && system("clang -v > /dev/null 2>&1") == 0)){
		compiler = "clang";
		compilerSpecificOpts = "-O2 -ffunction-sections -fdata-sections -Wl,--gc-sections -flto -ffast-math"; //            ||                                  ||
	}else if(forceCompiler == "tcc" || (forceCompiler.empty() && system("tcc -v > /dev/null 2>&1") == 0)){
		compiler = "tcc";
		//no applicable compiler specific opts
	}else if(!forceCompiler.empty()){
		compiler = forceCompiler;
	}else{
		std::cerr<<"cCodeGen Error: unable to find an installed C complier to compile the generated code"<<std::endl;
		return false;
	}

	std::cout<<"cCodeGen Note: selected C compiler: "<<compiler<<std::endl;

	std::string filename = "cCodeGen_temp" + std::to_string(time(nullptr)) + ".c";
	while(std::filesystem::exists(filename)){
		filename = "cCodeGen_temp" + std::to_string(time(nullptr)) + ".c";//!!NEVER!! overwrite an existing file
	}

	writeFile(filename, code);

	std::string command = compiler + " -o " + std::string(outputFilename) + " " + compilerSpecificOpts;
	for(const auto& linkLib : linkLibs)
		command += " -l" + linkLib;
	command += " " + filename;

	std::cout<<"cCodeGen: running C compilation command: "<<command<<std::endl;

	if(system(command.c_str()) != 0){
		std::cerr<<"cCodeGen Error: C compilation command returned a non-zero error status.  Indicates problem with the compiler or internal code generation error."<<std::endl;
		std::cerr<<"\t Intermediate compilation C source file \""<<filename<<"\" will not be deleted for debugging purposes"<<std::endl;
		return false;
	}

	std::filesystem::remove(filename);

	return true;
}

std::string minLang::backends::cCodeGen::mangleName(const std::string& name){
	auto output = "cCodeGen_" + name.substr(0, 5) + std::to_string(hashing::hashValues(name));//for now just do this;
	std::replace_if(output.begin(), output.end(), [](const char c){return (c == '<' || c == '>');}, '_');
	return output;
}

std::string minLang::backends::cCodeGen::mangleFuncName(const minLang::ast::function& func){
	std::size_t hash = hashing::hashValues(func.name, COMPILE_TIME_CRC32_STR("ret_ty"), func.ty.hash(), COMPILE_TIME_CRC32_STR("args"));
	for(const auto& arg : func.args)
		hash = hashing::hashValues(hash, arg.ty.hash());
	auto output = "cCodeGen_" + func.name.substr(0, 5) + std::to_string(hash);//for now just do this;
	std::replace_if(output.begin(), output.end(), [](const char c){return (c == '<' || c == '>');}, '_');
	return output;
}

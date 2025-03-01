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

void findUsedFunctions_iter(const minLang::ast::function& funcToSearch, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::cCodeGen::funcSigHasher, minLang::backends::cCodeGen::funcSigComp>& foundFuncs);

void findUsedFunctions_iter_addExpr(const minLang::ast::expr& expr, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::cCodeGen::funcSigHasher, minLang::backends::cCodeGen::funcSigComp>& foundFuncs){
	if(std::holds_alternative<minLang::ast::call>(expr.value)){
		const auto& call = std::get<minLang::ast::call>(expr.value);
		if(!foundFuncs.contains(call.validatedDef->get())){
			std::cout<<"adding never found before func ("<<call.name<<"/"<<call.validatedDef->get().name<<") in traversal"<<std::endl;
			//foundFuncs.emplace(std::make_pair(call.name, *call.validatedDef));
			findUsedFunctions_iter(call.validatedDef->get(), foundFuncs);
		}else{
			/*
			const auto& matches = foundFuncs.equal_range(call.name);
			bool foundExactMatch = false;
			for(auto it = matches.first; it != matches.second; ++it){
				const auto& match = it->second.get();
				if(match.ty == call.validatedDef->get().ty && match.args.size() == call.validatedDef->get().args.size()){
					bool argsMatching = true;
					for(unsigned int i=0;i<match.args.size();i++){
						if(match.args[i].ty != call.validatedDef->get().args[i].ty){
							argsMatching = false;
							break;
						}
					}
					if(!argsMatching) //if there is not an exact match
						foundExactMatch = false;
				}
			}
			
			if(!foundExactMatch){
				findUsedFunctions_iter(call.validatedDef->get(), foundFuncs);
			}
			*/
		}
		for(const auto& arg : call.args)
			findUsedFunctions_iter_addExpr(arg, foundFuncs);
	}
}

void findUsedFunctions_iter(const minLang::ast::block& block, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::cCodeGen::funcSigHasher, minLang::backends::cCodeGen::funcSigComp>& foundFuncs){
	for(const auto& state : block.statements){
		if(std::holds_alternative<minLang::ast::expr>(state)){
			const auto& expr = std::get<minLang::ast::expr>(state);
			findUsedFunctions_iter_addExpr(expr, foundFuncs);
		}else if(std::holds_alternative<minLang::ast::block::assignment>(state)){
			const auto& asgn = std::get<minLang::ast::block::assignment>(state);
			findUsedFunctions_iter_addExpr(asgn.assignFrom, foundFuncs);
		}else if(std::holds_alternative<minLang::ast::block::ifStatement>(state)){
			const auto& ifStat = std::get<minLang::ast::block::ifStatement>(state);
			findUsedFunctions_iter_addExpr(ifStat.condition, foundFuncs);
			findUsedFunctions_iter(*ifStat.ifBody, foundFuncs);
			findUsedFunctions_iter(*ifStat.elseBody, foundFuncs);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_normal>(state)){
			const auto& forStat = std::get<minLang::ast::block::forStatement_normal>(state);
			findUsedFunctions_iter_addExpr(forStat.initialDecl.assignFrom, foundFuncs);
			findUsedFunctions_iter_addExpr(forStat.breakCond, foundFuncs);
			findUsedFunctions_iter_addExpr(forStat.perLoopAsgn.assignFrom, foundFuncs);
			findUsedFunctions_iter(*forStat.body, foundFuncs);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_while>(state)){
			const auto& forStat = std::get<minLang::ast::block::forStatement_while>(state);
			findUsedFunctions_iter_addExpr(forStat.condition, foundFuncs);
			findUsedFunctions_iter(*forStat.body, foundFuncs);
		}else if(std::holds_alternative<minLang::ast::block::returnStatement>(state)){
			const auto& retStat = std::get<minLang::ast::block::returnStatement>(state);
			findUsedFunctions_iter_addExpr(retStat.val, foundFuncs);
		}else{
			std::cerr<<"cCodeGen function traversal error: encountered block statement of unknown type (index "<<state.index()<<")"<<std::endl;
		}
	}
}

void findUsedFunctions_iter(const minLang::ast::function& funcToSearch, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::cCodeGen::funcSigHasher, minLang::backends::cCodeGen::funcSigComp>& foundFuncs){
	if(foundFuncs.contains(funcToSearch))
		return;
	std::cout<<"Traversing function: "<<funcToSearch.name<<std::endl;
	//foundFuncs.emplace(std::make_pair(funcToSearch.name, std::cref(funcToSearch))); //note: need cref to make sure the reference gets passed by address and not copied and then taking the addr of that local value (for some reason)
	foundFuncs.emplace(std::cref(funcToSearch));
	findUsedFunctions_iter(funcToSearch.body, foundFuncs);
}

std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::cCodeGen::funcSigHasher, minLang::backends::cCodeGen::funcSigComp> minLang::backends::cCodeGen::findUsedFunctions(std::reference_wrapper<const minLang::ast::function> entrypoint){
	std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::cCodeGen::funcSigHasher, minLang::backends::cCodeGen::funcSigComp> output;
	findUsedFunctions_iter(entrypoint, output);
	return output;
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

#include "util.hpp"

void findUsedFunctions_iter(const minLang::ast::function& funcToSearch, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& foundFuncs);

void findUsedFunctions_iter_addExpr(const minLang::ast::expr& expr, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& foundFuncs){
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

void findUsedFunctions_iter(const minLang::ast::block& block, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& foundFuncs){
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

void findUsedFunctions_iter(const minLang::ast::function& funcToSearch, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& foundFuncs){
	if(foundFuncs.contains(funcToSearch))
		return;
	std::cout<<"Traversing function: "<<funcToSearch.name<<std::endl;
	//foundFuncs.emplace(std::make_pair(funcToSearch.name, std::cref(funcToSearch))); //note: need cref to make sure the reference gets passed by address and not copied and then taking the addr of that local value (for some reason)
	foundFuncs.emplace(std::cref(funcToSearch));
	findUsedFunctions_iter(funcToSearch.body, foundFuncs);
}

std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp> minLang::backends::findUsedFunctions(const ast::function& entrypoint){
	std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp> output;
	findUsedFunctions_iter(entrypoint, output);
	return output;
}



void findUsedTypes_iter(const minLang::ast::block& block, std::unordered_set<minLang::ast::type, minLang::backends::typeHasher>& foundTypes, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& traversedFunctions);

void findUsedTypes_iter_addExpr(const minLang::ast::expr& expr, std::unordered_set<minLang::ast::type, minLang::backends::typeHasher>& foundTypes, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& traversedFunctions){
	if(std::holds_alternative<minLang::ast::literal>(expr.value)){
		const auto& lit = std::get<minLang::ast::literal>(expr.value);
		foundTypes.insert(lit.ty);
	}else if(std::holds_alternative<minLang::ast::call>(expr.value)){
		const auto& call = std::get<minLang::ast::call>(expr.value);
		foundTypes.insert(call.validatedDef->get().ty);
		for(const auto& arg : call.args)
			findUsedTypes_iter_addExpr(arg, foundTypes, traversedFunctions);
		if(!traversedFunctions.contains(call.validatedDef->get())){
			traversedFunctions.insert(call.validatedDef->get());
			findUsedTypes_iter(call.validatedDef->get().body, foundTypes, traversedFunctions);
		}
	}else if(std::holds_alternative<minLang::ast::varName>(expr.value)){
		const auto& varName = std::get<minLang::ast::varName>(expr.value);
		foundTypes.insert(*varName.matchedType);
	}else{
		std::cerr<<"Error: found unknown expr while searhcing for types"<<std::endl;
	}
}

void findUsedTypes_iter(const minLang::ast::block& block, std::unordered_set<minLang::ast::type, minLang::backends::typeHasher>& foundTypes, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& traversedFunctions){
	for(const auto& state : block.statements){
		if(std::holds_alternative<minLang::ast::expr>(state)){
			const auto& expr = std::get<minLang::ast::expr>(state);
			findUsedTypes_iter_addExpr(expr, foundTypes, traversedFunctions);
		}else if(std::holds_alternative<minLang::ast::block::declaration>(state)){
			const auto& decl = std::get<minLang::ast::block::declaration>(state);
			foundTypes.insert(decl.ty);
		}else if(std::holds_alternative<minLang::ast::block::assignment>(state)){
			//can be ignored as we're (hopefully) only assigning to variable we've already defined (and thus we already know the type)
		}else if(std::holds_alternative<minLang::ast::block::ifStatement>(state)){
			const auto& ifStat = std::get<minLang::ast::block::ifStatement>(state);
			findUsedTypes_iter_addExpr(ifStat.condition, foundTypes, traversedFunctions);
			findUsedTypes_iter(*ifStat.ifBody, foundTypes, traversedFunctions);
			findUsedTypes_iter(*ifStat.elseBody, foundTypes, traversedFunctions);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_while>(state)){
			const auto& whileStat = std::get<minLang::ast::block::forStatement_while>(state);
			findUsedTypes_iter_addExpr(whileStat.condition, foundTypes, traversedFunctions);
			findUsedTypes_iter(*whileStat.body, foundTypes, traversedFunctions);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_normal>(state)){
			const auto& forStat = std::get<minLang::ast::block::forStatement_normal>(state);
			findUsedTypes_iter_addExpr(forStat.breakCond, foundTypes, traversedFunctions);
			findUsedTypes_iter(*forStat.body, foundTypes, traversedFunctions);
		}else{
			std::cerr<<"Error: found unknown statement while searching for types in block"<<std::endl;
		}
	}
}

std::unordered_set<minLang::ast::type, minLang::backends::typeHasher> minLang::backends::findUsedTypes(const minLang::ast::function& entrypoint){
	std::unordered_set<minLang::ast::type, minLang::backends::typeHasher> output;
	std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp> traversedFunctions;
	traversedFunctions.insert(entrypoint);
	findUsedTypes_iter(entrypoint.body, output, traversedFunctions);
	return output;
}


void checkCallsValid_iter(const minLang::ast::block& block, bool& valid, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& traversedFunctions);

void checkCallsValid_iter_addExpr(const minLang::ast::expr& expr, bool& valid, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& traversedFunctions){
	if(std::holds_alternative<minLang::ast::call>(expr.value)){
		const auto& call = std::get<minLang::ast::call>(expr.value);
		if(!call.validatedDef){
			std::cerr<<"Encountered non-valid call: "<<call.name<<std::endl;
			valid = false;
			return;
		}
		for(const auto& arg : call.args)
			checkCallsValid_iter_addExpr(arg, valid, traversedFunctions);
		if(!traversedFunctions.contains(call.validatedDef->get())){
			traversedFunctions.insert(call.validatedDef->get());
			checkCallsValid_iter(call.validatedDef->get().body, valid, traversedFunctions);
		}
	}
}

void checkCallsValid_iter(const minLang::ast::block& block, bool& valid, std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp>& traversedFunctions){
	for(const auto& state : block.statements){
		if(std::holds_alternative<minLang::ast::expr>(state)){
			const auto& expr = std::get<minLang::ast::expr>(state);
			checkCallsValid_iter_addExpr(expr, valid, traversedFunctions);
		}else if(std::holds_alternative<minLang::ast::block::declaration>(state)){
		}else if(std::holds_alternative<minLang::ast::block::assignment>(state)){
			//can be ignored as we're (hopefully) only assigning to variable we've already defined (and thus we already know the type)
		}else if(std::holds_alternative<minLang::ast::block::ifStatement>(state)){
			const auto& ifStat = std::get<minLang::ast::block::ifStatement>(state);
			checkCallsValid_iter_addExpr(ifStat.condition, valid, traversedFunctions);
			checkCallsValid_iter(*ifStat.ifBody, valid, traversedFunctions);
			checkCallsValid_iter(*ifStat.elseBody, valid, traversedFunctions);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_while>(state)){
			const auto& whileStat = std::get<minLang::ast::block::forStatement_while>(state);
			checkCallsValid_iter_addExpr(whileStat.condition, valid, traversedFunctions);
			checkCallsValid_iter(*whileStat.body, valid, traversedFunctions);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_normal>(state)){
			const auto& forStat = std::get<minLang::ast::block::forStatement_normal>(state);
			checkCallsValid_iter_addExpr(forStat.breakCond, valid, traversedFunctions);
			checkCallsValid_iter(*forStat.body, valid, traversedFunctions);
		}else{
			std::cerr<<"Error: found unknown statement while searching for types in block"<<std::endl;
		}
	}
}

bool minLang::backends::checkAllCallsValidated(const minLang::ast::function& entrypoint){
	bool output = true;
	std::unordered_set<std::reference_wrapper<const minLang::ast::function>, minLang::backends::funcSigHasher, minLang::backends::funcSigComp> traversedFunctions;
	traversedFunctions.insert(entrypoint);
	checkCallsValid_iter(entrypoint.body, output, traversedFunctions);
	return output;
}

void checkCallsValid_nonrecursive_addExpr(const minLang::ast::expr& expr, bool& valid){
	if(std::holds_alternative<minLang::ast::call>(expr.value)){
		const auto& call = std::get<minLang::ast::call>(expr.value);
		if(!call.validatedDef){
			std::cerr<<"Encountered non-valid call: "<<call.name<<std::endl;
			valid = false;
			return;
		}
		for(const auto& arg : call.args)
			checkCallsValid_nonrecursive_addExpr(arg, valid);
	}
}

void checkCallsValid_nonrecursive(const minLang::ast::block& block, bool& valid){
	for(const auto& state : block.statements){
		if(std::holds_alternative<minLang::ast::expr>(state)){
			const auto& expr = std::get<minLang::ast::expr>(state);
			checkCallsValid_nonrecursive_addExpr(expr, valid);
		}else if(std::holds_alternative<minLang::ast::block::declaration>(state)){
		}else if(std::holds_alternative<minLang::ast::block::assignment>(state)){
			//can be ignored as we're (hopefully) only assigning to variable we've already defined (and thus we already know the type)
		}else if(std::holds_alternative<minLang::ast::block::ifStatement>(state)){
			const auto& ifStat = std::get<minLang::ast::block::ifStatement>(state);
			checkCallsValid_nonrecursive_addExpr(ifStat.condition, valid);
			checkCallsValid_nonrecursive(*ifStat.ifBody, valid);
			checkCallsValid_nonrecursive(*ifStat.elseBody, valid);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_while>(state)){
			const auto& whileStat = std::get<minLang::ast::block::forStatement_while>(state);
			checkCallsValid_nonrecursive_addExpr(whileStat.condition, valid);
			checkCallsValid_nonrecursive(*whileStat.body, valid);
		}else if(std::holds_alternative<minLang::ast::block::forStatement_normal>(state)){
			const auto& forStat = std::get<minLang::ast::block::forStatement_normal>(state);
			checkCallsValid_nonrecursive_addExpr(forStat.breakCond, valid);
			checkCallsValid_nonrecursive(*forStat.body, valid);
		}else{
			std::cerr<<"Error: found unknown statement while searching for types in block"<<std::endl;
		}
	}
}

bool minLang::backends::checkAllCallsValidated_nonRecursive(const minLang::ast::function& func){
	bool output = true;
	checkCallsValid_nonrecursive(func.body, output);
	return output;
}

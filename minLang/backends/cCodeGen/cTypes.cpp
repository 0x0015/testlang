#include "cCodeGen.hpp"
#include <unordered_set>

void findUsedTypes_iter(const minLang::ast::block& block, std::unordered_map<minLang::ast::type, minLang::backends::cCodeGen::cTypeInfo, minLang::backends::cCodeGen::typeHasher>& foundTypes, std::unordered_set<std::string>& traversedFunctions);
void findUsedTypes_iter_addExpr(const minLang::ast::expr& expr, std::unordered_map<minLang::ast::type, minLang::backends::cCodeGen::cTypeInfo, minLang::backends::cCodeGen::typeHasher>& foundTypes, std::unordered_set<std::string>& traversedFunctions){
	if(std::holds_alternative<minLang::ast::literal>(expr.value)){
		const auto& lit = std::get<minLang::ast::literal>(expr.value);
		foundTypes[lit.ty] = {};
	}else if(std::holds_alternative<minLang::ast::call>(expr.value)){
		const auto& call = std::get<minLang::ast::call>(expr.value);
		foundTypes[call.validatedDef->get().ty] = {};
		for(const auto& arg : call.args)
			findUsedTypes_iter_addExpr(arg, foundTypes, traversedFunctions);
		if(!traversedFunctions.contains(minLang::backends::cCodeGen::mangleFuncName(call.validatedDef->get()))){
			traversedFunctions.insert(minLang::backends::cCodeGen::mangleFuncName(call.validatedDef->get()));
			findUsedTypes_iter(call.validatedDef->get().body, foundTypes, traversedFunctions);
		}
	}else if(std::holds_alternative<minLang::ast::varName>(expr.value)){
		const auto& varName = std::get<minLang::ast::varName>(expr.value);
		foundTypes[*varName.matchedType] = {};
	}else{
		std::cerr<<"cCodeGen Error: found unknown expr while searhcing for types"<<std::endl;
	}
}

void findUsedTypes_iter(const minLang::ast::block& block, std::unordered_map<minLang::ast::type, minLang::backends::cCodeGen::cTypeInfo, minLang::backends::cCodeGen::typeHasher>& foundTypes, std::unordered_set<std::string>& traversedFunctions){
	for(const auto& state : block.statements){
		if(std::holds_alternative<minLang::ast::expr>(state)){
			const auto& expr = std::get<minLang::ast::expr>(state);
			findUsedTypes_iter_addExpr(expr, foundTypes, traversedFunctions);
		}else if(std::holds_alternative<minLang::ast::block::declaration>(state)){
			const auto& decl = std::get<minLang::ast::block::declaration>(state);
			foundTypes[decl.ty] = {};
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
			std::cerr<<"cCodeGen Error: found unknown statement while searching for types in block"<<std::endl;
		}
	}
}

std::unordered_map<minLang::ast::type, minLang::backends::cCodeGen::cTypeInfo, minLang::backends::cCodeGen::typeHasher> minLang::backends::cCodeGen::findUsedTypes(std::reference_wrapper<const minLang::ast::function> entrypoint){
	std::unordered_map<minLang::ast::type, cTypeInfo, minLang::backends::cCodeGen::typeHasher> output;
	std::unordered_set<std::string> traversedFunctions;
	traversedFunctions.insert(mangleFuncName(entrypoint.get()));
	findUsedTypes_iter(entrypoint.get().body, output, traversedFunctions);
	return output;
}

minLang::backends::cCodeGen::cTypeInfo genCBuiltinTypeInfo(const minLang::ast::type::builtin_type& ty){
	switch(ty){
		case minLang::ast::type::builtin_type::void_type:
			return minLang::backends::cCodeGen::cTypeInfo{"void"};
		case minLang::ast::type::builtin_type::int8_type:
			return minLang::backends::cCodeGen::cTypeInfo{"int8_t"};
		case minLang::ast::type::builtin_type::int16_type:
			return minLang::backends::cCodeGen::cTypeInfo{"int16_t"};
		case minLang::ast::type::builtin_type::int32_type:
			return minLang::backends::cCodeGen::cTypeInfo{"int32_t"};
		case minLang::ast::type::builtin_type::int64_type:
			return minLang::backends::cCodeGen::cTypeInfo{"int64_t"};
		case minLang::ast::type::builtin_type::uint8_type:
			return minLang::backends::cCodeGen::cTypeInfo{"uint8_t"};
		case minLang::ast::type::builtin_type::uint16_type:
			return minLang::backends::cCodeGen::cTypeInfo{"uint16_t"};
		case minLang::ast::type::builtin_type::uint32_type:
			return minLang::backends::cCodeGen::cTypeInfo{"uint32_t"};
		case minLang::ast::type::builtin_type::uint64_type:
			return minLang::backends::cCodeGen::cTypeInfo{"uint64_t"};
		case minLang::ast::type::builtin_type::float32_type:
			return minLang::backends::cCodeGen::cTypeInfo{"float"};
		case minLang::ast::type::builtin_type::float64_type:
			return minLang::backends::cCodeGen::cTypeInfo{"double"};
		case minLang::ast::type::builtin_type::bool_type:
			return minLang::backends::cCodeGen::cTypeInfo{"unsigned char"};
		default:
			return {};
	}
}

struct typeStructTable_entry{
	minLang::ast::type ty;
	std::string name;
	std::string def;
};
struct typeStructTable{
private:
	static inline const typeStructTable_entry defaultEntry = {};
public:
	std::unordered_set<minLang::ast::type, minLang::backends::cCodeGen::typeHasher> knownTypes;
	std::vector<typeStructTable_entry> entries;//we want the entries to be ordered so that if (int, float) is struct 0, then it is above (in the c file) ((int, float), bool) which contains it.
	const typeStructTable_entry& find(const minLang::ast::type& ty) const{
		for(const auto& entry : entries){
			if(entry.ty == ty)
				return entry;
		}
		std::cerr<<"cCodeGen Error: did not find type ("<<ty.toString()<<") struct def in table (when it was expected to be there)"<<std::endl;
		return defaultEntry;
	}
};

minLang::backends::cCodeGen::cTypeInfo genCTypeInfo(const minLang::ast::type& ty, typeStructTable& tst){
	//for now assume that the system width is 64 bits.  somewhat bad optimization, but anything bigger will be referenced through a pointer dereference.
	if(std::holds_alternative<minLang::ast::type::builtin_type>(ty.ty)){
		return genCBuiltinTypeInfo(std::get<minLang::ast::type::builtin_type>(ty.ty));
	}else if(std::holds_alternative<minLang::ast::type::array_type>(ty.ty)){
		const auto& arrayTy = std::get<minLang::ast::type::array_type>(ty.ty);
		auto baseTy = genCTypeInfo(*arrayTy.ty, tst);
		baseTy.cName += "*";
		baseTy.isPointer++;
		return baseTy;
	}else{
		std::cerr<<"cCodeGen Error: tried to get C type info of an unknown type: "<<ty.toString()<<std::endl;
		return {};
	}

	return {};
}

std::string minLang::backends::cCodeGen::genUsedCTypes(std::unordered_map<minLang::ast::type, cTypeInfo, typeHasher>& usedTypes){
	typeStructTable tst;
	for(auto& [type, cInfo] : usedTypes){
		cInfo = genCTypeInfo(type, tst);
	}

	std::string typeStructSection;
	for(const auto& entry : tst.entries){
		typeStructSection += entry.def;
		typeStructSection += '\n';
	}
	return typeStructSection;
}

std::string minLang::backends::cCodeGen::getDefaultTypeValue(const minLang::ast::type& ty, const std::unordered_map<minLang::ast::type, cTypeInfo, typeHasher>& types){
	if(std::holds_alternative<minLang::ast::type::builtin_type>(ty.ty)){
		const auto& builtin = std::get<minLang::ast::type::builtin_type>(ty.ty);
		switch (builtin){
			case minLang::ast::type::int8_type:
				return "((int8_t)0)";
			case minLang::ast::type::int16_type:
				return "((int16_t)0)";
			case minLang::ast::type::int32_type:
				return "((int32_t)0)";
			case minLang::ast::type::int64_type:
				return "((int64_t)0)";
			case minLang::ast::type::uint8_type:
				return "((uint8_t)0u)";
			case minLang::ast::type::uint16_type:
				return "((uint16_t)0u)";
			case minLang::ast::type::uint32_type:
				return "((uint32_t)0u)";
			case minLang::ast::type::uint64_type:
				return "((uint64_t)0u)";
			case minLang::ast::type::float32_type:
				return "0.0f";
			case minLang::ast::type::float64_type:
				return "0.0";
			case minLang::ast::type::bool_type:
				return "((unsigned char)0)";
			default:
				std::cerr<<"cCodeGen Error: requested default value of empty/void builtin type"<<std::endl;
				return "<empty builtin type>";
		}
	}else if(std::holds_alternative<minLang::ast::type::array_type>(ty.ty)){
		const auto& array = std::get<minLang::ast::type::array_type>(ty.ty);
		std::string perElementDefVal = getDefaultTypeValue(*array.ty, types);
		std::string output = "(" + types.at(*array.ty).cName + "[" + std::to_string(array.length) + "]){";
		output.reserve(perElementDefVal.size()*array.length + array.length*2);
		for(unsigned int i=0;i<array.length;i++){
			output += perElementDefVal;
			if(i+1 < array.length)
				output += ", ";
		}
		output += "}";
		return output;
	}else{
		std::cerr<<"cCodeGen Error: Encountered unknown type in getDefaultTypeValue"<<std::endl;
		return "<value of unknown type>";
	}
}


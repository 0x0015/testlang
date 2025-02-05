#include "cCodeGen.hpp"
#include <unordered_set>

void findUsedTypes_iter(const ast::block& block, std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher>& foundTypes, std::unordered_set<std::string>& traversedFunctions);
void findUsedTypes_iter_addExpr(const ast::expr& expr, std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher>& foundTypes, std::unordered_set<std::string>& traversedFunctions){
	if(std::holds_alternative<ast::literal>(expr.value)){
		const auto& lit = std::get<ast::literal>(expr.value);
		foundTypes[lit.ty] = {};
	}else if(std::holds_alternative<ast::call>(expr.value)){
		const auto& call = std::get<ast::call>(expr.value);
		foundTypes[call.validatedDef->get().ty] = {};
		for(const auto& arg : call.args)
			findUsedTypes_iter_addExpr(arg, foundTypes, traversedFunctions);
		if(!traversedFunctions.contains(cCodeGen::mangleFuncName(call.validatedDef->get()))){
			traversedFunctions.insert(cCodeGen::mangleFuncName(call.validatedDef->get()));
			findUsedTypes_iter(call.validatedDef->get().body, foundTypes, traversedFunctions);
		}
	}else if(std::holds_alternative<ast::varName>(expr.value)){
		const auto& varName = std::get<ast::varName>(expr.value);
		foundTypes[*varName.matchedType] = {};
	}else{
		std::cerr<<"cCodeGen Error: found unknown expr while searhcing for types"<<std::endl;
	}
}

void findUsedTypes_iter(const ast::block& block, std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher>& foundTypes, std::unordered_set<std::string>& traversedFunctions){
	for(const auto& state : block.statements){
		if(std::holds_alternative<ast::expr>(state)){
			const auto& expr = std::get<ast::expr>(state);
			findUsedTypes_iter_addExpr(expr, foundTypes, traversedFunctions);
		}else if(std::holds_alternative<ast::block::declaration>(state)){
			const auto& decl = std::get<ast::block::declaration>(state);
			foundTypes[decl.ty] = {};
		}else if(std::holds_alternative<ast::block::assignment>(state)){
			//can be ignored as we're (hopefully) only assigning to variable we've already defined (and thus we already know the type)
		}else if(std::holds_alternative<ast::block::ifStatement>(state)){
			const auto& ifStat = std::get<ast::block::ifStatement>(state);
			findUsedTypes_iter_addExpr(ifStat.condition, foundTypes, traversedFunctions);
			findUsedTypes_iter(*ifStat.ifBody, foundTypes, traversedFunctions);
			findUsedTypes_iter(*ifStat.elseBody, foundTypes, traversedFunctions);
		}else if(std::holds_alternative<ast::block::forStatement_while>(state)){
			const auto& whileStat = std::get<ast::block::forStatement_while>(state);
			findUsedTypes_iter_addExpr(whileStat.condition, foundTypes, traversedFunctions);
			findUsedTypes_iter(*whileStat.body, foundTypes, traversedFunctions);
		}else if(std::holds_alternative<ast::block::forStatement_normal>(state)){
			const auto& forStat = std::get<ast::block::forStatement_normal>(state);
			findUsedTypes_iter_addExpr(forStat.breakCond, foundTypes, traversedFunctions);
			findUsedTypes_iter(*forStat.body, foundTypes, traversedFunctions);
		}else{
			std::cerr<<"cCodeGen Error: found unknown statement while searching for types in block"<<std::endl;
		}
	}
}

std::unordered_map<ast::type, cCodeGen::cTypeInfo, cCodeGen::typeHasher> cCodeGen::findUsedTypes(std::reference_wrapper<const ast::function> entrypoint){
	std::unordered_map<ast::type, cTypeInfo, cCodeGen::typeHasher> output;
	std::unordered_set<std::string> traversedFunctions;
	traversedFunctions.insert(mangleFuncName(entrypoint.get()));
	findUsedTypes_iter(entrypoint.get().body, output, traversedFunctions);
	return output;
}

cCodeGen::cTypeInfo genCBuiltinTypeInfo(const ast::type::builtin_type& ty){
	switch(ty){
		case ast::type::builtin_type::void_type:
			return cCodeGen::cTypeInfo{"void"};
		case ast::type::builtin_type::int8_type:
			return cCodeGen::cTypeInfo{"int8_t"};
		case ast::type::builtin_type::int16_type:
			return cCodeGen::cTypeInfo{"int16_t"};
		case ast::type::builtin_type::int32_type:
			return cCodeGen::cTypeInfo{"int32_t"};
		case ast::type::builtin_type::int64_type:
			return cCodeGen::cTypeInfo{"int64_t"};
		case ast::type::builtin_type::uint8_type:
			return cCodeGen::cTypeInfo{"uint8_t"};
		case ast::type::builtin_type::uint16_type:
			return cCodeGen::cTypeInfo{"uint16_t"};
		case ast::type::builtin_type::uint32_type:
			return cCodeGen::cTypeInfo{"uint32_t"};
		case ast::type::builtin_type::uint64_type:
			return cCodeGen::cTypeInfo{"uint64_t"};
		case ast::type::builtin_type::float32_type:
			return cCodeGen::cTypeInfo{"float"};
		case ast::type::builtin_type::float64_type:
			return cCodeGen::cTypeInfo{"double"};
		case ast::type::builtin_type::bool_type:
			return cCodeGen::cTypeInfo{"unsigned char"};
		default:
			return {};
	}
}

struct typeStructTable_entry{
	ast::type ty;
	std::string name;
	std::string def;
};
struct typeStructTable{
private:
	static inline const typeStructTable_entry defaultEntry = {};
public:
	std::unordered_set<ast::type, cCodeGen::typeHasher> knownTypes;
	std::vector<typeStructTable_entry> entries;//we want the entries to be ordered so that if (int, float) is struct 0, then it is above (in the c file) ((int, float), bool) which contains it.
	const typeStructTable_entry& find(const ast::type& ty) const{
		for(const auto& entry : entries){
			if(entry.ty == ty)
				return entry;
		}
		std::cerr<<"cCodeGen Error: did not find type ("<<ty.toString()<<") struct def in table (when it was expected to be there)"<<std::endl;
		return defaultEntry;
	}
};

cCodeGen::cTypeInfo genCTypeInfo(const ast::type& ty, typeStructTable& tst){
	//for now assume that the system width is 64 bits.  somewhat bad optimization, but anything bigger will be referenced through a pointer dereference.
	if(std::holds_alternative<ast::type::builtin_type>(ty.ty)){
		return genCBuiltinTypeInfo(std::get<ast::type::builtin_type>(ty.ty));
	}else if(std::holds_alternative<ast::type::array_type>(ty.ty)){
		const auto& arrayTy = std::get<ast::type::array_type>(ty.ty);
		auto baseTy = genCTypeInfo(*arrayTy.ty, tst);
		baseTy.cName += "*";
		baseTy.isPointer++;
		return baseTy;
	}else if(std::holds_alternative<ast::type::tuple_type>(ty.ty)){
		const auto& tupleTy = std::get<ast::type::tuple_type>(ty.ty);
		cCodeGen::cTypeInfo output{};
		if(tst.knownTypes.contains(ty)){
			output.cName = tst.find(ty).name;
			//std::cout<<"looking up type in table: "<<ty.toString()<<std::endl;
		}else{
			std::string structDef = "{\n";
			for(unsigned int i=0;i<tupleTy.size();i++){
				auto subTypeInfo = genCTypeInfo(tupleTy[i], tst);
				structDef += "\t" + subTypeInfo.cName + " val" + std::to_string(i) + ";\n";
			}
			structDef += "};";
			std::string structName = "cCodeGen_tupleStructDef_" + std::to_string(tst.entries.size());
			structDef = "struct " + structName + structDef;
			if(tst.knownTypes.contains(ty)){
				output.cName = tst.find(ty).name;
			}else{
				tst.knownTypes.insert(ty);
				tst.entries.push_back(typeStructTable_entry{ty, "struct " + structName, structDef});
				output.cName = "struct " + structName;
			}

			//let's actually just not worry about this yet
			/*
			if(ty.getSize() > 8 *64 bytes*){
				output.cName += "*";
				output.isPointer++;
			}
			*/
		}
		return output;
	}else if(std::holds_alternative<ast::type::alias_type>(ty.ty)){
		const auto& alias = std::get<ast::type::alias_type>(ty.ty);
		const auto& tyInfo = genCTypeInfo(*alias.underlyingType, tst);
		//std::cout<<"Returning alias: "<<ty.toString()<<" direction to ctype: "<<tyInfo.cName<<std::endl;
		return tyInfo;
	}else{
		std::cerr<<"cCodeGen Error: tried to get C type info of an unknown type: "<<ty.toString()<<std::endl;
		return {};
	}

	return {};
}

std::string cCodeGen::genUsedCTypes(std::unordered_map<ast::type, cTypeInfo, typeHasher>& usedTypes){
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

std::string cCodeGen::getDefaultTypeValue(const ast::type& ty, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types){
	if(std::holds_alternative<ast::type::builtin_type>(ty.ty)){
		const auto& builtin = std::get<ast::type::builtin_type>(ty.ty);
		switch (builtin){
			case ast::type::int8_type:
				return "((int8_t)0)";
			case ast::type::int16_type:
				return "((int16_t)0)";
			case ast::type::int32_type:
				return "((int32_t)0)";
			case ast::type::int64_type:
				return "((int64_t)0)";
			case ast::type::uint8_type:
				return "((uint8_t)0u)";
			case ast::type::uint16_type:
				return "((uint16_t)0u)";
			case ast::type::uint32_type:
				return "((uint32_t)0u)";
			case ast::type::uint64_type:
				return "((uint64_t)0u)";
			case ast::type::float32_type:
				return "0.0f";
			case ast::type::float64_type:
				return "0.0";
			case ast::type::bool_type:
				return "((unsigned char)0)";
			default:
				std::cerr<<"cCodeGen Error: requested default value of empty/void builtin type"<<std::endl;
				return "<empty builtin type>";
		}
	}else if(std::holds_alternative<ast::type::array_type>(ty.ty)){
		const auto& array = std::get<ast::type::array_type>(ty.ty);
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
	}else if(std::holds_alternative<ast::type::tuple_type>(ty.ty)){
		//TODO: very VERY likely needs to be fixed.  Not sure how to though (maybe just include a table of default values at the top of the file?)
		const auto& tuple = std::get<ast::type::tuple_type>(ty.ty);
		std::string output = "{";
		for(unsigned int i=0;i<tuple.size();i++){
			output += getDefaultTypeValue(tuple[i], types);
			if(i+1 < tuple.size())
				output += ", ";
		}
		output += "}";
		return output;
	}else if(std::holds_alternative<ast::type::alias_type>(ty.ty)){
		const auto& alias = std::get<ast::type::alias_type>(ty.ty);
		return getDefaultTypeValue(*alias.underlyingType, types);
	}else if(std::holds_alternative<ast::type::template_type>(ty.ty)){
		std::cerr<<"cCodeGen Error: Encountered template type in getDefaultTypeValue.  Should never happen!"<<std::endl;
		return "<template type>";
	}else{
		std::cerr<<"cCodeGen Error: Encountered unknown type in getDefaultTypeValue"<<std::endl;
		return "<value of unknown type>";
	}
}


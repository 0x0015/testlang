#include "cCodeGen.hpp"

std::string minLang::backends::cCodeGen::genBuiltins(){
	std::string output;
	//TODO: make these generalize for the normal bulitin types (rather than just the subset listed here)

	//prints
	ast::function print_int_func;
	print_int_func.name = "print";
	print_int_func.ty = ast::type::builtin_type::void_type;
	print_int_func.args = {ast::function::argument{ast::type::builtin_type::int32_type}};
	output += "void " + minLang::backends::cCodeGen::mangleFuncName(print_int_func) + "(int32_t val){\n\tprintf(\"%i\", val);\n}\n";

	ast::function print_bool_func;
	print_bool_func.name = "print";
	print_bool_func.ty = ast::type::builtin_type::void_type;
	print_bool_func.args = {ast::function::argument{ast::type::builtin_type::bool_type}};
	output += "void " + minLang::backends::cCodeGen::mangleFuncName(print_bool_func) + "(unsigned char val){\n\tprintf(val ? \"true\" : \"false\");\n}\n";

	ast::function print_float_func;
	print_float_func.name = "print";
	print_float_func.ty = ast::type::builtin_type::void_type;
	print_float_func.args = {ast::function::argument{ast::type::builtin_type::float32_type}};
	output += "void " + minLang::backends::cCodeGen::mangleFuncName(print_float_func) + "(float val){\n\tprintf(\"%g\", val);\n}\n";

	ast::function println_int_func;
	println_int_func.name = "println";
	println_int_func.ty = ast::type::builtin_type::void_type;
	println_int_func.args = {ast::function::argument{ast::type::builtin_type::int32_type}};
	output += "void " + minLang::backends::cCodeGen::mangleFuncName(println_int_func) + "(int val){\n\tprintf(\"%i\\n\", val);\n}\n";

	ast::function println_bool_func;
	println_bool_func.name = "println";
	println_bool_func.ty = ast::type::builtin_type::void_type;
	println_bool_func.args = {ast::function::argument{ast::type::builtin_type::bool_type}};
	output += "void " + minLang::backends::cCodeGen::mangleFuncName(println_bool_func) + "(unsigned char val){\n\tprintf(val ? \"true\\n\" : \"false\\n\");\n}\n";

	ast::function println_float_func;
	println_float_func.name = "println";
	println_float_func.ty = ast::type::builtin_type::void_type;
	println_float_func.args = {ast::function::argument{ast::type::builtin_type::float32_type}};
	output += "void " + minLang::backends::cCodeGen::mangleFuncName(println_float_func) + "(float val){\n\tprintf(\"%g\\n\", val);\n}\n";

	return output;
}

std::string minLang::backends::cCodeGen::genTemplateFuncBuiltins(const std::unordered_set<std::reference_wrapper<const ast::function>, funcSigHasher, funcSigComp>& funcs, const std::unordered_map<ast::type, cTypeInfo, typeHasher>& types){
	std::string output;
	for(const auto& func_wrap : funcs){
		const auto& func = func_wrap.get();
		if(func.name.starts_with("get<")){
			std::cout<<"found get template builtin! "<<func.name<<std::endl;
			const auto& arrayItemTy = types.at(func.ty).cName;
			output += arrayItemTy + " " + minLang::backends::cCodeGen::mangleFuncName(func) + "(" + types.at(func.args[0].ty).cName + " arr, " + types.at(func.args[1].ty).cName + " index){\n\treturn(arr[index]);\n}\n";
		}else if(func.name.starts_with("set<")){
			std::cout<<"found set template builtin! "<<func.name<<std::endl;
			const auto& arrayTy = types.at(func.ty).cName;
			//const auto& arrayItemTy = types.at(*std::get<ast::type::array_type>(func.ty.ty).ty).cName;
			output += arrayTy + " " + minLang::backends::cCodeGen::mangleFuncName(func) + "(" + types.at(func.args[0].ty).cName + " arr, " + types.at(func.args[1].ty).cName + " val, "+ types.at(func.args[2].ty).cName + " index){\n\tarr[index] = val;\n\treturn(arr);\n}\n"; //THIS IS TECHNICALLY NOT CORRECT!  with that being said, we will assume it is for now
		}
	}
	return output;
}

bool minLang::backends::cCodeGen::isSpecificBuiltinFuncCase(const ast::function& func){
	if(func.status != ast::function::positionStatus::builtin)
		return false;
	//arithmatic
	if(func.name == "add" || func.name == "sub" || func.name == "mul" || func.name == "div" || func.name == "mod")
		return true;
	//logic
	if(func.name == "equal" || func.name == "greater" || func.name == "less" || func.name == "greaterOrEqual" || func.name == "lessOrEqual")
		return true;
	if(func.name == "and" || func.name == "or" || func.name == "not")
		return true;
	return false;
}

std::string minLang::backends::cCodeGen::genSpecificBuiltinFuncCase(const ast::call& call, const std::vector<std::string>& generatedArgs){
	//arithmatic
	if(call.name == "add")
		return "(" + generatedArgs[0] + "+" + generatedArgs[1] + ")";
	if(call.name == "sub")
		return "(" + generatedArgs[0] + "-" + generatedArgs[1] + ")";
	if(call.name == "div")
		return "(" + generatedArgs[0] + "/" + generatedArgs[1] + ")";
	if(call.name == "mul")
		return "(" + generatedArgs[0] + "*" + generatedArgs[1] + ")";
	if(call.name == "mod")
		return "(" + generatedArgs[0] + "%" + generatedArgs[1] + ")";

	//logic
	if(call.name == "equal")
		return "(" + generatedArgs[0] + "==" + generatedArgs[1] + ")";
	if(call.name == "greater")
		return "(" + generatedArgs[0] + ">" + generatedArgs[1] + ")";
	if(call.name == "less")
		return "(" + generatedArgs[0] + "<" + generatedArgs[1] + ")";
	if(call.name == "greaterOrEqual")
		return "(" + generatedArgs[0] + ">=" + generatedArgs[1] + ")";
	if(call.name == "lessOrEqual")
		return "(" + generatedArgs[0] + "<=" + generatedArgs[1] + ")";
	if(call.name == "and")
		return "(" + generatedArgs[0] + "&&" + generatedArgs[1] + ")";
	if(call.name == "or")
		return "(" + generatedArgs[0] + "||" + generatedArgs[1] + ")";
	if(call.name == "not")
		return "(!" + generatedArgs[0] + ")";

	std::cerr<<"Internal error: cCodeGen specfic builtin case fell through! (call name: "<<call.name<<")"<<std::endl;
	return "";//should never get to this case
}


#include "parseContext.hpp"
#include "../tokenize/mediumTokenize.hpp"
#include "parseFunction.hpp"
#include "../parseUtil.hpp"

std::optional<ast::context> parseContext(std::span<const mediumToken> tokens){
	ast::context context;
	bool trying = true;
	while(trying){
		trying = false;
		if(tokens.empty())
			break;
		const auto& funcTry = parseFunction(tokens);
		if(funcTry){
			parse_debug_print("context parsed function");
			tokens = tokens.subspan(funcTry->toksConsumed);
			context.funcs.push_back(funcTry->val);
			trying = true;
		}else{
			std::cerr<<"Error: Failed to parse function"<<std::endl;
			return std::nullopt;
		}
	}

	return context;
}


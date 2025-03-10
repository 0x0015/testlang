#include "ast_literal.hpp"

//TODO: add implementation
std::string ast::literal::toString() const{
	if(std::holds_alternative<ast::literal::builtin_literal>(value)){
		const auto& builtin = std::get<ast::literal::builtin_literal>(value);
		if(std::holds_alternative<int64_t>(builtin)){
			return("(int64) " + std::to_string(std::get<int64_t>(builtin)));
		}else if(std::holds_alternative<int32_t>(builtin)){
			return("(int32) " + std::to_string(std::get<int32_t>(builtin)));
		}else if(std::holds_alternative<uint64_t>(builtin)){
			return("(uint64) " + std::to_string(std::get<uint64_t>(builtin)) + "u");
		}else if(std::holds_alternative<uint32_t>(builtin)){
			return("(uint32) " + std::to_string(std::get<uint32_t>(builtin)) + "u");
		}else if(std::holds_alternative<double>(builtin)){
			return("(float64) " + std::to_string(std::get<double>(builtin)));
		}else if(std::holds_alternative<float>(builtin)){
			return("(float32) " + std::to_string(std::get<float>(builtin)) + "f");
		}else if(std::holds_alternative<bool>(builtin)){
			return(std::get<bool>(builtin) ? "(bool) true" : "(bool) false");
		}else{
			std::cerr<<"Error: invalid builtin literal type"<<std::endl;
			return "<unknown literal>";
		}
	}else if(std::holds_alternative<array_literal>(value)){
		const auto& arr = std::get<array_literal>(value);
		std::string output = "[";
		for(unsigned int i=0;i<arr.vals.size();i++){
			output += arr.vals[i].toString();
			if(i+1 < arr.vals.size())
				output += ", ";
		}
		output += ']';
		return output;
	}else if(std::holds_alternative<tuple_literal>(value)){
		const auto& arr = std::get<tuple_literal>(value);
		std::string output = "{";
		for(unsigned int i=0;i<arr.vals.size();i++){
			output += arr.vals[i].toString();
			if(i+1 < arr.vals.size())
				output += ", ";
		}
		output += '}';
		return output;
	}
	std::cerr<<"Error: invalid builtin"<<std::endl;
	return "<unknown literal>";
}

#pragma once
#include <iostream>
#include <vector>
#include <variant>
#include <optional>

namespace ast{
	enum type{
		none_type,
		void_type,
		int_type,
		float_type,
		bool_type
	};
	constexpr inline type type_map(const std::string_view type){
		if(type == "void")
			return void_type;
		if(type == "int")
			return int_type;
		if(type == "float")
			return float_type;
		if(type == "bool")
			return bool_type;
		return none_type;
	}
	constexpr inline std::string_view type_rmap(type ty){
		switch(ty){
			case void_type:
				return "void";
			case int_type:
				return "int";
			case float_type:
				return "float";
			case bool_type:
				return "bool";
			default:
				return "none";
		}
	}
	struct function{
		type ty;
		std::string name;
		struct argument{
			type ty;
			std::string name;
		};
		std::vector<argument> args;
		struct declaration{
			type ty;
			std::string name;
		};
		struct call{
			std::string name;
			std::vector<std::string> args;
			std::optional<std::reference_wrapper<const function>> validatedDef = std::nullopt;
		};
		using statement = std::variant<declaration, call>;
		std::vector<statement> body;
		bool builtin = false;
		void dump() const;
	};
	struct context{
		std::vector<function> funcs;
		void dump() const;
	};
}

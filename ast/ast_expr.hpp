#pragma once
#include <string>
#include <vector>
#include <optional>
#include <variant>

#include "ast_literal.hpp"

namespace ast{
	struct function;
	struct expr;
	struct call{
		std::string name;
		std::vector<expr> args;
		std::optional<std::reference_wrapper<const function>> validatedDef = std::nullopt;
	};
	struct expr{
		using varName = std::string;
		std::variant<literal, call, varName> value;
		expr(const call& c) : value(c){}
		expr(const literal& l) : value(l){}
		expr(const std::string& s) : value(s){}
		void dump() const;
	};
}

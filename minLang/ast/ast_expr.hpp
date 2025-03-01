#pragma once
#include <string>
#include <vector>
#include <optional>
#include <variant>

#include "ast_literal.hpp"

namespace minLang{
namespace ast{
	struct function;
	struct expr;
	struct call{
		std::string name;
		std::vector<expr> args;
		std::optional<std::reference_wrapper<const function>> validatedDef = std::nullopt;
	};
	struct varName{
		std::string name;
		std::optional<type> matchedType = std::nullopt;
	};
	struct expr{
		std::variant<literal, call, varName> value;
		expr() = default;
		expr(const call& c) : value(c){}
		expr(const literal& l) : value(l){}
		expr(const std::string& s) : value(varName{s}){}
		void dump() const;
		expr clone() const;
		std::optional<ast::type> inferType() const;
	};
}
}

#pragma once
#include <string>
#include <vector>
#include <optional>
#include <variant>

#include "ast_literal.hpp"

namespace ast{
	struct function;
	struct functionTemplate;
	struct expr;
	struct call{
		std::string name;
		std::vector<expr> args;
		std::optional<std::reference_wrapper<const function>> validatedDef = std::nullopt;
	};
	struct templateCall{
		std::string name;
		std::vector<expr> args;
		std::vector<ast::type> templateArgs;
	};
	struct varName{
		std::string name;
		std::optional<type> matchedType = std::nullopt;
	};
	struct expr{
		std::variant<literal, call, templateCall, varName> value;
		expr() = default;
		expr(const call& c) : value(c){}
		expr(const templateCall& tc) : value(tc){}
		expr(const literal& l) : value(l){}
		expr(const std::string& s) : value(varName{s}){}
		void dump() const;
		expr clone() const;
		std::optional<ast::type> inferType() const;
	};
}

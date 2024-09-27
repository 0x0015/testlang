#pragma once
#include <iostream>
#include <vector>
#include <variant>
#include <optional>

#include "ast_type.hpp"
#include "ast_literal.hpp"

namespace ast{
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
			using varNameArg = std::string;
			using argument = std::variant<varNameArg, literal>;
			std::vector<argument> args;
			std::optional<std::reference_wrapper<const function>> validatedDef = std::nullopt;
		};
		struct assignment{
			using varName = std::string;
			varName assignTo;
			std::variant<varName, literal> assignFrom;
		};
		using statement = std::variant<declaration, call, assignment>;
		std::vector<statement> body;
		enum positionStatus{
			normal,
			builtin,
			external
		};
		positionStatus status = positionStatus::normal;
		void dump() const;
	};
	struct context{
		std::vector<function> funcs;
		void dump() const;
	};
}

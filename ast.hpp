#pragma once
#include <iostream>
#include <vector>
#include <variant>
#include <optional>

#include "ast_type.hpp"

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
			using literalArg = std::variant<int, float, bool>;
			using argument = std::variant<varNameArg, literalArg>;
			static type getLiteralType(const literalArg& lit);
			std::vector<argument> args;
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

#pragma once
#include <iostream>
#include <vector>
#include <variant>
#include <optional>

#include "ast_type.hpp"
#include "ast_literal.hpp"
#include "ast_expr.hpp"
#include "ast_block.hpp"

namespace ast{
	struct function{
		type ty;
		std::string name;
		struct argument{
			type ty;
			std::string name;
		};
		std::vector<argument> args;
		block body;
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

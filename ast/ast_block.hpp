#pragma once
#include <vector>
#include <variant>

#include "ast_type.hpp"
#include "ast_literal.hpp"
#include "ast_expr.hpp"

namespace ast{
	struct block{
		struct declaration{
			type ty;
			std::string name;
		};
		struct assignment{
			using varName = std::string;
			varName assignTo;
			expr assignFrom;
		};
		struct ifStatement{
			expr condition;
			std::shared_ptr<block> ifBody;
			std::shared_ptr<block> elseBody;
		};
		struct returnStatement{
			expr val;
		};
		using statement = std::variant<declaration, assignment, ifStatement, returnStatement, expr>;
		std::vector<statement> statements;
		void dump() const;
	};
}

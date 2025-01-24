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
		struct forStatement_normal{
			assignment initialDecl;
			expr breakCond;
			assignment perLoopAsgn;
			std::shared_ptr<block> body;
		};
		struct forStatement_while{
			expr condition;
			std::shared_ptr<block> body;
		};
		struct returnStatement{
			expr val;
		};
		using statement = std::variant<declaration, assignment, ifStatement, forStatement_normal, forStatement_while, returnStatement, expr>;
		std::vector<statement> statements;
		void dump() const;
		block clone() const;
	};
}

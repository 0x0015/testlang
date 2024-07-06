#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>
#include "../ast.hpp"

#include "type.hpp"
#include "identifier.hpp"
#include "literal.hpp"
#include "util.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct function_decl_t{
		struct arguments_t{
			struct argument_t{
				static constexpr auto rule = type + identifier;
				static constexpr auto value = lexy::construct<ast::function::argument>;
			};
			static constexpr auto argument = dsl::p<argument_t>;
			static constexpr auto rule = dsl::parenthesized(util::optList(argument, dsl::sep(dsl::comma)));
			static constexpr auto value = lexy::as_list<std::vector<ast::function::argument>>;
		};
		static constexpr auto arguments = dsl::p<arguments_t>;
		struct body_t{
			struct statement_t{
				struct declaration_t{
					static constexpr auto rule = type + identifier;
					static constexpr auto value = lexy::construct<ast::function::declaration>;
				};
				static constexpr auto declaration = dsl::p<declaration_t>;
				struct call_t{
					struct arguments_t{
						struct argument_t{
							static constexpr auto rule = identifier | literal;
							static constexpr auto value = lexy::construct<ast::function::call::argument>;
						};
						static constexpr auto argument = dsl::p<argument_t>;
						static constexpr auto rule = dsl::parenthesized(util::optList(argument, dsl::sep(dsl::comma)));;
						static constexpr auto value = lexy::as_list<std::vector<ast::function::call::argument>>;
					};
					static constexpr auto arguments = dsl::p<arguments_t>;
					static constexpr auto rule = identifier >> arguments;
					static constexpr auto value = lexy::construct<ast::function::call>;
				};
				static constexpr auto call = dsl::p<call_t>;
				static constexpr auto rule = declaration | call;
				static constexpr auto value = lexy::construct<ast::function::statement>;
			};
			static constexpr auto statement = dsl::p<statement_t>;
			static constexpr auto rule = dsl::curly_bracketed(util::optList(statement, dsl::trailing_sep(dsl::semicolon)));
			static constexpr auto value = lexy::as_list<std::vector<ast::function::statement>>;
		};
		static constexpr auto body = dsl::p<body_t>;
		static constexpr auto rule = type >> identifier + arguments + body;
		static constexpr auto value = lexy::construct<ast::function>;
	};
	constexpr auto function_decl = dsl::p<function_decl_t>;
}
}

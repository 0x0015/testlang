#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>
#include "../ast.hpp"
#include "id.hpp"
#include "literal.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct type_t : lexy::expression_production{
		static constexpr auto recur_type = dsl::recurse<type_t>;
		struct builtin_type_t{
			struct void_type{
				static constexpr auto rule = LEXY_KEYWORD("void", id);
				static constexpr auto value = lexy::constant(ast::type::builtin_type::void_type);
			};
			struct int_type{
				static constexpr auto rule = LEXY_KEYWORD("int", id);
				static constexpr auto value = lexy::constant(ast::type::builtin_type::int_type);
			};
			struct float_type{
				static constexpr auto rule = LEXY_KEYWORD("float", id);
				static constexpr auto value = lexy::constant(ast::type::builtin_type::float_type);
			};
			struct bool_type{
				static constexpr auto rule = LEXY_KEYWORD("bool", id);
				static constexpr auto value = lexy::constant(ast::type::builtin_type::bool_type);
			};
			static constexpr auto rule = dsl::p<void_type> | dsl::p<int_type> | dsl::p<float_type> | dsl::p<bool_type>;
			static constexpr auto value = lexy::forward<ast::type::builtin_type>;
		};
		static constexpr auto builtin_type = dsl::p<builtin_type_t>;
		//note: add some peeks to make sure that tuple and array will parse correctly
		struct tuple_type_t{
			//empty tuple is not a valid type (it is a void type)
			static constexpr auto rule = dsl::parenthesized(dsl::list(recur_type, dsl::sep(dsl::comma)));
			static constexpr auto value = lexy::as_list<ast::type::tuple_type>;
		};
		static constexpr auto tuple_type = dsl::p<tuple_type_t>;
		static constexpr auto atom = builtin_type | tuple_type;
		struct array_type_t : dsl::postfix_op{
			//static constexpr auto rule = dsl::peek(LEXY_LIT("array")) >> builtin_type;
			//static constexpr auto value = lexy::construct<ast::type::builtin_type>;
			static constexpr auto op = dsl::op(dsl::square_bracketed(dsl::digits<>));
			using operand = dsl::atom;
			static constexpr auto value = lexy::callback<ast::type>([](const ast::type& value){return value;},
									      [](const ast::type& value, lexy::op<op>){return value;},
									      [](){});
		};
		using array_type = array_type_t;

		using operation = array_type;
		

		//static constexpr auto rule = array_type | builtin_type | tuple_type;
		static constexpr auto value = lexy::construct<ast::type>;
	};
	constexpr auto type = dsl::peek(dsl::p<type_t>) >> dsl::p<type_t>;
}
}

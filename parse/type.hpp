#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>
#include "../ast.hpp"
#include "id.hpp"
#include "literal.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct type_t{
		static constexpr auto recur_type = dsl::recurse<type_t>;
		struct builtin_type_t{
			struct void_type{
				static constexpr auto rule = LEXY_KEYWORD("void", id);
				static constexpr auto value = lexy::constant(ast::builtin_type::void_type);
			};
			struct int_type{
				static constexpr auto rule = LEXY_KEYWORD("int", id);
				static constexpr auto value = lexy::constant(ast::builtin_type::int_type);
			};
			struct float_type{
				static constexpr auto rule = LEXY_KEYWORD("float", id);
				static constexpr auto value = lexy::constant(ast::builtin_type::float_type);
			};
			struct bool_type{
				static constexpr auto rule = LEXY_KEYWORD("bool", id);
				static constexpr auto value = lexy::constant(ast::builtin_type::bool_type);
			};
			static constexpr auto rule = dsl::p<void_type> | dsl::p<int_type> | dsl::p<float_type> | dsl::p<bool_type>;
			static constexpr auto value = lexy::forward<ast::builtin_type>;
		};
		static constexpr auto builtin_type = dsl::p<builtin_type_t>;
		//note: add some peeks to make sure that tuple and array will parse correctly
		struct tuple_type_t{
			//empty tuple is not a valid type (it is a void type)
			static constexpr auto rule = dsl::peek(LEXY_LIT("(")) >> dsl::parenthesized(dsl::list(recur_type, dsl::sep(dsl::comma)));
			static constexpr auto value = lexy::as_list<std::vector<ast::type>>;
		};
		static constexpr auto tuple_type = dsl::p<tuple_type_t>;	
		struct array_type_t{
			static constexpr auto bad_rule = recur_type + LEXY_LIT("[]");
			static constexpr auto rule = dsl::square_bracketed(recur_type);
			static constexpr auto value = lexy::construct<ast::array_type>;
		};
		static constexpr auto array_type = dsl::p<array_type_t>;
		

		static constexpr auto rule = builtin_type | tuple_type | array_type;
		static constexpr auto value = lexy::construct<ast::type>;
	};
	constexpr auto type = dsl::p<type_t>;
}
}

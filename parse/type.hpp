#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>
#include "../ast.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	static constexpr auto id = dsl::identifier(dsl::ascii::alpha_underscore, dsl::ascii::alpha_digit_underscore);

	struct type_t{
		struct void_type{
			static constexpr auto rule = LEXY_KEYWORD("void", id);
			static constexpr auto value = lexy::constant(ast::type::void_type);
		};
		struct int_type{
			static constexpr auto rule = LEXY_KEYWORD("int", id);
			static constexpr auto value = lexy::constant(ast::type::int_type);
		};
		struct float_type{
			static constexpr auto rule = LEXY_KEYWORD("float", id);
			static constexpr auto value = lexy::constant(ast::type::float_type);
		};
		struct bool_type{
			static constexpr auto rule = LEXY_KEYWORD("bool", id);
			static constexpr auto value = lexy::constant(ast::type::bool_type);
		};
		static constexpr auto rule = dsl::p<void_type> | dsl::p<int_type> | dsl::p<float_type> | dsl::p<bool_type>;
		static constexpr auto value = lexy::forward<ast::type>;
	};
	constexpr auto type = dsl::p<type_t>;
}
}

#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>
#include "../ast.hpp"
#include "id.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct literal_t{
		struct builtin_literal_t{
			struct float_t{
				static constexpr auto fr_0 = dsl::capture(LEXY_LIT("-")) >> dsl::capture(dsl::digits<>) + dsl::capture(dsl::period) + dsl::capture(dsl::digits<>);
				static constexpr auto fr_1 = dsl::peek(dsl::digits<> + dsl::period) >> dsl::capture(dsl::digits<>) + dsl::capture(dsl::period) + dsl::capture(dsl::digits<>);
				//static constexpr auto fr_1 = dsl::sign + dsl::digits<> + (LEXY_LIT("e") | LEXY_LIT("E")) + dsl::sign + dsl::digits<>;
				static constexpr auto float_rule = fr_0 | fr_1;
				static constexpr auto rule = float_rule;
				static constexpr auto value = lexy::callback<float>([]<class... Args>(const Args&... args){
						std::string val;
						auto catStr = [&](const auto& lexeme){
							val += std::string(lexeme.begin(), lexeme.end());
						};
						(catStr(args), ...);
						return std::stof(val);
				});
			};
			struct integer_t{
				static constexpr auto integer_rule = dsl::sign + dsl::integer<int>;
				static constexpr auto rule = dsl::peek(dsl::digits<>) >> dsl::peek_not(dsl::p<float_t>) >> integer_rule;
				static constexpr auto value = lexy::as_integer<int>;
			};
			struct boolean_t{
				struct true_t{
					static constexpr auto rule = LEXY_KEYWORD("true", id);
					static constexpr auto value = lexy::constant<bool>(true);
				};
				struct false_t{
					static constexpr auto rule = LEXY_KEYWORD("false", id);
					static constexpr auto value = lexy::constant<bool>(false);
				};
				static constexpr auto rule = dsl::p<true_t> | dsl::p<false_t>;
				static constexpr auto value = lexy::forward<bool>;
			};
			static constexpr auto rule = dsl::p<boolean_t> | dsl::p<float_t> | dsl::p<integer_t>;
			static constexpr auto value = lexy::construct<ast::literal>;
		};
		static constexpr auto builtin_literal = dsl::p<builtin_literal_t>;
		struct array_literal_t{
			struct array_literal_impl_t{
				static constexpr auto rule = dsl::square_bracketed(dsl::list(dsl::recurse_branch<literal_t>, dsl::sep(dsl::comma)));
				static constexpr auto value = lexy::as_list<std::vector<ast::literal>>;
			};
			static constexpr auto rule = dsl::p<array_literal_impl_t>;
			static constexpr auto value = lexy::construct<ast::literal::array_literal>;
		};
		static constexpr auto array_literal = dsl::p<array_literal_t>;
		struct tuple_literal_t{
			struct tuple_literal_impl_t{
				static constexpr auto rule = dsl::parenthesized(dsl::list(dsl::recurse_branch<literal_t>, dsl::sep(dsl::comma)));
				static constexpr auto value = lexy::as_list<std::vector<ast::literal>>;
			};
			static constexpr auto rule = dsl::p<tuple_literal_impl_t>;
			static constexpr auto value = lexy::construct<ast::literal::tuple_literal>;
		};
		static constexpr auto tuple_literal = dsl::p<tuple_literal_t>;
		static constexpr auto rule = builtin_literal | array_literal | tuple_literal;
		static constexpr auto value = lexy::construct<ast::literal>;
	};
	constexpr auto literal = dsl::p<literal_t>;
}
}

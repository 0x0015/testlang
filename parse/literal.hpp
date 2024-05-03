#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>

#include "type.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct literal_t{
		struct float_t{
			static constexpr auto fr_0 = dsl::capture(LEXY_LIT("-")) >> dsl::capture(dsl::digits<>) + dsl::capture(dsl::period) + dsl::capture(dsl::digits<>);
			static constexpr auto fr_1 = dsl::peek_not(LEXY_LIT("-")) >> dsl::capture(dsl::digits<>) + dsl::capture(dsl::period) + dsl::capture(dsl::digits<>);
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
			static constexpr auto rule = dsl::peek_not(dsl::p<float_t>) >> integer_rule;
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
		static constexpr auto rule = dsl::p<boolean_t> | dsl::p<integer_t> | dsl::p<float_t>;
		static constexpr auto value = lexy::construct<ast::function::call::literalArg>;
	};
	constexpr auto literal = dsl::p<literal_t>;
}
}

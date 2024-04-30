#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>

#include "type.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct identifier_t{
		static constexpr auto rule = dsl::identifier(dsl::ascii::alpha_digit_underscore)
			.reserve(type_t::void_type::rule, type_t::int_type::rule, type_t::float_type::rule, type_t::bool_type::rule);
		static constexpr auto value = lexy::as_string<std::string>;
	};
	constexpr auto identifier = dsl::p<identifier_t>;
}
}

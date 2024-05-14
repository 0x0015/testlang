#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>

#include "type.hpp"
#include "literal.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct identifier_t{
		static constexpr auto rule = id.reserve(type_t::builtin_type_t::void_type::rule, type_t::builtin_type_t::int_type::rule, type_t::builtin_type_t::float_type::rule, type_t::builtin_type_t::bool_type::rule, literal_t::builtin_literal_t::boolean_t::true_t::rule, literal_t::builtin_literal_t::boolean_t::false_t::rule);
		static constexpr auto value = lexy::as_string<std::string>;
	};
	constexpr auto identifier = dsl::p<identifier_t>;
}
}

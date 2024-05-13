#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	static constexpr auto id = dsl::identifier(dsl::ascii::alpha_underscore, dsl::ascii::alpha_digit_underscore);
}
}

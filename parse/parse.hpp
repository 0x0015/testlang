#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>
#include "../ast.hpp"

#include "context.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct production{
		static constexpr auto whitespace = dsl::ascii::space | dsl::newline | LEXY_LIT("\t") | (LEXY_LIT("/*") >> dsl::until(LEXY_LIT("*/"))) | (LEXY_LIT("//") >> dsl::until(dsl::newline));
		static constexpr auto rule = context;
		static constexpr auto value = lexy::forward<ast::context>;
	};
}
}

#pragma once
#include <lexy/dsl.hpp>
#include <lexy/callback.hpp>
#include "../ast.hpp"

#include "function_decl.hpp"

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct context_t{
		struct entries_t{
			static constexpr auto rule = dsl::list(function_decl);
			static constexpr auto value = lexy::as_list<std::vector<ast::function>>;
		};
		static constexpr auto entries = dsl::p<entries_t>;
		static constexpr auto rule = entries;
		static constexpr auto value = lexy::construct<ast::context>;
	};
	constexpr auto context = dsl::p<context_t>;
}
}

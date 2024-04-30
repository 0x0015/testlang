#pragma once
#include <lexy/dsl.hpp>

namespace{
namespace grammer{
	namespace dsl = lexy::dsl;

	struct Color{
		uint8_t r, g, b;
	};

	struct color_channel{
		static constexpr auto rule = dsl::integer<uint8_t>(dsl::n_digits<2, dsl::hex>);
		static constexpr auto value = lexy::forward<std::uint8_t>;
	};
	struct color{
		static constexpr auto rule = dsl::hash_sign + dsl::times<3>(dsl::p<color_channel>) + dsl::eof;
		static constexpr auto value = lexy::construct<Color>;
	};

	using production = color;
}
}

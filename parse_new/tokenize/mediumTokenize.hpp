#pragma once
#include <vector>
#include <variant>
#include "basicTokenize.hpp"

struct mediumToken{
	struct tokList{
		enum type_t{
			PAREN,
			CURL_BRACK,
			SQUARE_BRACK
		};
		static std::string_view type_t_to_string(const type_t& ty);
		type_t type;
		std::vector<mediumToken> value;
	};
	std::variant<tokList, basicToken> value;
	void print() const;

	static inline int currentRecur = 0;
	void print_internal() const;
};

std::optional<std::vector<mediumToken>> parseBrackets(const std::vector<basicToken>& toks);


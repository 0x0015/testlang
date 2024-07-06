#pragma once
#include "fileDetails.hpp"
#include <string>
#include <optional>
#include <vector>
#include <cstdint>
#include <variant>
#include "basicParse.hpp"

struct mediumToken{
	struct stringToken{
		std::string value;
	};
	struct intToken{
		int64_t value;
	};
	struct uintToken{
		uint64_t value;
	};
	struct doubleToken{
		double value;
	};
	struct floatToken{
		float value;
	};
	struct boolToken{
		bool value;
	};
	std::variant<stringToken, intToken, uintToken, doubleToken, floatToken, boolToken, std::string> value;
	fileSourceInfo file;
	void print() const;
};

std::optional<std::vector<mediumToken>> parseBasicLiterals(const std::vector<basicToken>& toks);


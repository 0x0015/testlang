#pragma once
#include <vector>
#include <iostream>
#include <optional>

#include "../ast/ast.hpp"

namespace minLang{
class parser{
public:
	std::vector<std::string> files;
	std::optional<minLang::ast::context> parseAll();
};
}

#pragma once
#include <vector>
#include <iostream>
#include <optional>

#include "../ast.hpp"

class parser{
public:
	std::vector<std::string> files;
	std::optional<ast::context> parseAll();
};

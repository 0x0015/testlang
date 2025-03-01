#pragma once
#include "../../ast/ast.hpp"
#include <optional>
#include <span>

class mediumToken;
namespace minLang{

std::optional<minLang::ast::context> parseContext(std::span<const mediumToken> tokens);
}

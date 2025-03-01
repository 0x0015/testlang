#pragma once
#include "../ast/ast.hpp"

namespace minLang{
bool checkFunctionsDefined(const ast::context& context);
bool checkConflictingFunctionDefinitions(const ast::context& context);
}

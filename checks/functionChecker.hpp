#pragma once
#include "../ast/ast.hpp"

bool checkFunctionsDefined(const ast::context& context);
bool checkConflictingFunctionDefinitions(const ast::context& context);


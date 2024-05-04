#pragma once
#include "../ast.hpp"

void addBuiltins(ast::context& context);
void addBuiltin(ast::context& context, const ast::function& func);

void addPrints(ast::context& context);
void addAssignments(ast::context& context);


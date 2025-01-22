#pragma once
#include "../ast/ast.hpp"

void addBuiltins(ast::context& context);
void addBuiltin(ast::context& context, const ast::function& func);

void addPrints(ast::context& context);
void addArithmatic(ast::context& context);
void addLogic(ast::context& context);


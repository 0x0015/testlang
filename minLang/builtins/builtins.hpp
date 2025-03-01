#pragma once
#include "../ast/ast.hpp"

void addBuiltins(minLang::ast::context& context);
void addBuiltin(minLang::ast::context& context, const minLang::ast::function& func);

void addPrints(minLang::ast::context& context);
void addArithmatic(minLang::ast::context& context);
void addLogic(minLang::ast::context& context);
void addConversions(minLang::ast::context& context);


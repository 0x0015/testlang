#pragma once
#include "../ast/ast.hpp"

//maybe rename, as it also fills in function calls with corrosponding function defs
bool checkTypeUsesValid(ast::context& context);

struct multiContextDefinedVars_t;
struct functionCallMatcher;
std::optional<ast::type> deriveExprTypeAndFill(ast::expr& expr, const multiContextDefinedVars_t& definedVars, const ast::function& func, functionCallMatcher& funcCallMatcher);

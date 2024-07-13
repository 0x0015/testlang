#pragma once
#include "../ast/ast.hpp"

//maybe rename, as it also fills in function calls with corrosponding function defs
bool checkTypeUsesValid(ast::context& context);


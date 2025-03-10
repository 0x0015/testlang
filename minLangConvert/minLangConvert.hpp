#pragma once
#include <optional>
#include "../minLang/ast/ast.hpp"
#include "../ast/ast.hpp"

std::optional<minLang::ast::expr> minLangConvertExpr(const ast::expr& exp);
std::optional<minLang::ast::block> minLangConvertBlock(const ast::block& block);
std::optional<minLang::ast::type> minLangConvertType(const ast::type& ty);
std::optional<minLang::ast::function> minLangConvertFunction(const ast::function& func);
std::optional<minLang::ast::context> minLangConvert(const ast::context& context);


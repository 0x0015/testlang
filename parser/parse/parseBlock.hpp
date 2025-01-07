#pragma once
#include "../../ast/ast.hpp"
#include <span>
#include "../parseUtil.hpp"

class mediumToken;

parseRes<ast::block> parseBlock(std::span<const mediumToken> tokens, bool includeCurlBrackets = true /*false if you don't want to look for curly brackets and want to look for statements right here*/);


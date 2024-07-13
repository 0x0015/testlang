#pragma once
#include "../ast.hpp"
#include <cstdint>
#include <unordered_map>

namespace interpreter{
	void interpret(const ast::context& context, const std::string_view entryPoint);
	struct interpreter{
		std::vector<uint8_t> stack;
		static constexpr inline unsigned int stackSize = 1024 * 1024 * 4;//4M stack
		unsigned int stackPtr;
		struct functionContext{
			//name -> stack position
			std::unordered_map<std::string, unsigned int> variablePtrs;
			std::unordered_map<std::string, unsigned int> variableSizes;
			unsigned int usedStack = 0;
		};
		std::vector<functionContext> functionExecutions;
	};
	void handleBulitin(const ast::function& func, const ast::function::call& call, interpreter& M);
}


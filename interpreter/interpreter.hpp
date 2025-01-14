#pragma once
#include "../ast/ast.hpp"
#include <cstdint>
#include <unordered_map>
#include <span>
#include "ffi.h"

namespace interpreter{
	bool interpret(const ast::context& context, const std::string_view entryPoint, std::span<const std::string> linkLibs = {});
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
		struct externalFunctionHandler{
			std::unordered_map<std::string, void*> dlHandles;
			struct funcDetails{
				void* handle;
				ffi_type rtype;
				std::vector<ffi_type> atypes;
				std::vector<ffi_type*> atypePtrs;
				ffi_cif cif;
			};
			std::unordered_map<std::string, funcDetails> funcHandles;
			void handleExternal(const ast::function& func, const ast::call& call, interpreter& M);
		};
		externalFunctionHandler externalHandler;
	};
	void handleBulitin(const ast::function& func, const ast::call& call, interpreter& M);
	std::optional<interpreter::externalFunctionHandler> loadExternalFunctions(const std::list<ast::function>& funcs, std::span<const std::string> linkLibs);
}


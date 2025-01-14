#pragma once
#include "../ast/ast.hpp"
#include <unordered_map>
#include <span>
#include "ffi.h"

namespace interpreterv2{
	bool interpret(const ast::context& context, const std::string_view entryPoint, std::span<const std::string> linkLibs = {});
	struct interpreter{
		std::unordered_map<std::string, std::vector<uint8_t>> varEntries;
		std::vector<uint8_t> returnBuffer;
		bool interpretStatement(const ast::block::statement& statement);
		std::vector<uint8_t> interpretExpr(const ast::expr& expr);
		std::vector<uint8_t> interpretCall(const ast::call& call);
		void interpretBlock(const ast::block& block);
		std::vector<uint8_t> handleBuiltinCall(const ast::call& call);
		std::vector<uint8_t> handleExternalCall(const ast::call& call);
	
		//stuff for external functions
		std::unordered_map<std::string, void*> dlHandles;
		struct funcDetails{
			void* handle;
			ffi_type rtype;
			std::vector<ffi_type> atypes;
			std::vector<ffi_type*> atypePtrs;
			ffi_cif cif;
		};
		std::unordered_map<std::string, funcDetails> funcHandles;
		bool loadExternalFunctions(const std::list<ast::function>& funcs, std::span<const std::string> linkLibs);
	};
}

OBJS	= main.cpp.o\
	  argParse.cpp.o \
	  parser/fileDetails.cpp.o \
	  parser/parseUtil.cpp.o \
	  parser/errorUtil.cpp.o \
	  parser/fileIO.cpp.o \
	  parser/parse.cpp.o \
	  parser/tokenize/basicTokenize.cpp.o \
	  parser/tokenize/mediumTokenize.cpp.o \
	  parser/parse/parseContext.cpp.o \
	  parser/parse/parseBlock.cpp.o \
	  parser/parse/parseFunction.cpp.o \
	  parser/parse/parseAlias.cpp.o \
	  parser/parse/parseType.cpp.o \
	  parser/parse/parseLiteral.cpp.o \
	  parser/parse/parseExpr.cpp.o \
	  parser/parse/parseTemplate.cpp.o \
	  ast/ast.cpp.o \
	  ast/ast_block.cpp.o \
	  ast/ast_type.cpp.o \
	  ast/ast_literal.cpp.o \
	  ast/ast_expr.cpp.o \
	  builtins/builtins.cpp.o \
	  builtins/prints.cpp.o \
	  builtins/arithmatic.cpp.o \
	  builtins/logic.cpp.o \
	  builtins/arrayOpFuncs.cpp.o \
	  builtins/convert.cpp.o \
	  checks/functionChecker.cpp.o \
	  checks/typeChecker.cpp.o \
	  checks/functionCallMatcher.cpp.o \
	  checks/assignmentChecker.cpp.o \
	  minLangConvert/minLangConvert.cpp.o \
	  minLang/parser/parse.cpp.o \
	  minLang/parser/parse/parseContext.cpp.o \
	  minLang/parser/parse/parseBlock.cpp.o \
	  minLang/parser/parse/parseFunction.cpp.o \
	  minLang/parser/parse/parseType.cpp.o \
	  minLang/parser/parse/parseLiteral.cpp.o \
	  minLang/parser/parse/parseExpr.cpp.o \
	  minLang/ast/ast.cpp.o \
	  minLang/ast/ast_block.cpp.o \
	  minLang/ast/ast_type.cpp.o \
	  minLang/ast/ast_literal.cpp.o \
	  minLang/ast/ast_expr.cpp.o \
	  minLang/builtins/builtins.cpp.o \
	  minLang/builtins/prints.cpp.o \
	  minLang/builtins/arithmatic.cpp.o \
	  minLang/builtins/logic.cpp.o \
	  minLang/builtins/convert.cpp.o \
	  minLang/checks/functionChecker.cpp.o \
	  minLang/checks/typeChecker.cpp.o \
	  minLang/checks/functionCallMatcher.cpp.o \
	  minLang/checks/assignmentChecker.cpp.o \
	  minLang/backends/util.cpp.o \
	  minLang/backends/interpreter/interpreter.cpp.o \
	  minLang/backends/interpreter/builtin.cpp.o \
	  minLang/backends/interpreter/external.cpp.o \
	  minLang/backends/cCodeGen/cCodeGen.cpp.o \
	  minLang/backends/cCodeGen/cTypes.cpp.o \
	  minLang/backends/cCodeGen/cFunctionGen.cpp.o \
	  minLang/backends/cCodeGen/cBuiltins.cpp.o


OUT	= main
CXX	= g++
CC      = gcc
CC_ACCEL = ccache
BUILD_CXX_FLAGS	 = -Wall -std=c++20 -g -Wno-reorder
BULID_CC_FLAGS   =
LINK_OPTS	 = -ldl -lffi

all: $(OBJS)
	$(CC_ACCEL) $(CXX) $(OBJS) -o $(OUT) $(LINK_OPTS)

%.cpp.o: %.cpp
	$(CC_ACCEL) $(CXX) $< $(BUILD_CXX_FLAGS) -g -c -o $@

%.c.o: %.c
	$(CC_ACCEL) $(CXX) $< $(BUILD_CXX_FLAGS) -g -c -o $@

clean:
	rm -f $(OBJS) $(OUT)

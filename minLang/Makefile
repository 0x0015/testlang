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
	  parser/parse/parseType.cpp.o \
	  parser/parse/parseLiteral.cpp.o \
	  parser/parse/parseExpr.cpp.o \
	  ast/ast.cpp.o \
	  ast/ast_block.cpp.o \
	  ast/ast_type.cpp.o \
	  ast/ast_literal.cpp.o \
	  ast/ast_expr.cpp.o \
	  builtins/builtins.cpp.o \
	  builtins/prints.cpp.o \
	  builtins/arithmatic.cpp.o \
	  builtins/logic.cpp.o \
	  builtins/convert.cpp.o \
	  checks/functionChecker.cpp.o \
	  checks/typeChecker.cpp.o \
	  checks/functionCallMatcher.cpp.o \
	  checks/assignmentChecker.cpp.o \
	  backends/interpreter/interpreter.cpp.o \
	  backends/interpreter/builtin.cpp.o \
	  backends/interpreter/external.cpp.o \
	  backends/cCodeGen/cCodeGen.cpp.o \
	  backends/cCodeGen/cTypes.cpp.o \
	  backends/cCodeGen/cFunctionGen.cpp.o \
	  backends/cCodeGen/cBuiltins.cpp.o

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

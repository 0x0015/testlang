OBJS	= main.cpp.o\
	  parse/fileDetails.cpp.o \
	  parse/parseUtil.cpp.o \
	  parse/errorUtil.cpp.o \
	  parse/fileIO.cpp.o \
	  parse/parse.cpp.o \
	  parse/tokenize/basicTokenize.cpp.o \
	  parse/tokenize/mediumTokenize.cpp.o \
	  parse/parse/parseContext.cpp.o \
	  parse/parse/parseFunction.cpp.o \
	  parse/parse/parseType.cpp.o \
	  parse/parse/parseLiteral.cpp.o \
	  ast/ast.cpp.o \
	  ast/ast_type.cpp.o \
	  ast/ast_literal.cpp.o \
	  builtins/builtins.cpp.o \
	  builtins/prints.cpp.o \
	  checks/functionChecker.cpp.o \
	  checks/typeChecker.cpp.o \
	  checks/assignmentChecker.cpp.o \
	  interpreter/interpreter.cpp.o \
	  interpreter/builtins.cpp.o


OUT	= main
CXX	= g++
CC      = gcc
BUILD_CXX_FLAGS	 = -Wall -std=c++20 -g -Wno-reorder
BULID_CC_FLAGS   =
LINK_OPTS	 = 

all: $(OBJS) $(SPIRV)
	$(CXX) $(OBJS) -o $(OUT) $(LINK_OPTS)

%.cpp.o: %.cpp
	$(CXX) $< $(BUILD_CXX_FLAGS) -g -c -o $@

%.c.o: %.c
	$(CXX) $< $(BUILD_CXX_FLAGS) -g -c -o $@

clean:
	rm -f $(OBJS) $(OUT)

OBJS	= main.cpp.o\
	  parse_new/fileDetails.cpp.o \
	  parse_new/parseUtil.cpp.o \
	  parse_new/parse.cpp.o \
	  parse_new/tokenize/basicTokenize.cpp.o \
	  parse_new/tokenize/mediumTokenize.cpp.o \
	  parse_new/parse/parseContext.cpp.o \
	  parse_new/parse/parseFunction.cpp.o \
	  parse_new/parse/parseType.cpp.o \
	  parse_new/parse/parseLiteral.cpp.o \
	  ast.cpp.o \
	  ast_type.cpp.o \
	  ast_literal.cpp.o \
	  builtins/builtins.cpp.o \
	  builtins/prints.cpp.o \
	  builtins/assignments.cpp.o \
	  functionChecker.cpp.o \
	  typeChecker.cpp.o \
	  interpreter/interpreter.cpp.o \
	  interpreter/builtins.cpp.o


OUT	= main
CXX	= g++
CC      = gcc
BUILD_CXX_FLAGS	 = -Wall -std=c++20 -g -Ilexy/include
BULID_CC_FLAGS   =
LINK_OPTS	 = lexy/src/input/file.cpp

all: $(OBJS) $(SPIRV)
	$(CXX) $(OBJS) -o $(OUT) $(LINK_OPTS)

%.cpp.o: %.cpp
	$(CXX) $< $(BUILD_CXX_FLAGS) -g -c -o $@

%.c.o: %.c
	$(CXX) $< $(BUILD_CXX_FLAGS) -g -c -o $@

clean:
	rm -f $(OBJS) $(OUT)

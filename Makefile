OBJS	= main.cpp.o\
	  ast.cpp.o \
	  builtins/builtins.cpp.o \
	  builtins/prints.cpp.o \
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

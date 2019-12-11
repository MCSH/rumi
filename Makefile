
TESTCOUNT = 1
OBJS = lex.o parse.o compiler.o
CODE_GEN = lex.cpp parse.cpp parse.hpp
CC = clang++
COMPILER_BASE_FLAGS = -w -g
COMPILER_LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`
COMPILER_OBJECT_FLAGS = -c $(COMPILER_BASE_FLAGS)
COMPILER_OBJECT_LLVM_FLAGS = $(COMPILER_OBJECT_FLAGS) $(COMPILER_LLVM_FLAGS)
COMPILER_EXEC_FLAGS = $(COMPILER_BASE_FLAGS)
COMPILER_EXEC_LLVM_FLAGS = $(COMPILER_EXEC_FLAGS) $(COMPILER_LLVM_FLAGS)

all:	rum rumi

rum:	compiler rum.cpp
	$(CC) $(COMPILER_EXEC_FLAGS) $(OBJS) rum.cpp -o rum

rumi:	compiler rumi.cpp
	$(CC) $(COMPILER_EXEC_FLAGS) $(OBJS) rumi.cpp -o rumi

compiler: $(OBJS)
	true

compiler.o:	compiler.cpp compiler.h
	$(CC) $(COMPILER_OBJECT_FLAGS) compiler.cpp

lex.o:	lex.cpp
	$(CC) $(COMPILER_OBJECT_FLAGS) lex.cpp

lex.cpp:	lex.l
	flex -o lex.cpp lex.l

parse.o: parse.cpp
	$(CC) $(COMPILER_OBJECT_FLAGS) parse.cpp

parse.cpp:	parse.y
	bison -d -l -o parse.cpp parse.y

tests:	rum
	number=1 ; while [[ $$number -le $(TESTCOUNT) ]] ; do \
		./rum tests/$$number.rum ; \
		((number = number + 1)) ; \
	done # TODO change this to actually run tests

clean:
	rm -f rum rumi $(OBJS) $(CODE_GEN) || true

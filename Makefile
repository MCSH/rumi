
NUMTESTS = 6
OBJS = lex.o parse.o compiler.o
BASE_HEADS = node.h type.h
CODE_GEN = lex.cpp parse.cpp parse.hpp
CC = clang++
COMPILER_BASE_FLAGS = -w -g
COMPILER_LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`
COMPILER_OBJECT_FLAGS = -c $(COMPILER_BASE_FLAGS)
COMPILER_OBJECT_LLVM_FLAGS = $(COMPILER_OBJECT_FLAGS) $(COMPILER_LLVM_FLAGS)
COMPILER_EXEC_FLAGS = $(COMPILER_BASE_FLAGS)
COMPILER_EXEC_LLVM_FLAGS = $(COMPILER_EXEC_FLAGS) $(COMPILER_LLVM_FLAGS)

all:	rum rumi

rum:	$(OBJS) rum.cpp codegen.o
	$(CC) $(COMPILER_EXEC_LLVM_FLAGS) $(OBJS) codegen.o rum.cpp -o rum

rumi:	$(OBJS) rumi.cpp
	$(CC) $(COMPILER_EXEC_FLAGS) $(OBJS) rumi.cpp -o rumi

compiler.o:	compiler.cpp compiler.h $(BASE_HEADS)
	$(CC) $(COMPILER_OBJECT_FLAGS) compiler.cpp

lex.o:	lex.cpp parse.hpp
	$(CC) $(COMPILER_OBJECT_FLAGS) lex.cpp

codegen.o:	codegen.cpp codegen.h $(BASE_HEADS)
	$(CC) $(COMPILER_OBJECT_LLVM_FLAGS) codegen.cpp

lex.cpp:	lex.l
	flex -o lex.cpp lex.l

parse.o: parse.cpp
	$(CC) $(COMPILER_OBJECT_FLAGS) parse.cpp

parse.hpp: parse.cpp parse.o

parse.cpp:	parse.y $(BASE_HEADS)
	bison -d -l -o parse.cpp parse.y

.PHONY: tests
tests:	rum
	number=1 ; while [[ $$number -le $(NUMTESTS) ]] ; do \
		echo "==============" ; \
		echo tests/$$number.rum ; \
		cat -n tests/$$number.rum ; \
		echo ; \
		./rum tests/$$number.rum ; \
		lli tests/$$number.rum.o ; \
		((number = number + 1)) ; \
	done

clean:
	rm -f rum rumi $(OBJS) $(CODE_GEN) codegen.o tests/*.o || true

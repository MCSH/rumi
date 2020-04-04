
export RUMI_PATH=$(shell pwd)/rumlib
NUMTESTS = 4
CC = clang++
COMPILER_BASE_FLAGS = -w -g
COMPILER_LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`
COMPILER_OBJECT_FLAGS = -c $(COMPILER_BASE_FLAGS)
COMPILER_EXEC_LLVM_FLAGS = $(COMPILER_BASE_FLAGS) $(COMPILER_LLVM_FLAGS)

BIN = rum
BINI = rumi

BUILD_DIR = ./build

CPP = Context.cpp codegen.cpp compiler.cpp BlockContext.cpp $(wildcard nodes/**/*.cpp) $(wildcard nodes/*.cpp) lex.cpp parse.cpp

OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)

DEP = $(OBJ:%.o=%.d)

all:	$(BIN) $(BINI)

$(BUILD_DIR)/$(BIN) : $(OBJ)
	mkdir -p $(@D)

	$(CC) $(COMPILER_OBJECT_FLAGS) $^ -o $@

-include $(DEP)


$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CC) $(COMPILER_OBJECT_FLAGS) -MMD $< -o $@

.PHONY: clean
clean:
	-rm -f $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP) tests/*.o $(BIN) $(BINI) parse.cpp lex.cpp $(BUILD_DIR)/$(BIN).o $(BUILD_DIR)/$(BINI).o  $(BUILD_DIR)/$(BIN).d $(BUILD_DIR)/$(BINI).d

$(BIN):	$(OBJ) $(BUILD_DIR)/$(BIN).o
	$(CC) $(COMPILER_EXEC_LLVM_FLAGS) $(OBJ) $(BUILD_DIR)/$(BIN).o -o $(BIN)

$(BINI): $(OBJ) $(BUILD_DIR)/$(BINI).o
	$(CC) $(COMPILER_EXEC_LLVM_FLAGS) $(OBJ) $(BUILD_DIR)/$(BINI).o -o $(BINI)

lex.cpp: lex.l parse.hpp
	flex -o lex.cpp lex.l

parse.hpp: parse.cpp

parse.cpp: parse.y
	bison -d -l -o parse.cpp parse.y

.PHONY: tests
tests:  rum rumi
	rm -f tests/*.o || true
	env | grep RUM
	number=1 ; while [[ $$number -le $(NUMTESTS) ]] ; do \
		echo "==============" ; \
		echo tests/$$number.rum ; \
		cat -n tests/$$number.rum ; \
		echo ; \
		./rum -l tests/$$number.rum ; \
		lli tests/$$number.rum.o ; \
		((number = number + 1)) ; \
	done



CC = clang++

COMPILER_BASE_FLAGS = -w -g
COMPILER_LLVM_FLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs core`
COMPILER_OBJECT_FLAGS = -c $(COMPILER_BASE_FLAGS)
COMPILER_EXEC_LLVM_FLAGS = $(COMPILER_BASE_FLAGS) $(COMPILER_LLVM_FLAGS)

BIN = rum

BUILD_DIR = ./build

CPP = lex.cpp parse.cpp helper.cpp

OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)

DEP = $(OBJ:%.o=%.d)

all:	$(BIN)

$(BUILD_DIR)/$(BIN) : $(OBJ)
	mkdir -p $(@D)

	$(CC) $(COMPILER_OBJECT_FLAGS) $^ -o $@

-include $(DEP)


$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CC) $(COMPILER_OBJECT_FLAGS) -MMD $< -o $@

.PHONY: clean
clean:
	-rm -f $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP) $(BIN) parse.cpp lex.cpp $(BUILD_DIR)/$(BIN).o $(BUILD_DIR)/$(BIN).d

$(BIN):	$(OBJ) $(BUILD_DIR)/$(BIN).o
	$(CC) $(COMPILER_EXEC_LLVM_FLAGS) $(OBJ) $(BUILD_DIR)/$(BIN).o -o $(BIN)

lex.cpp: lex.l parse.hpp
	flex -o lex.cpp lex.l

parse.hpp: parse.cpp

parse.cpp: parse.y
	bison -d -l -o parse.cpp parse.y

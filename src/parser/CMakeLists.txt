
# add_library(parser parser.cpp IdParser.cpp KeywordParser.cpp NumberParser.cpp SymbolParser.cpp FunctionParser.cpp DefineParser.cpp TypeParser.cpp ValueParser.cpp ReturnParser.cpp StatementParser.cpp VariableValueParser.cpp IfParser.cpp ExpressionParser.cpp)

file(GLOB SRC "*.cpp")
add_library(parser ${SRC})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

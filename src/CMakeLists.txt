
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(base base.cpp LLContext.cpp CC.cpp Source.cpp)

add_subdirectory(parser)
add_subdirectory(ast)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

execute_process(COMMAND llvm-config --libs all
                OUTPUT_VARIABLE llvm_libs)

string(REGEX REPLACE "\n$" "" llvm_libs "${llvm_libs}")

# To include the config file
configure_file(RumiConfig.h.in RumiConfig.h)
target_include_directories(base PUBLIC "${PROJECT_BINARY_DIR}/src/")

target_link_libraries(base PUBLIC parser ast)

add_executable(rum rum.cpp)
target_link_libraries(rum PUBLIC ${llvm_libs} base)
add_executable(rumi rumi.cpp)
target_link_libraries(rumi PUBLIC base)

find_package(Clang REQUIRED CONFIG)

message(STATUS "Found ClangConfig.cmake in: ${Clang_DIR}")

add_executable(c2rumi c2rumi.cpp)
target_link_libraries(c2rumi PUBLIC clang libclang)

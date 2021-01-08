
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(base base.cpp LLContext.cpp)

add_subdirectory(parser)
add_subdirectory(ast)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# llvm_map_components_to_libnames(llvm_libs support core irreader mcjit)
llvm_map_components_to_libnames(llvm_libs
  core 
  ExecutionEngine
  Interpreter
  MC
  Support
  nativecodegen
  mcjit
  asmparser
  objcarcopts
  scalaropts
  transformutils
  irreader
  x86asmparser
  codegen
  )

# To include the config file
configure_file(RumiConfig.h.in RumiConfig.h)
target_include_directories(base PUBLIC "${PROJECT_BINARY_DIR}/src/")

target_link_libraries(base PUBLIC parser ast)

add_executable(rum rum.cpp)
target_link_libraries(rum PUBLIC base ${llvm_libs})
add_executable(rumi rumi.cpp)
target_link_libraries(rumi PUBLIC base)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")


add_library(base base.cpp)

add_subdirectory(parser)
add_subdirectory(ast)

# To include the config file
configure_file(RumiConfig.h.in RumiConfig.h)
target_include_directories(base PUBLIC "${PROJECT_BINARY_DIR}/src/")

target_link_libraries(base PUBLIC parser ast)

add_executable(rum rum.cpp)
target_link_libraries(rum PUBLIC base)
add_executable(rumi rumi.cpp)
target_link_libraries(rumi PUBLIC base)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_library(board board.cpp)
target_include_directories(board PUBLIC .)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ..)
add_executable(FastFeud main.cpp board.cpp ab_node.cpp alphabeta.cpp)

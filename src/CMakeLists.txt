add_library(board board.cpp)
target_include_directories(board PUBLIC .)

add_executable(FastFeud main.cpp board.cpp ab_node.cpp alphabeta.cpp)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
  board_test
  board_test.cpp
)

target_link_libraries(
  board_test
  GTest::gtest_main
  board
)

include(GoogleTest)
gtest_discover_tests(
  board_test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
#set_tests_properties(board_test PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

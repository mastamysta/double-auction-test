enable_testing()

add_executable(test_book testbook.cpp)
target_link_libraries(test_book gtest gtest_main uuid book)

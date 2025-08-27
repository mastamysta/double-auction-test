add_library(server INTERFACE)
target_include_directories(server INTERFACE ./include)

add_executable(server_test ./src/server_test.cpp)
target_link_libraries(server_test protocol server)

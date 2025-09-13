add_library(netserver INTERFACE)
add_executable(client_test ./src/client_test.cpp)
add_executable(server_test ./src/server_test.cpp)

target_include_directories(netserver INTERFACE ./include)
target_link_libraries(client_test netserver protocol)
target_link_libraries(server_test netserver protocol)


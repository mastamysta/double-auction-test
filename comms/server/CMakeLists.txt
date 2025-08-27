add_library(netserver INTERFACE)
target_include_directories(netserver INTERFACE ./include)

add_executable(netserver_test ./src/server_test.cpp)
target_link_libraries(netserver_test protocol netserver)

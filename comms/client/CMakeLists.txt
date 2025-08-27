add_library(client INTERFACE)
target_include_directories(client INTERFACE ./include)

add_executable(client_test ./src/client_test.cpp)
target_link_libraries(client_test protocol client)

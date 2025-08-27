add_library(netclient INTERFACE)
target_include_directories(netclient INTERFACE ./include)

add_executable(netclient_test ./src/client_test.cpp)
target_link_libraries(netclient_test protocol netclient)

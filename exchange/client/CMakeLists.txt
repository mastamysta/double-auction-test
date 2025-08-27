add_library(exchange_client INTERFACE)
target_include_directories(exchange_client INTERFACE ./include)

add_executable(exchange_client_test ./src/exchange_client_test.cpp)
target_link_libraries(exchange_client_test exchange_client)

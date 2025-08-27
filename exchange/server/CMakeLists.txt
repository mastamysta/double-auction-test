add_executable(exchange_server ./src/exchange_server.cpp)
target_link_libraries(exchange_server netserver protocol book)

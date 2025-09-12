cmake_minimum_required(VERSION 3.22)
project(minimarket)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_subdirectory(book)
add_subdirectory(comms)
add_subdirectory(protocol)
add_subdirectory(exchange)
add_subdirectory(agents)

add_subdirectory(extern)
add_subdirectory(tests)

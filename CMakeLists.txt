cmake_minimum_required(VERSION 3.22)
project(minimarket)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_subdirectory(book)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(extern)
add_subdirectory(tests)

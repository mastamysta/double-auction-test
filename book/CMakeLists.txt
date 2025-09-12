add_library(book INTERFACE)
target_include_directories(book INTERFACE ./include)
target_link_libraries(book INTERFACE -luuid)

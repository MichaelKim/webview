cmake_minimum_required (VERSION 3.1)
project (basic)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/webview)
add_executable(basic basic.cpp)
target_link_libraries(basic PUBLIC webview)

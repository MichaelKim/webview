cmake_minimum_required (VERSION 3.1)
project (callback)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/webview)
add_executable(callback callback.cpp)
target_link_libraries(callback PUBLIC webview)

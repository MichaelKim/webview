cmake_minimum_required(VERSION 3.8)
project(clipboard LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR}/webview)

configure_file(index.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)

set(CMAKE_WIN32_EXECUTABLE ${WIN32})

add_executable(clipboard clipboard.cpp)
target_compile_features(clipboard PRIVATE cxx_std_17)
target_link_libraries(clipboard PRIVATE webview)

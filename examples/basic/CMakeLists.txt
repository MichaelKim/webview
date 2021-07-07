cmake_minimum_required (VERSION 3.8)
project (basic)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR}/webview)

set(CMAKE_WIN32_EXECUTABLE ${WIN32})

add_executable(basic basic.cpp)
target_compile_features(basic PRIVATE cxx_std_17)
target_link_libraries(basic PRIVATE webview)

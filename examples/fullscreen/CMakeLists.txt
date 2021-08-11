cmake_minimum_required(VERSION 3.8)
project(fullscreen LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR}/webview)

set(CMAKE_WIN32_EXECUTABLE ${WIN32})

add_executable(fullscreen fullscreen.cpp)
target_compile_features(fullscreen PRIVATE cxx_std_17)
target_link_libraries(fullscreen PRIVATE webview)

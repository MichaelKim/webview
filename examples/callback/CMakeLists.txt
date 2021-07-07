cmake_minimum_required (VERSION 3.8)
project (callback)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR}/webview)

# Copy index.html to build directory
configure_file(index.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)

set(CMAKE_WIN32_EXECUTABLE ${WIN32})

add_executable(callback callback.cpp)
target_compile_features(callback PRIVATE cxx_std_17)
target_link_libraries(callback PRIVATE webview)

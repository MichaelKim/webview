cmake_minimum_required (VERSION 3.8)
project (basic)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
	set(CMAKE_WIN32_EXECUTABLE YES)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/webview)
add_executable(basic basic.cpp)
target_link_libraries(basic PUBLIC webview)

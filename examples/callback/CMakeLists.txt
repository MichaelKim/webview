cmake_minimum_required (VERSION 3.8)
project (callback)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
	set(CMAKE_WIN32_EXECUTABLE YES)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/webview)
add_executable(callback callback.cpp)
target_link_libraries(callback PUBLIC webview)

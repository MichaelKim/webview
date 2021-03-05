cmake_minimum_required(VERSION 3.1)
project(webview VERSION 0.1 DESCRIPTION "The WebView component of Soundux")

file(GLOB src
    "src/*.cpp"
    "src/*/*.cpp"
    "src/*/*/*.cpp"
)

add_library(webview STATIC ${src})

if (WIN32)
    target_compile_options(webview PRIVATE /W4)
    
    find_program(NUGET_EXE NAMES nuget)
	if(NOT NUGET_EXE)
		message("NUGET.EXE not found.")
		message(FATAL_ERROR "Please install this executable, and run CMake again.")
	endif()

	exec_program(${NUGET_EXE}
    	ARGS install "Microsoft.Web.WebView2" -Version 1.0.790-prerelease -ExcludeVersion -OutputDirectory ${CMAKE_BINARY_DIR}/packages)
	exec_program(${NUGET_EXE}
    	ARGS install "Microsoft.Windows.ImplementationLibrary" -ExcludeVersion -OutputDirectory ${CMAKE_BINARY_DIR}/packages)

    target_link_libraries(webview INTERFACE ${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2/build/native/Microsoft.Web.WebView2.targets ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.ImplementationLibrary/build/native/Microsoft.Windows.ImplementationLibrary.targets)
    target_include_directories(webview SYSTEM PUBLIC ${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2/build/native/include ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.ImplementationLibrary/include)
elseif(UNIX)
    target_compile_options(webview INTERFACE -Wall -Wextra -Werror -pedantic -Wno-unused-lambda-capture)
    
    find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.0)

    target_link_libraries(webview INTERFACE ${GTK3_LIBRARIES} ${WEBKIT2_LIBRARIES})
    target_include_directories(webview SYSTEM PUBLIC ${GTK3_INCLUDE_DIRS} ${WEBKIT2_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})
endif()

target_include_directories(webview SYSTEM PUBLIC "src/")
target_include_directories(webview SYSTEM PUBLIC "lib/json/single_include/nlohmann")

target_compile_features(webview PUBLIC cxx_std_17)
set_target_properties(webview PROPERTIES CMAKE_CXX_STANDARD 17)
set_target_properties(webview PROPERTIES CMAKE_CXX_EXTENSIONS Off)
set_target_properties(webview PROPERTIES CMAKE_CXX_STANDARD_REQUIRED On)

set_target_properties(webview PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(webview PROPERTIES PROJECT_NAME ${PROJECT_NAME})
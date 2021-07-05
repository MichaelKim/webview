cmake_minimum_required (VERSION 3.15)
project(webview VERSION 1.0.0 LANGUAGES CXX)
include(cmake/Settings.cmake)

option(WEBVIEW_USE_EDGE "Enables Edge Chromium for Windows" ON)

# For Edge Chromium (WEBVIEW_EDGE) only
set(WEBVIEW_WEBVIEW2_VERSION 1.0.864.35)
set(WEBVIEW_WIL_VERSION 1.0.210204.1)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

add_library(project_warnings INTERFACE)
include(cmake/Warnings.cmake)
set_project_warnings(project_warnings)

include(cmake/StaticAnalyzers.cmake)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(APPLE)
	set(WEBVIEW_COMPILE_DEFS WEBVIEW_MAC=1)

	find_library(COCOA_LIBRARY Cocoa)
	find_library(WEBKIT_LIBRARY Webkit)
	set(WEBVIEW_LIBS ${COCOA_LIBRARY} ${WEBKIT_LIBRARY})
	# enable_language(OBJCXX)
	set(WEBVIEW_COMPILE_OPTS "-ObjC++")
elseif(WIN32 AND WEBVIEW_USE_EDGE)
	set(WEBVIEW_COMPILE_DEFS WEBVIEW_EDGE=1)

	find_program(NUGET nuget REQUIRED)
	configure_file(cmake/packages.config.in ${CMAKE_BINARY_DIR}/packages.config)
	execute_process(COMMAND 
		${NUGET} restore packages.config -SolutionDirectory ${CMAKE_BINARY_DIR}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
	set(WEBVIEW_COMPILE_INCS
		${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2.${WEBVIEW_WEBVIEW2_VERSION}/build/native/include
		${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.ImplementationLibrary.${WEBVIEW_WIL_VERSION}/include)

	set(WEBVIEW_LIBS ${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2.${WEBVIEW_WEBVIEW2_VERSION}/build/native/x64/WebView2LoaderStatic.lib user32.lib Version.lib Advapi32.lib Shell32.lib)
elseif(WIN32)
	set(WEBVIEW_COMPILE_DEFS WEBVIEW_WIN=1)

	set(WEBVIEW_LIBS "WindowsApp.lib user32.lib kernel32.lib")
else()
	set(WEBVIEW_COMPILE_DEFS WEBVIEW_GTK=1)

	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.0)
	set(WEBVIEW_COMPILE_INCS ${GTK3_INCLUDE_DIRS} ${WEBKIT2_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})
	set(WEBVIEW_LIBS ${GTK3_LIBRARIES} ${WEBKIT2_LIBRARIES})
endif()

# include(CMakePackageConfigHelpers)

target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR} ${WEBVIEW_COMPILE_INCS})
target_compile_definitions(${PROJECT_NAME} INTERFACE ${WEBVIEW_COMPILE_DEFS})
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
target_compile_options(${PROJECT_NAME} INTERFACE ${WEBVIEW_COMPILE_OPTS})
target_link_libraries(${PROJECT_NAME} INTERFACE project_options project_warnings ${WEBVIEW_LIBS})

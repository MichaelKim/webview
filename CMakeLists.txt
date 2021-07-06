cmake_minimum_required (VERSION 3.15)
project(webview VERSION 1.0.0 LANGUAGES CXX)
include(cmake/Settings.cmake)

if(WIN32)
	option(WEBVIEW_USE_EDGE "Enables Edge Chromium for Windows" ON)
endif()

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

add_library(project_warnings INTERFACE)
include(cmake/Warnings.cmake)
set_project_warnings(project_warnings)

include(cmake/StaticAnalyzers.cmake)
include(cmake/NuGet.cmake)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(APPLE)
	set(WEBVIEW_COMPILE_DEFS WEBVIEW_MAC)

	# Add Cocoa
	find_library(COCOA_LIBRARY Cocoa)
	find_library(WEBKIT_LIBRARY Webkit)
	set(WEBVIEW_LIBS ${COCOA_LIBRARY} ${WEBKIT_LIBRARY})

	# Enable Objective C++
	# enable_language(OBJCXX)
	set(WEBVIEW_COMPILE_OPTS "-ObjC++")
elseif(WIN32 AND WEBVIEW_USE_EDGE)
	set(WEBVIEW_COMPILE_DEFS WEBVIEW_EDGE)
	set(WEBVIEW_WEBVIEW2_VERSION 1.0.864.35)
	set(WEBVIEW_WIL_VERSION 1.0.210204.1)	
	
	option(WEBVIEW_STATIC_WEBVIEW2 "Statically link the WebView2 loader library (WEBVIEW_EDGE only)" OFF)

	set(WEBVIEW_WEBVIEW2_PATH ${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2.${WEBVIEW_WEBVIEW2_VERSION}/build/native)

	# Add NuGet packages
	install_nuget(cmake/packages-edge.config.in)
	set(WEBVIEW_COMPILE_INCS
		${WEBVIEW_WEBVIEW2_PATH}/include
		${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.ImplementationLibrary.${WEBVIEW_WIL_VERSION}/include
	)

	# Statically link WebView2
	if(WEBVIEW_STATIC_WEBVIEW2)
		set(WEBVIEW_LIBS ${WEBVIEW_WEBVIEW2_PATH}/x64/WebView2LoaderStatic.lib version.lib)
	else()
		set(WEBVIEW_LIBS ${WEBVIEW_WEBVIEW2_PATH}/x64/WebView2Loader.dll.lib)
		configure_file(
			${WEBVIEW_WEBVIEW2_PATH}/x64/WebView2Loader.dll
			${CMAKE_BINARY_DIR}/WebView2Loader.dll
			COPYONLY
		)
	endif()
elseif(WIN32)
	set(WEBVIEW_COMPILE_DEFS WEBVIEW_WIN)
	set(WEBVIEW_CPPWINRT_VERSION 2.0.210629.4)

	# Add NuGet packages
	install_nuget(cmake/packages-win.config.in)

	# Generate C++/WinRT headers
	execute_process(
		COMMAND ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.CppWinRT.${WEBVIEW_CPPWINRT_VERSION}/bin/cppwinrt.exe -in sdk
	)
else()
	set(WEBVIEW_COMPILE_DEFS WEBVIEW_GTK)

	# Add GTK
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

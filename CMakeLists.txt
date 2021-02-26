cmake_minimum_required (VERSION 3.8)
project (webview)

add_subdirectory(lib/fmt)

if(WIN32)
	set(WEBVIEW_COMPILE_DEFS "-DWEBVIEW_EDGE=1 -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS=1")
	
	# Thanks to flutter for this, Fuck you Microsoft for this abomination!
	find_program(NUGET_EXE NAMES nuget)
	if(NOT NUGET_EXE)
		message("NUGET.EXE not found.")
		message(FATAL_ERROR "Please install this executable, and run CMake again.")
	endif()

	exec_program(${NUGET_EXE}
    	ARGS install "Microsoft.Web.WebView2" -Version 1.0.790-prerelease -ExcludeVersion -OutputDirectory ${CMAKE_BINARY_DIR}/packages)
	exec_program(${NUGET_EXE}
    	ARGS install "Microsoft.Windows.ImplementationLibrary" -ExcludeVersion -OutputDirectory ${CMAKE_BINARY_DIR}/packages)
	
	set(WEBVIEW_LIBS ${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2/build/native/Microsoft.Web.WebView2.targets ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.ImplementationLibrary/build/native/Microsoft.Windows.ImplementationLibrary.targets)
	set(WEBVIEW_COMPILE_INCS ${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2/build/native/include ${CMAKE_BINARY_DIR}/packages/Microsoft.Windows.ImplementationLibrary/include)
else()
	set(WEBVIEW_COMPILE_DEFS "-DWEBVIEW_GTK=1")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.0)
	set(WEBVIEW_COMPILE_INCS ${GTK3_INCLUDE_DIRS} ${WEBKIT2_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})
	set(WEBVIEW_LIBS ${GTK3_LIBRARIES} ${WEBKIT2_LIBRARIES})
endif()

add_library(webview INTERFACE)
target_include_directories(webview INTERFACE ${PROJECT_SOURCE_DIR} ${WEBVIEW_COMPILE_INCS})
target_compile_definitions(webview INTERFACE ${WEBVIEW_COMPILE_DEFS})
target_compile_features(webview INTERFACE cxx_std_17)
target_compile_options(webview INTERFACE ${WEBVIEW_COMPILE_OPTS})
target_link_libraries(webview INTERFACE ${WEBVIEW_LIBS} fmt)
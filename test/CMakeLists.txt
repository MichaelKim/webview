cmake_minimum_required(VERSION 3.8)
project(test LANGUAGES CXX)
enable_testing()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/webview)

set(CMAKE_WIN32_EXECUTABLE ${WIN32})

function(AddTest testname)
  add_executable(${testname} ${testname}.cpp)
  target_compile_features(${testname} PRIVATE cxx_std_17)
  target_link_libraries(${testname} PRIVATE webview)
  add_test(${testname} ${testname})
endfunction()

AddTest(test-default)
AddTest(test-callback)
AddTest(test-eval)
AddTest(test-navigate-data)

configure_file(local.html ${CMAKE_BINARY_DIR}/local.html COPYONLY)
AddTest(test-navigate-hosted)

# Edge Legacy can't navigate to local files
if((NOT WIN32) OR WEBVIEW_USE_EDGE)
  AddTest(test-navigate-local)
endif()

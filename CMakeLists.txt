cmake_minimum_required(VERSION 3.15)
include(cmake/warnings.cmake)

project(slo CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_library(slo INTERFACE)

set_property(TARGET slo PROPERTY CXX_STANDARD 23)

target_include_directories(slo INTERFACE 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)


option(BUILD_TESTING "Enable tests" ON)
option(ENABLE_SANITIZERS "Enable asan and ubsan" OFF)
option(ENABLE_COVERAGE "Enable coverage instrumentation" OFF)
option(ENABLE_BENCHMARK "Enable benchmarks" OFF)

if(NOT BUILD_TESTING STREQUAL OFF)
  message(STATUS "Building unit tests")

  enable_testing()
  add_executable(slo_test "")
  
  enable_warnings(slo_test)

  find_package(GTest REQUIRED)
  target_link_libraries(slo_test PRIVATE slo)
  target_link_libraries(slo_test PRIVATE GTest::gtest GTest::gtest_main GTest::gmock)

  include(GoogleTest)
  gtest_discover_tests(slo_test)
  
  add_subdirectory(test)

  if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(slo_test PRIVATE -g -O0 --coverage -fprofile-arcs -ftest-coverage -fprofile-abs-path)
      target_link_libraries(slo_test PRIVATE gcov)
      message(STATUS "Instrumenting for coverage")
    else()
      message(FATAL_ERROR "Currently only GCC is supported for coverage instrumentation.")
    endif()
  endif()

  if(ENABLE_SANITIZERS AND WIN32)
    message(STATUS "Enabling sanitizers")
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      target_compile_options(slo_test INTERFACE -fsanitize=address)
      target_link_options(slo_test INTERFACE -fsanitize=address)
    endif()
  elseif(ENABLE_SANITIZERS)
    message(STATUS "Enabling sanitizers")
    target_compile_options(slo_test INTERFACE -fsanitize=address,undefined)
    target_link_options(slo_test INTERFACE -fsanitize=address,undefined)
  endif()
endif()

if (ENABLE_BENCHMARK)
  message(STATUS "Building runtime benchmarks")
  add_executable(slo_benchmark "")
  add_subdirectory(benchmark)
  find_package(benchmark REQUIRED)
  target_link_libraries(slo_benchmark PRIVATE benchmark::benchmark_main)
  target_link_libraries(slo_benchmark PRIVATE slo)
endif()

## headers
set_target_properties(slo PROPERTIES PUBLIC_HEADER "include/slo")
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
        DESTINATION include)

## binaries
install(TARGETS ${TARGET}
        ARCHIVE DESTINATION lib # Windows import libraries (.lib)
        RUNTIME DESTINATION bin # Windows DLLs
        LIBRARY DESTINATION lib) # shared libraries (ie Linux .so)

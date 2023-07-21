cmake_minimum_required(VERSION 3.11)
project(troll_string)

# dependencies
Include(FetchContent)

FetchContent_Declare(
  etl
  GIT_REPOSITORY https://github.com/ETLCPP/etl
  GIT_TAG        20.35.11
)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_Declare(
  fpm
  GIT_REPOSITORY https://github.com/MikeLankamp/fpm.git
  GIT_TAG        19ad7a3336e2ec0d7faa9da5232398873b555cfb
)

FetchContent_MakeAvailable(etl)
FetchContent_MakeAvailable(Catch2)
FetchContent_MakeAvailable(fpm)

option(BUILD_TESTS "Build unit tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if ((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

configure_file(${CMAKE_SOURCE_DIR}/CTestCustom.cmake ${CMAKE_BINARY_DIR} @ONLY)

# build test
if (BUILD_TESTS)
  enable_testing()
  add_executable(
    troll_util_tests
    tests/test_format.cpp
    tests/test_format_scan.cpp
  )
  add_test(troll_util_tests troll_util_tests)

  target_include_directories(troll_util_tests PRIVATE include)
  target_link_libraries(troll_util_tests PRIVATE Catch2::Catch2WithMain)
  target_link_libraries(troll_util_tests PRIVATE etl::etl)
  target_link_libraries(troll_util_tests PRIVATE fpm)

  add_custom_target(test_verbose COMMAND ${CMAKE_CTEST_COMMAND} --verbose)
endif()

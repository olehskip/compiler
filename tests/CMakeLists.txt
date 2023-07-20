cmake_minimum_required(VERSION 3.0)
project(CompilerTests)
set(GTEST_PATH ${CMAKE_SOURCE_DIR}/submodules/googletest)

add_subdirectory(${GTEST_PATH} build)
include(GoogleTest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(lexical_analyzer_test lexical_analyzer_test.cpp)
target_link_libraries(lexical_analyzer_test GTest::gtest_main ${COMPILER_LIB_OUTPUT})
target_include_directories(lexical_analyzer_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

gtest_discover_tests(lexical_analyzer_test)

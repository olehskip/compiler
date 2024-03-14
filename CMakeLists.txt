cmake_minimum_required(VERSION 3.15)
project(Compiler)

set(CMAKE_CXX_STANDARD 20)
enable_testing()

include(src/CMakeLists.txt)
add_subdirectory(tests)
add_subdirectory(src/std)

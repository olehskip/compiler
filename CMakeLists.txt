cmake_minimum_required(VERSION 3.15)
project(Compiler)

set(CMAKE_CXX_STANDARD 20)
add_compile_options("-Wall" "-Wextra" "-Wpedantic" "-O0" "-g")
enable_testing()

include(src/CMakeLists.txt)
add_subdirectory(tests)

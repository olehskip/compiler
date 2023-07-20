cmake_minimum_required(VERSION 3.0)

project(Compiler)

set(CMAKE_CXX_STANDARD 20)
add_compile_options("-Wall" "-Wextra" "-Wextra" "-O0" "-g")

enable_testing()

include(src/CMakeLists.txt)
add_subdirectory(tests)
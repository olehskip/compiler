# cmake_minimum_required(VERSION 3.0)

# project(Compiler)

# set(CMAKE_CXX_STANDARD 20)
# enable_testing()

set(COMPILER_LIB_OUTPUT compiler_lib)
set(COMPILER_OUTPUT compiler_output)
set(MAGIC_ENUM_INCLUDES "${PROJECT_SOURCE_DIR}/submodules/magic_enum/include")


set(SOURCES 
	src/lexical_analyzer/lexical_analyzer.cpp
	src/lexical_analyzer/thompson_constructor.cpp
    src/syntax_analyzer.cpp
    src/parser_utils.cpp
    src/x64_nasm_generator.cpp
    src/IR/code_generator.cpp
    src/IR/value.cpp
    src/IR/instructions.cpp
    src/IR/symbol_table.cpp
    src/IR/procedure.cpp
)

add_library(${COMPILER_LIB_OUTPUT} STATIC ${SOURCES})
if(NOT DEFINED LOG_EXIT_FUNC)
    set(LOG_EXIT_FUNC STD_EXIT)
endif()
message ("LOG_EXIT_FUNC = " ${LOG_EXIT_FUNC})
target_compile_options(${COMPILER_LIB_OUTPUT} PUBLIC "-Wall" "-Wextra" "-Wpedantic" "-O0" "-g")
target_compile_definitions(${COMPILER_LIB_OUTPUT} PUBLIC LOG_EXIT_FUNC=${LOG_EXIT_FUNC})
target_include_directories(${COMPILER_LIB_OUTPUT} PUBLIC ${MAGIC_ENUM_INCLUDES})
target_include_directories(${COMPILER_LIB_OUTPUT} PUBLIC src)

add_executable(${COMPILER_OUTPUT} src/main.cpp)
target_link_libraries(${COMPILER_OUTPUT} ${COMPILER_LIB_OUTPUT})

# set(FLEX_OUTPUT_NAME flex_output)
# set(FLEX_INPUT input.flex)
# set(FLEX_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/flex_input.yy.cpp)
# execute_process (
# 	COMMAND bash -c "flex -o ${FLEX_SOURCES} ${FLEX_INPUT}"
#     RESULT_VARIABLE flex_ret
# 	ERROR_VARIABLE flex_err
# )
# if(flex_ret EQUAL "1")
# 	# TODO: add error
# endif()
# add_executable(${FLEX_OUTPUT_NAME} ${FLEX_SOURCES})
# target_include_directories(${FLEX_OUTPUT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/submodules/magic_enum/include")
# target_include_directories(${FLEX_OUTPUT_NAME} PRIVATE src)

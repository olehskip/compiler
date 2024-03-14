project(Std ASM_NASM)

set(SOURCE_FILES std.asm)
set_source_files_properties(${SOURCE_FILES} PROPERTIES
    LANGUAGE ASM_NASM 
    COMPILE_FLAGS "-f elf64 -g"
)
add_library(std OBJECT ${SOURCE_FILES})

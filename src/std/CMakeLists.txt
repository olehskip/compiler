project(Std ASM_NASM)

set(SOURCE_FILES std.asm)  # etc for all source files
set_source_files_properties(${SOURCE_FILES} PROPERTIES
    LANGUAGE ASM_NASM 
    COMPILE_FLAGS "-f elf64"
)
add_library(std STATIC ${SOURCE_FILES})

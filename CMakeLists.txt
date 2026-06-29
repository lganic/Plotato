cmake_minimum_required(VERSION 3.16)
project(plotato LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

find_program(OBJCOPY_EXECUTABLE objcopy REQUIRED)

set(ICON_PNG "${CMAKE_CURRENT_SOURCE_DIR}/static/Icon-small.png")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(ICON_OBJECT "${GENERATED_DIR}/icon_small_png.o")

add_custom_command(
    OUTPUT "${ICON_OBJECT}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
    COMMAND ${OBJCOPY_EXECUTABLE}
        --input binary
        --output elf64-x86-64
        --binary-architecture i386:x86-64
        Icon-small.png
        "${ICON_OBJECT}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/static"
    DEPENDS "${ICON_PNG}"
    COMMENT "Embedding Icon-small.png"
)

set_source_files_properties("${ICON_OBJECT}"
    PROPERTIES
        GENERATED TRUE
        EXTERNAL_OBJECT TRUE
        LANGUAGE C
)

add_library(plotato
    src/Graph.cpp
    src/items/LinePlot.cpp
    src/util/GraphRenderer.cpp
    "${ICON_OBJECT}"
)

target_include_directories(plotato
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GTK3_INCLUDE_DIRS}
)

target_link_libraries(plotato
    PUBLIC
        ${GTK3_LIBRARIES}
)

target_compile_options(plotato
    PUBLIC
        ${GTK3_CFLAGS_OTHER}
)
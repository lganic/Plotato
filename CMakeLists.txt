cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJ_VERSION "0.0.0")  # version fallback, in case git tag check fails.

find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" describe --tags --match "v[0-9]*" --abbrev=0
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )

  if(TAG)
    string(REGEX REPLACE "^v" "" PROJ_VERSION "${TAG}")
  endif()
endif()

project(plotato VERSION ${PROJ_VERSION} LANGUAGES CXX)
message(STATUS "Project version: ${PROJECT_VERSION}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

find_program(OBJCOPY_EXECUTABLE objcopy REQUIRED)

set(ICON_PNG "${CMAKE_CURRENT_SOURCE_DIR}/static/Icon-small.png")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(ICON_OBJECT "${GENERATED_DIR}/icon_small_png.o")

# Generate a header containing the version string
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/generated/version.hpp"
  @ONLY
)

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
        ${CMAKE_CURRENT_BINARY_DIR}/generated
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
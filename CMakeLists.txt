cmake_minimum_required(VERSION 3.16)
project(plotato LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_library(plotato
    src/Graph.cpp
    src/items/LinePlot.cpp
    src/util/GraphRenderer.cpp
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
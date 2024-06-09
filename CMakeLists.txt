cmake_minimum_required(VERSION 3.28)

project(IconConverter)

list(
    APPEND
    CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/libs/cmake-modules/"
    )

include(common)
include(nuget)
include(packages/wil)

add_subdirectory(previous/gdiplus)
add_subdirectory(previous/wic)

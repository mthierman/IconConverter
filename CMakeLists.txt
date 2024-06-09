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

include(release_info)

add_executable(${PROJECT_NAME})

target_sources(
    ${PROJECT_NAME}
    PRIVATE "src/IconConverter.cxx"
            "data/IconConverter.manifest"
    PRIVATE FILE_SET
            HEADERS
            FILES
            "src/IconConverter.hxx"
    )

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE common::features
            common::definitions
            common::flags
            wil::wil
    )

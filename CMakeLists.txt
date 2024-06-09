cmake_minimum_required(VERSION 3.28)

project(IconConverter VERSION 0.0.1)

include(libs/cmake-modules/common.cmake)

add_executable(IconConverter)

target_sources(
    IconConverter
    PRIVATE "src/IconConverter.cxx"
            "${PROJECT_SOURCE_DIR}/data/IconConverter.manifest"
    PRIVATE FILE_SET
            HEADERS
            FILES
            "src/IconConverter.hxx"
    )

target_link_libraries(
    IconConverter
    PRIVATE common::features
            common::definitions
            common::flags
    )

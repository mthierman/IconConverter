cmake_minimum_required(VERSION 3.28)

project(IconConverter VERSION 0.0.1)

list(
    APPEND
    CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/libs/cmake-modules/"
    )

include(common)
include(release_info)
include(nuget)
include(packages/wil)

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

cmake_minimum_required(VERSION 3.28)

project(IconConverter VERSION 0.0.1)

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

file(
    COPY_FILE
    "data/main.rc"
    "${CMAKE_BINARY_DIR}/main.rc"
    )

target_sources(
    ${PROJECT_NAME}
    PRIVATE "src/main.cxx"
            "src/helpers.cxx"
            # "data/main.rc"
            "data/main.manifest"
    )

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE common::features
            common::definitions
            common::flags
            wil::wil
    )

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE APP_NAME="${PROJECT_NAME}"
            APP_VERSION="${PROJECT_VERSION}"
    )

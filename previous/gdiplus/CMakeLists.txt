project(IconConverter_GDIPLUS VERSION 0.0.1)

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
    PRIVATE "src/main.cxx"
            "data/IconConverter.manifest"
    PRIVATE FILE_SET
            HEADERS
            FILES
            "src/helpers.hxx"
    )

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE common::features
            common::flags
            gdiplus.lib
    )

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE GDIPVER=0x0110
            APP_NAME="${PROJECT_NAME}"
            APP_VERSION="${PROJECT_VERSION}"
    )

cmake_minimum_required(VERSION 3.26)
set(APP_NAME "IconConverter")
set(APP_VERSION "0.0.1")
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${APP_NAME}")

include(FetchContent)

project(
    ${APP_NAME}
    VERSION ${APP_VERSION}
    LANGUAGES C CXX ASM
)

add_library(common INTERFACE)

target_compile_definitions(
    common
    INTERFACE
    APP_NAME="${APP_NAME}"
    APP_VERSION="${APP_VERSION}"
)

target_link_libraries(
    common
    INTERFACE
    gdiplus.lib
)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(
        common
        INTERFACE
        /await:strict
        /bigobj
        /diagnostics:caret
        /EHsc
        /permissive-
        /utf-8
        /W4
        /WX
        /Zc:__cplusplus
    )

    target_link_options(
        common
        INTERFACE
        /WX
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(
        common
        INTERFACE
        -fcoroutines
    )
endif()

file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cxx")
file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}/data/app.manifest" MANIFEST_FILE)

add_executable(
    ${APP_NAME}
    ${SOURCES}
    ${MANIFEST_FILE}
)

set_property(
    TARGET ${APP_NAME}
    APPEND PROPERTY LINK_LIBRARIES common
)

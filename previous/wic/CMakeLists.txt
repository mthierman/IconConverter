project(IconConverter_WIC VERSION 0.0.1)

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

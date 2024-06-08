add_executable(IconConverter)

target_sources(
    IconConverter
    PRIVATE "IconConverter/IconConverter.cxx"
            "${PROJECT_SOURCE_DIR}/data/glow.manifest"
    PRIVATE FILE_SET
            HEADERS
            FILES
            "IconConverter/IconConverter.hxx"
)

target_link_libraries(
    IconConverter
    PRIVATE glow::glow
            glow::flags
)

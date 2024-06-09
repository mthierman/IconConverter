cmake_minimum_required(VERSION 3.28)

project(IconConverter)

# include(libs/cmake-modules/common.cmake) include(libs/cmake-modules/release_info.cmake)
# include(libs/cmake-modules/nuget.cmake) include(libs/cmake-modules/packages/wil.cmake)

# add_executable(${PROJECT_NAME})

# target_sources( ${PROJECT_NAME} PRIVATE "src/IconConverter.cxx"
# "${PROJECT_SOURCE_DIR}/data/IconConverter.manifest" PRIVATE FILE_SET HEADERS FILES
# "src/IconConverter.hxx" )

# target_link_libraries( ${PROJECT_NAME} PRIVATE common::features common::definitions common::flags
# wil::wil )

# add_subdirectory(previous/gdiplus)
add_subdirectory(previous/wic)

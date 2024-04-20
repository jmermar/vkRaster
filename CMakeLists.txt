cmake_minimum_required(VERSION 3.22)

project(
    VkRaster
    VERSION 1.0
    LANGUAGES CXX
)

add_subdirectory(res)


if (WIN32)
    ADD_DEFINITIONS(-DRESPATH=\"../../../res/\")
else()
    ADD_DEFINITIONS(-DRESPATH=\"../res/\")
endif(WIN32)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(vendor EXCLUDE_FROM_ALL)
add_subdirectory(src)
cmake_minimum_required(VERSION 3.25)

project(
    VkRays
    VERSION 1.0
    LANGUAGES CXX
)

add_subdirectory(res)

ADD_DEFINITIONS(-DRESPATH=\"../../../res/\")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(vendor EXCLUDE_FROM_ALL)
add_subdirectory(src)
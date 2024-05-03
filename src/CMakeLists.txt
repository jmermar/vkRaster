# Add source to this project's executable.

file(GLOB_RECURSE SRC
     "*.cpp"
)

add_executable (${PROJECT_NAME} ${SRC} 
    "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui/imgui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui/imgui_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui/backends/imgui_impl_sdl3.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui/backends/imgui_impl_vulkan.cpp"
)

add_dependencies(${PROJECT_NAME} Shaders)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

target_compile_options(${PROJECT_NAME} PRIVATE "-Wall" "-Werror")


target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui")


find_package(Vulkan REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    ${Vulkan_LIBRARIES}
    SDL3::SDL3
    vk-bootstrap::vk-bootstrap
    vma
)
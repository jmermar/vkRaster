# Add source to this project's executable.
add_executable (${PROJECT_NAME} "main.cpp"  "Program.cpp" "System.cpp" "rendering/Renderer.cpp" "rendering/vk/vkSync.cpp"  "rendering/vk/vkCommand.cpp" "rendering/vk/vkImage.cpp" "rendering/vk/vkInit.cpp" "rendering/vk/vkDescriptors.cpp" "rendering/vk/vkPipelines.cpp" "rendering/FrameData.cpp" "rendering/Swapchain.cpp" "rendering/vk/vma.cpp" "rendering/vk/vkDeletion.cpp" "rendering/vk/vkBuffer.cpp" "rendering/CommandsSubmitter.cpp" "rendering/MeshHandler.cpp"  "rendering/renderers/UnlitRenderer.cpp" "rendering/BindlessDescriptor.cpp" "stb_image.h" "stb_image_write.hpp" "tinygltf.h")

add_dependencies(${PROJECT_NAME} Shaders)

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../public/")


target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/vma/include")

find_package(Vulkan REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    ${Vulkan_LIBRARIES}
    SDL2::SDL2
    vk-bootstrap::vk-bootstrap
)
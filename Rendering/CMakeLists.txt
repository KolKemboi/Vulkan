cmake_minimum_required(VERSION 3.10)
project(VulkanTriangle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find dependencies
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIRS})

# -----------------------------
# 1. Automatic shader compilation
# -----------------------------
set(SHADER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shader)
set(VERT_SHADER ${SHADER_SRC_DIR}/vertex.vert)
set(FRAG_SHADER ${SHADER_SRC_DIR}/fragment.frag)

set(VERT_SPV ${SHADER_SRC_DIR}/vert.spv)
set(FRAG_SPV ${SHADER_SRC_DIR}/frag.spv)

# Vertex shader
add_custom_command(
    OUTPUT ${VERT_SPV}
    COMMAND glslangValidator -V ${VERT_SHADER} -o ${VERT_SPV}
    DEPENDS ${VERT_SHADER}
    COMMENT "Compiling vertex shader to SPIR-V"
)

# Fragment shader
add_custom_command(
    OUTPUT ${FRAG_SPV}
    COMMAND glslangValidator -V ${FRAG_SHADER} -o ${FRAG_SPV}
    DEPENDS ${FRAG_SHADER}
    COMMENT "Compiling fragment shader to SPIR-V"
)

# Create a custom target that builds shaders
add_custom_target(shaders ALL
    DEPENDS ${VERT_SPV} ${FRAG_SPV}
)

# -----------------------------
# 2. Build the executable
# -----------------------------
add_executable(VulkanTriangle src/main.cpp)

# Ensure the executable depends on shaders
add_dependencies(VulkanTriangle shaders)

# Link libraries
target_link_libraries(VulkanTriangle glfw Vulkan::Vulkan)

# -----------------------------
# 3. Optional: Copy shaders to build folder
# -----------------------------
foreach(SPIRV ${VERT_SPV} ${FRAG_SPV})
    add_custom_command(
        TARGET VulkanTriangle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SPIRV}
            $<TARGET_FILE_DIR:VulkanTriangle>
    )
endforeach()

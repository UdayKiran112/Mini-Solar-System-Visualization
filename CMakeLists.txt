cmake_minimum_required(VERSION 3.14)
project(MiniSolarSystem)

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()
if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)

# GLAD
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# GLFW
add_subdirectory(external/glfw)

# OpenGL
find_package(OpenGL REQUIRED)

# Sources
file(GLOB SOURCES "src/*.cpp")

add_executable(solar_system ${SOURCES})

# Include paths
target_include_directories(solar_system PRIVATE
    include
    external/glad/include
    external/glm
)

# Link libraries
target_link_libraries(solar_system
    glad
    glfw
    OpenGL::GL
)

# Copy textures and shaders
add_custom_command(TARGET solar_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:solar_system>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:solar_system>/shaders
)

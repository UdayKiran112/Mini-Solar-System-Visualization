cmake_minimum_required(VERSION 3.14)
project(MiniSolarSystem)

# Enforce modern CMake policies
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)  # GLVND for OpenGL
endif()
if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)  # target_sources inheritance
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------
# GLAD (static library)
# ------------------------
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# ------------------------
# GLFW
# ------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# ------------------------
# GLM (header-only)
# ------------------------
include_directories(external/glm)

# ------------------------
# Find OpenGL
# ------------------------
find_package(OpenGL REQUIRED)

# ------------------------
# Collect source files
# ------------------------
file(GLOB_RECURSE SOURCES src/*.cpp)

# ------------------------
# Build executable
# ------------------------
add_executable(solar_system ${SOURCES})

# ------------------------
# Include directories
# ------------------------
target_include_directories(solar_system PRIVATE
    include
    external/glad/include
    external/glm
    external/glfw/include
)

# ------------------------
# Link libraries
# ------------------------
target_link_libraries(solar_system
    glad
    glfw
    OpenGL::GL
)

# ------------------------
# Windows: Add system libraries if using MinGW
# ------------------------
if(WIN32 AND MINGW)
    target_link_libraries(solar_system opengl32)
endif()

# ------------------------
# Copy resources and shaders after build
# ------------------------
add_custom_command(TARGET solar_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:solar_system>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:solar_system>/shaders
)

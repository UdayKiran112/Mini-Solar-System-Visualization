cmake_minimum_required(VERSION 3.10)
project(MiniSolarSystem)

set(CMAKE_CXX_STANDARD 17)

# Include GLAD manually
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)
include_directories(include)
file(GLOB SOURCES "src/*.cpp")

# GLFW via subdirectory
add_subdirectory(external/glfw)

include_directories(
    include
    external/glad/include
    external/glfw/include
)

add_executable(solar_system ${SOURCES})

target_link_libraries(solar_system glad glfw GL)
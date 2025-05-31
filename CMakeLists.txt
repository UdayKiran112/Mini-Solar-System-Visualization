cmake_minimum_required(VERSION 3.10)
project(MiniSolarSystem)

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

add_subdirectory(external/glfw)

file(GLOB SOURCES "src/*.cpp" "src/stb_image.cpp")

add_executable(solar_system ${SOURCES})

target_include_directories(solar_system PRIVATE include external/glad/include external/glfw/include)

find_package(OpenGL REQUIRED)

target_link_libraries(solar_system PRIVATE glad glfw OpenGL::GL)

add_custom_command(TARGET solar_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:solar_system>/textures
)

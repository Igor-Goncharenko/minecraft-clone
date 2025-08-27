cmake_minimum_required(VERSION 3.10)
project(minecraft-clone)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if (NOT EXISTS ${CMAKE_SOURCE_DIR}/libs/glfw/CMakeLists.txt OR 
    NOT EXISTS ${CMAKE_SOURCE_DIR}/libs/cglm/CMakeLists.txt)
   find_package(Git REQUIRED)
   message(STATUS "Submodules not found, initializing...")
   execute_process(
       COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
       RESULT_VARIABLE GIT_SUBMOD_RESULT
       OUTPUT_QUIET
       ERROR_QUIET
   )
endif()

add_subdirectory(libs/cglm)
set_target_properties(cglm PROPERTIES
    CGLM_STATIC ON
    CGLM_SHARED OFF
)

add_subdirectory(libs/glfw)
set_target_properties(glfw PROPERTIES
    GLFW_BUILD_EXAMPLES OFF
    GLFW_BUILD_TESTS OFF
    GLFW_BUILD_DOCS OFF
)

add_executable(${PROJECT_NAME})

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/libs/glad/src/glad.c"
)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/libs/glad/include"
    "${CMAKE_SOURCE_DIR}/libs/glfw/include"
    "${CMAKE_SOURCE_DIR}/libs/cglm/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    cglm
    glfw
    m
    dl
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-unused-parameter
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    MINECRAFT_DEBUG 
    PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)

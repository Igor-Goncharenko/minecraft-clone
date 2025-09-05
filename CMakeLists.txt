cmake_minimum_required(VERSION 3.10)
project(minecraft-clone)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

message(STATUS "Using VCPKG_ROOT: $ENV{VCPKG_ROOT}")
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg target triplet")
message(STATUS "Using vcpkg triplet: ${VCPKG_TARGET_TRIPLET}")

list(APPEND CMAKE_PREFIX_PATH 
    "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}"
    "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/share"
)

option(BUILD_DEBUG "Build with debug symbols" ON)

add_executable(${PROJECT_NAME})

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/libs/glad/src/glad.c"
)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/libs/glad/include"
)

find_package(glfw3 CONFIG REQUIRED)
find_package(cglm CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(ZLIB REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    cglm::cglm
    SQLite::SQLite3
    ZLIB::ZLIB
)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        #/W4
        #/WX
        #/wd4100
        #/wd4201
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wno-unused-parameter
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        MINECRAFT_DEBUG 
        PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
    )
    target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME "${PROJECT_NAME}-debug"
    )

    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /Od /Zi)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O0 -g)
    endif()
else()
    file(COPY "${CMAKE_SOURCE_DIR}/shaders" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /O2)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    endif()
endif()

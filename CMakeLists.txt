#---------------------------------------------------------------------
# Vulkan test project definition

cmake_minimum_required(VERSION 3.0.0)
project(vulkan_test)

#---------------------------------------------------------------------
# Setup compiler

if(MSVC)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
else(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ggdb -g3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(MSVC)

#---------------------------------------------------------------------
# Set variables

set (CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "" FORCE)

string (TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
if (BUILD_TYPE STREQUAL "debug")
    set (IS_DEBUG "ON")
endif(BUILD_TYPE STREQUAL "debug")

#---------------------------------------------------------------------
# Modules

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)

#---------------------------------------------------------------------
# Search dependency libraries

find_package(vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

include(add_glfw)
include(add_glm)

#---------------------------------------------------------------------
# Application sources

file(GLOB_RECURSE CPP_SOURCES src/*.cpp)

add_executable(${PROJECT_NAME}
    ${CPP_SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME}
    ${Vulkan_LIBRARIES}
    ${GLFW_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}   RUNTIME DESTINATION bin)

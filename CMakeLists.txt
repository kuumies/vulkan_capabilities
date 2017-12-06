#-------------------------------------------------------------------------------
# Vulkan capabilities project definition

cmake_minimum_required(VERSION 3.0.0)
project(vulkan_capabilities)

#---------------------------------------------------------------------
# Externals

set (EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${EXTERNAL_DIR}/assimp
    ${EXTERNAL_DIR}/glm
)

include(add_assimp)
include(add_glm)

#-------------------------------------------------------------------------------
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

#-------------------------------------------------------------------------------
# Search dependency libraries

find_package(openmp REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

find_package(vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

FIND_PACKAGE(Qt5Core REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Core_INCLUDE_DIRS})
ADD_DEFINITIONS(${Qt5Core_DEFINITIONS})
SET(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5Core_LIBRARIES})

FIND_PACKAGE(Qt5Gui REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Gui_INCLUDE_DIRS})
ADD_DEFINITIONS(${Qt5Gui_DEFINITIONS})
SET(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5Gui_LIBRARIES})

FIND_PACKAGE(Qt5Widgets REQUIRED)
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})
ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS})
SET(QT_LIBRARIES ${QT_LIBRARIES} ${Qt5Widgets_LIBRARIES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#-------------------------------------------------------------------------------
# Application sources

file(GLOB_RECURSE CPP_SOURCES src/*.cpp)
file(GLOB_RECURSE UI_SOURCES  src/*.ui)
file(GLOB_RECURSE QRC_SOURCES src/*.qrc)
file(GLOB_RECURSE GLSL_SOURCES src/*.vert src/*.frag)
file(GLOB_RECURSE TEXTURE_SOURCES src/textures/*.png)

add_executable(${PROJECT_NAME}
    ${CPP_SOURCES}
    ${UI_SOURCES}
    ${QRC_SOURCES}
    ${GLSL_SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME}
    ${Vulkan_LIBRARIES}
    ${QT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}   RUNTIME DESTINATION bin)
install(FILES ${TEXTURE_SOURCES}          DESTINATION bin/textures)

#-------------------------------------------------------------------------------
# GLSL to SPIR-V
# based on the code from: https://gist.github.com/vlsh/a0d191701cb48f157b05be7f74d79396

if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else()
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
endif()

foreach(GLSL ${GLSL_SOURCES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
    )

add_dependencies(${PROJECT_NAME} Shaders)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
        )

install(FILES "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders" DESTINATION bin/)


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
# Search dependency libraries

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

#---------------------------------------------------------------------
# Application sources

file(GLOB_RECURSE CPP_SOURCES src/*.cpp)
file(GLOB_RECURSE UI_SOURCES  src/*.ui)
file(GLOB_RECURSE QRC_SOURCES src/*.qrc)

add_executable(${PROJECT_NAME}
    ${CPP_SOURCES}
    ${UI_SOURCES}
    ${QRC_SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME}
    ${Vulkan_LIBRARIES}
    ${QT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}   RUNTIME DESTINATION bin)

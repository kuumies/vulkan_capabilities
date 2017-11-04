#---------------------------------------------------------------------
# Add GLFW

set(GLFW_DIR ${CMAKE_CURRENT_LIST_DIR})
include_directories(${GLFW_DIR}/include)

if (APPLE)
	set(GLFW_DIR_LIBRARY_DIR ${GLFW_DIR}/lib/apple_clang)
    if (IS_DEBUG)
        set(GLFW_LIBRARIES glfw3d)
    else(IS_DEBUG)
        set(GLFW_LIBRARIES glfw3)
    endif(IS_DEBUG)
else()
    set(GLFW_DIR_LIBRARY_DIR ${GLFW_DIR}/lib/msvc14)
    if (IS_DEBUG)
        set(GLFW_LIBRARIES glfw3d.lib)
    else(IS_DEBUG)
        set(GLFW_LIBRARIES glfw3.lib)
    endif(IS_DEBUG)
endif (APPLE)

link_directories(${GLFW_DIR_LIBRARY_DIR})

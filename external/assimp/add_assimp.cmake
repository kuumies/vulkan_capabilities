#---------------------------------------------------------------------
# Add Assimp

set(ASSIMP_DIR ${CMAKE_CURRENT_LIST_DIR})

include_directories(${ASSIMP_DIR}/include)

if (APPLE)
    set(ASSIMP_LIBRARY_DIR ${ASSIMP_DIR}/lib/apple_clang)
    if (IS_DEBUG)
        set(ASSIMP_LIBRARIES libassimpd.4.0.1.dylib)
    else(IS_DEBUG)
        set(ASSIMP_LIBRARIES libassimp.4.0.1.dylib)
    endif(IS_DEBUG)
else()
    set(ASSIMP_LIBRARY_DIR ${ASSIMP_DIR}/lib/msvc14)
    set(ASSIMP_BINARY_DIR ${ASSIMP_DIR}/bin)
    if (IS_DEBUG)
        set(ASSIMP_LIBRARIES assimp-vc140-mtd.lib)
        install(
            FILES
            ${ASSIMP_BINARY_DIR}/assimp-vc140-mtd.dll
            DESTINATION bin
        )
    else(IS_DEBUG)
        set(ASSIMP_LIBRARIES assimp-vc140-mt.lib)
        install(
            FILES
            ${ASSIMP_BINARY_DIR}/assimp-vc140-mt.dll
            DESTINATION bin
        )
    endif(IS_DEBUG)
endif (APPLE)

link_directories(${ASSIMP_LIBRARY_DIR})

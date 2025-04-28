project("bread")

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_FLAGS} /GR- /EHsc-")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -Wall -Wextra"
    )
endif()

# External sources
file(GLOB EXTERNAL_SRC
    # freetype
    freetype/src/base/ftsystem.c
    freetype/src/base/ftinit.c
    freetype/src/base/ftglyph.c
    freetype/src/base/ftstroke.c
    freetype/src/base/ftbitmap.c
    freetype/src/base/ftoutln.c
    freetype/src/base/ftobjs.c
    freetype/src/base/ftutil.c
    freetype/src/base/ftcalc.c
    freetype/src/base/ftstream.c
    freetype/src/base/fttrigon.c
    freetype/src/base/ftlcdfil.c
    freetype/src/base/ftgloadr.c
    freetype/src/base/ftfntfmt.c
    freetype/src/base/ftrfork.c
    freetype/src/base/ftmm.c
    freetype/src/base/ftdebug.c
    freetype/src/gzip/ftgzip.c
    freetype/src/truetype/truetype.c
    freetype/src/smooth/smooth.c
    freetype/src/sfnt/sfnt.c
    
    # stb
    external/stb_image.cpp
)

file(GLOB ENGINE_SRC
    core/string.cpp
    
    engine/engine.cpp
    
    graphics/egl/egl.cpp

    graphics/graphics.cpp
    graphics/render_target.cpp
    
    io/image.cpp
    io/input.cpp
    io/texture.cpp
    io/resource.cpp
    io/resource_manager.cpp
    
    mem/utils.cpp
    mem/generic_allocator.cpp
    
    2d/camera_2d.cpp
    2d/object_2d.cpp
    2d/sprite_2d.cpp

    gui/button.cpp
    gui/control.cpp
    
    physics/body_2d.cpp
    physics/physics_2d.cpp

    # Default 2d physics driver
    physics/p2d/p2d_driver.cpp

    objects/canvas_object.cpp
    objects/object.cpp
    objects/object_allocator.cpp
    objects/scene_manager.cpp
)

file(GLOB GLES_RENDERER_SRC
    graphics/gles/gles_cmd_proc.cpp
    graphics/gles/gles_driver.cpp
    graphics/gles/gles_utility.cpp
)

set(PLATFORM_LIBRARIES "")
if(ANDROID)
    file(GLOB PLATFORM_SRC
        platform/android/android_egl.cpp
        platform/android/android_file.cpp
        platform/android/android_log.cpp
        platform/android/android_main.cpp
        platform/android/android_mem_utils.cpp
        platform/android/android_page_allocator.cpp
        platform/android/android_time.cpp

        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    add_definitions(-DENGINE_ANDROID)
    set(PLATFORM_LIBRARIES GLESv3 EGL android log)
elseif(WIN32)
    file(GLOB PLATFORM_SRC
        platform/win32/win32_egl.cpp
        platform/win32/win32_file.cpp
        platform/win32/win32_input.cpp
        platform/win32/win32_log.cpp
        platform/win32/win32_main.cpp
        platform/win32/win32_mem_utils.cpp
        platform/win32/win32_page_allocator.cpp
        platform/win32/win32_thread.cpp
        platform/win32/win32_time.cpp
    )
    add_definitions(-DENGINE_WIN32)
    set(PLATFORM_LIBRARIES opengl32)
endif()

set(SOURCE_FILES
    ${EXTERNAL_SRC}
    ${ENGINE_SRC}
    ${PLATFORM_SRC}
    ${GLES_RENDERER_SRC}
)

add_library(
    "bread"
    STATIC
    ${SOURCE_FILES}
)

target_include_directories(
    "bread"
    PRIVATE
    "."
    "freetype/include"
    "external/box2d/include"
    ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(
    "bread"
    ${PLATFORM_LIBRARIES}
)

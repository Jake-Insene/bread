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
    external/freetype/src/base/ftsystem.c
    external/freetype/src/base/ftinit.c
    external/freetype/src/base/ftglyph.c
    external/freetype/src/base/ftstroke.c
    external/freetype/src/base/ftbitmap.c
    external/freetype/src/base/ftoutln.c
    external/freetype/src/base/ftobjs.c
    external/freetype/src/base/ftutil.c
    external/freetype/src/base/ftcalc.c
    external/freetype/src/base/ftstream.c
    external/freetype/src/base/fttrigon.c
    external/freetype/src/base/ftlcdfil.c
    external/freetype/src/base/ftgloadr.c
    external/freetype/src/base/ftfntfmt.c
    external/freetype/src/base/ftrfork.c
    external/freetype/src/base/ftmm.c
    external/freetype/src/base/ftdebug.c
    external/freetype/src/gzip/ftgzip.c
    external/freetype/src/truetype/truetype.c
    external/freetype/src/smooth/smooth.c
    external/freetype/src/sfnt/sfnt.c
    
    # stb
    external/stb_image.cpp
)

file(GLOB ENGINE_SRC
    core/string.cpp
    
    display/display.cpp
    display/window.cpp
    
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

    os/mutex.cpp
    os/os.cpp
    os/semaphore.cpp
    os/thread.cpp

    objects/canvas_object.cpp
    objects/object.cpp
    objects/object_allocator.cpp
    objects/scene_manager.cpp

    gui/button.cpp
    gui/control.cpp

    2d/camera_2d.cpp
    2d/object_2d.cpp
    2d/sprite_2d.cpp

    physics/body_2d.cpp
    physics/physics_2d.cpp

    # Default 2d physics driver
    physics/p2d/p2d_driver.cpp
)

file(GLOB GLES_RENDERER_SRC
    graphics/gles/gles_cmd_proc.cpp
    graphics/gles/gles_driver.cpp
    graphics/gles/gles_memory_allocator.cpp
    graphics/gles/gles_utility.cpp
)

set(PLATFORM_LIBRARIES "")
if(ANDROID)
    file(GLOB PLATFORM_SRC
        platform/android/android_display.cpp
        platform/android/android_egl.cpp
        platform/android/android_engine.cpp
        platform/android/android_file.cpp
        platform/android/android_log.cpp
        platform/android/android_main.cpp
        platform/android/android_os.cpp
        platform/android/android_page_allocator.cpp
        platform/android/android_time.cpp

        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    add_definitions(-DENGINE_ANDROID)
    set(PLATFORM_LIBRARIES GLESv3 EGL android log)
elseif(WIN32)
    file(GLOB PLATFORM_SRC
        platform/win32/win32_display.cpp
        platform/win32/win32_egl.cpp
        platform/win32/win32_engine.cpp
        platform/win32/win32_file.cpp
        platform/win32/win32_input.cpp
        platform/win32/win32_log.cpp
        platform/win32/win32_main.cpp
        platform/win32/win32_os.cpp
        platform/win32/win32_page_allocator.cpp
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
    "external/freetype/include"
    "external/box2d/include"
    ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(
    "bread"
    ${PLATFORM_LIBRARIES}
)

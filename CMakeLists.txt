project("bread")

set(BREAD_BUILD_DEFINITIONS "")
set(BREAD_DEBUG_DEFINITIONS "-DDEBUG -DSHOW_DEBUG_INFO -DENABLE_DEBUG_OPTIONS")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} ${BREAD_DEBUG_DEFINITIONS})
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DRELEASE")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} ${BREAD_DEBUG_DEFINITIONS})
endif()

# Dependencies
add_subdirectory(external/freetype)


# External sources
file(GLOB EXTERNAL_SRC
    # stb
    external/stb_image.cpp
)

file(GLOB ENGINE_SRC
    src/collections/string.cpp
    
    src/debug/debug.cpp
    
    src/display/display.cpp
    src/display/window.cpp
    
    src/engine/engine.cpp

    src/fs/file.cpp
    
    src/graphics/egl/egl.cpp
    src/graphics/graphics.cpp
    src/graphics/render_target.cpp

    src/input/input.cpp

    src/log/log.cpp
    
    src/mem/utils.cpp
    src/mem/generic_allocator.cpp

    src/os/mutex.cpp
    src/os/os.cpp
    src/os/semaphore.cpp
    src/os/thread.cpp

    src/object/object.cpp
    src/object/object_allocator.cpp
    src/object/scene_manager.cpp
    src/object/timer.cpp

    src/canvas/button.cpp
    src/canvas/canvas_object.cpp
    src/canvas/canvas_sprite.cpp
    src/canvas/label.cpp
    src/canvas/progress_bar.cpp

    src/2d/animated_sprite_2d.cpp
    src/2d/camera_2d.cpp
    src/2d/object_2d.cpp
    src/2d/sprite_2d.cpp
    src/2d/tile_map.cpp

    src/physics/body_2d.cpp
    src/physics/physics_2d.cpp

    # Default 2d physics driver
    src/physics/p2d/p2d_driver.cpp

    src/resource/font.cpp
    src/resource/image.cpp
    src/resource/resource.cpp
    src/resource/resource_manager.cpp
    src/resource/sprite_animation.cpp
    src/resource/texture.cpp
    src/resource/tile_set.cpp

    src/scene/scene_manager.cpp
)

file(GLOB GLES_RENDERER_SRC
    src/graphics/gles/gles_cmd_proc.cpp
    src/graphics/gles/gles_driver.cpp
    src/graphics/gles/gles_memory_allocator.cpp
    src/graphics/gles/gles_utility.cpp
)

set(PLATFORM_LIBRARIES "")
if(ANDROID)
    file(GLOB PLATFORM_SRC
        src/platform/android/android_display.cpp
        src/platform/android/android_egl.cpp
        src/platform/android/android_engine.cpp
        src/platform/android/android_file.cpp
        src/platform/android/android_log.cpp
        src/platform/android/android_main.cpp
        src/platform/android/android_math.cpp
        src/platform/android/android_os.cpp
        src/platform/android/android_page_allocator.cpp
        src/platform/android/android_time.cpp

        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DENGINE_ANDROID")
    set(PLATFORM_LIBRARIES GLESv3 EGL android log)
elseif(WIN32)
    file(GLOB PLATFORM_SRC
        src/platform/win32/win32_display.cpp
        src/platform/win32/win32_egl.cpp
        src/platform/win32/win32_engine.cpp
        src/platform/win32/win32_file.cpp
        src/platform/win32/win32_main.cpp
        src/platform/win32/win32_math.cpp
        src/platform/win32/win32_os.cpp
        src/platform/win32/win32_page_allocator.cpp
        src/platform/win32/win32_time.cpp
    )
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DENGINE_WIN32")
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

target_compile_definitions("bread" PUBLIC ${BREAD_BUILD_DEFINITIONS})

target_include_directories(
    "bread"
    PRIVATE
    "src"
    "."
    "external/freetype/include"
    ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(
    "bread"
    ${PLATFORM_LIBRARIES}
    freetype
)

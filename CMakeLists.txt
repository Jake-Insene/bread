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
    collections/string.cpp
    
    debug/debug.cpp
    
    display/display.cpp
    display/window.cpp
    
    engine/engine.cpp

    fs/file.cpp
    
    graphics/egl/egl.cpp
    graphics/graphics.cpp
    graphics/render_target.cpp

    input/input.cpp

    log/log.cpp
    
    mem/utils.cpp
    mem/generic_allocator.cpp

    os/mutex.cpp
    os/os.cpp
    os/semaphore.cpp
    os/thread.cpp

    object/object.cpp
    object/object_allocator.cpp
    object/scene_manager.cpp
    object/timer.cpp

    canvas/button.cpp
    canvas/canvas_object.cpp
    canvas/canvas_sprite.cpp
    canvas/label.cpp

    2d/animated_sprite_2d.cpp
    2d/camera_2d.cpp
    2d/object_2d.cpp
    2d/sprite_2d.cpp
    2d/tile_map.cpp

    physics/body_2d.cpp
    physics/physics_2d.cpp

    # Default 2d physics driver
    physics/p2d/p2d_driver.cpp

    resource/font.cpp
    resource/image.cpp
    resource/resource.cpp
    resource/resource_manager.cpp
    resource/sprite_animation.cpp
    resource/texture.cpp
    resource/tile_set.cpp

    scene/scene_manager.cpp
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
        platform/android/android_math.cpp
        platform/android/android_os.cpp
        platform/android/android_page_allocator.cpp
        platform/android/android_time.cpp

        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DENGINE_ANDROID")
    set(PLATFORM_LIBRARIES GLESv3 EGL android log)
elseif(WIN32)
    file(GLOB PLATFORM_SRC
        platform/win32/win32_display.cpp
        platform/win32/win32_egl.cpp
        platform/win32/win32_engine.cpp
        platform/win32/win32_file.cpp
        platform/win32/win32_main.cpp
        platform/win32/win32_math.cpp
        platform/win32/win32_os.cpp
        platform/win32/win32_page_allocator.cpp
        platform/win32/win32_time.cpp
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
    "."
    "external/freetype/include"
    "external/box2d/include"
    ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries(
    "bread"
    ${PLATFORM_LIBRARIES}
    freetype
)

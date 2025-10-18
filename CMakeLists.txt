project("bread")

if(MSVC)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_MINSIZEREL
    )
        string(REPLACE "/RTC1" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/RTCsu" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/EHsc" "" ${flag_var} "${${flag_var}}")
        set(${flag_var} "${${flag_var}}" CACHE STRING "" FORCE)
    endforeach()
endif()

# Dependencies
# External sources
file(GLOB EXTERNAL_SRC
    # stb
    external/external.cpp
)

file(GLOB ENGINE_SRC
    src/2d/animated_sprite_2d.cpp
    src/2d/camera_2d.cpp
    src/2d/object_2d.cpp
    src/2d/sprite_2d.cpp
    src/2d/text_2d.cpp
    src/2d/tile_map.cpp

    src/audio/audio.cpp

    src/canvas/button.cpp
    src/canvas/canvas_object.cpp
    src/canvas/canvas_sprite.cpp
    src/canvas/label.cpp
    src/canvas/progress_bar.cpp

    src/collections/error.cpp
    src/collections/job_queue.cpp
    src/collections/string.cpp
    
    src/debug/debug.cpp
    
    src/display/display.cpp
    src/display/window.cpp
    
    src/engine/engine.cpp

    src/graphics/egl/egl.cpp
    src/graphics/graphics.cpp
    src/graphics/render_target.cpp
    src/graphics/viewport.cpp

    src/input/input.cpp
    
    src/io/file.cpp
    src/io/writer.cpp

    src/log/log.cpp

    src/math/transform_2d.cpp
    src/math/vec2.cpp
    
    src/mem/utils.cpp
    src/mem/generic_allocator.cpp
    src/mem/page_allocator.cpp

    src/os/mutex.cpp
    src/os/os.cpp
    src/os/semaphore.cpp
    src/os/thread.cpp

    src/object/audio_player.cpp
    src/object/object.cpp
    src/object/object_allocator.cpp
    src/object/scene_manager.cpp
    src/object/timer.cpp

    src/physics/area_2d.cpp
    src/physics/body_2d.cpp
    src/physics/physics_2d.cpp
    src/physics/shape_2d.cpp

    # Default 2d physics driver
    src/physics/p2d/p2d_area.cpp
    src/physics/p2d/p2d_body.cpp
    src/physics/p2d/p2d_collision.cpp
    src/physics/p2d/p2d_driver.cpp
    src/physics/p2d/p2d_shape.cpp
    
    src/platform/intrinsics.cpp

    src/resource/font.cpp
    src/resource/image.cpp
    src/resource/resource.cpp
    src/resource/resource_manager.cpp
    src/resource/sound.cpp
    src/resource/sprite_animation.cpp
    src/resource/texture.cpp
    src/resource/tile_set.cpp

    src/scene/scene_manager.cpp
)

file(GLOB GLES_RENDERER_SRC
    src/graphics/gles/gles_driver.cpp
    src/graphics/gles/gles_memory_allocator.cpp
    src/graphics/gles/gles_utility.cpp
    src/graphics/gles/gles_renderer.cpp
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
        src/platform/android/android_time.cpp

        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )
    set(PLATFORM_LIBRARIES GLESv3 EGL android log)
elseif(WIN32)
    file(GLOB PLATFORM_SRC
        src/audio/xaudio2/xaudio2_driver.cpp

        src/platform/win32/win32_display.cpp
        src/platform/win32/win32_egl.cpp
        src/platform/win32/win32_engine.cpp
        src/platform/win32/win32_file.cpp
        src/platform/win32/win32_main.cpp
        src/platform/win32/win32_math.cpp
        src/platform/win32/win32_os.cpp
        src/platform/win32/win32_time.cpp
    )
    set(PLATFORM_LIBRARIES opengl32 xaudio2)
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
target_compile_options("bread" PUBLIC ${BREAD_COMPILE_OPTIONS})
target_link_options("bread" PUBLIC ${BREAD_LINK_OPTIONS})

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
)


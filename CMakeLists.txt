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
    src/audio/audio.cpp

    src/collections/error.cpp
    src/collections/string.cpp
    
    src/concurrency/job_queue.cpp

    src/debug/debug.cpp
    
    src/display/display.cpp
    src/display/window.cpp
    
    src/engine/engine_runtime.cpp

    src/gpu/gpu.cpp

    src/input/input.cpp
    
    src/io/file.cpp
    src/io/writer.cpp

    src/log/log.cpp

    src/math/transform_2d.cpp
    src/math/vec2.cpp
    
    src/mem/utils.cpp
    src/mem/generic_allocator.cpp
    src/mem/page_allocator.cpp
    src/mem/stack_allocator.cpp

    src/os/mutex.cpp
    src/os/os.cpp
    src/os/semaphore.cpp
    src/os/thread.cpp

    src/object/object.cpp

    src/modifier/2d/world_transform.cpp

    src/object/object_pool.cpp

    src/object/2d/camera.cpp

    src/object/core/audio_player.cpp
    src/object/core/timer.cpp

    src/physics/physics_2d.cpp
    src/physics/shape_2d.cpp

    # Default 2d physics driver
    src/physics/p2d/p2d_area.cpp
    src/physics/p2d/p2d_body.cpp
    src/physics/p2d/p2d_collision.cpp
    src/physics/p2d/p2d_driver.cpp
    src/physics/p2d/p2d_shape.cpp
    
    src/platform/intrinsics.cpp

    src/render/buffer.cpp
    src/render/command_queue.cpp
    src/render/memory_heap.cpp
    src/render/render_device.cpp
    
    src/render/core/gpu_material_manager.cpp
    src/render/core/gpu_memory_allocator.cpp
    src/render/core/gpu_resource_manager.cpp
    
    src/render/renderer/render_material_manager.cpp
    src/render/renderer/renderer.cpp

    src/render/pipeline/pipeline.cpp
    src/render/pipeline/pipeline_2d.cpp

    src/resource/font.cpp
    src/resource/image.cpp
    src/resource/material.cpp
    src/resource/resource.cpp
    src/resource/resource_manager.cpp
    src/resource/sound.cpp
    src/resource/sprite_animation.cpp
    src/resource/texture.cpp
    src/resource/tile_set.cpp

    src/scene/scene_manager.cpp
    src/scene/scene.cpp

    src/systems/system_manager.cpp
)

file(GLOB VULKAN_ADAPTER_SRC
    src/gpu/vk/vk_driver.cpp
    src/gpu/vk/vk_header.cpp
    src/gpu/vk/vk_utils.cpp
)

set(PLATFORM_LIBRARIES "")
if(ANDROID)
    file(GLOB PLATFORM_SRC
        src/platform/android/android_display.cpp
        src/platform/android/android_engine.cpp
        src/platform/android/android_file.cpp
        src/platform/android/android_log.cpp
        src/platform/android/android_main.cpp
        src/platform/android/android_math.cpp
        src/platform/android/android_os.cpp

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
    )
    set(PLATFORM_LIBRARIES opengl32 xaudio2 dbghelp)
endif()

set(SOURCE_FILES
    ${EXTERNAL_SRC}
    ${ENGINE_SRC}
    ${PLATFORM_SRC}
    ${VULKAN_ADAPTER_SRC}
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
    $ENV{VULKAN_SDK}/Include
)

target_link_libraries(
    "bread"
    ${PLATFORM_LIBRARIES}
)


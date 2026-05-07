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
    src/audio/audio_service.cpp
    src/audio/audio.cpp

    src/collections/error.cpp
    src/collections/string.cpp
    
    src/concurrency/job_queue.cpp

    src/debug/debug.cpp
    
    src/display/display.cpp
    src/display/window.cpp
    
    src/engine/engine_runtime.cpp

    src/gpu/gpu.cpp

    src/graphics/buffer.cpp
    src/graphics/command_encoder.cpp
    src/graphics/command_queue.cpp
    src/graphics/descriptor_pool.cpp
    src/graphics/descriptor_set.cpp
    src/graphics/device_object.cpp
    src/graphics/device.cpp
    src/graphics/fence.cpp
    src/graphics/memory_heap.cpp
    src/graphics/pipeline_2d.cpp
    src/graphics/pipeline_layout.cpp
    src/graphics/pipeline.cpp
    src/graphics/queue.cpp
    src/graphics/sampler.cpp
    src/graphics/semaphore.cpp
    src/graphics/shader.cpp
    src/graphics/swap_chain.cpp
    src/graphics/texture.cpp

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

    src/render_device/render_device.cpp
    
    src/render_device/core/gpu_material_manager.cpp
    src/render_device/core/gpu_memory_allocator.cpp
    src/render_device/core/gpu_resource_manager.cpp

    src/renderer/framed_buffer.cpp
    src/renderer/framed_pool.cpp
    src/renderer/renderer_2d.cpp
    src/renderer/renderer_batch_2d.cpp
    src/renderer/renderer.cpp
    
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
        # Audio drivers
        src/audio/aaudio/aaudio_driver.cpp

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
        # Audio drivers
        src/audio/wasapi/wasapi_driver.cpp

        src/platform/win32/win32_atomic.cpp
        src/platform/win32/win32_display.cpp
        src/platform/win32/win32_engine.cpp
        src/platform/win32/win32_file.cpp
        src/platform/win32/win32_main.cpp
        src/platform/win32/win32_math.cpp
        src/platform/win32/win32_mutex.cpp
        src/platform/win32/win32_os.cpp
        src/platform/win32/win32_semaphore.cpp
        src/platform/win32/win32_thread.cpp
    )
    set(PLATFORM_LIBRARIES opengl32 xaudio2 dbghelp ole32.lib)
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


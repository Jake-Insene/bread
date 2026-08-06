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
    src/log/log.cpp
    
    src/display/display.cpp
    src/display/window.cpp
    
    src/engine/application.cpp
    src/engine/engine_runtime.cpp

    src/gpu/gpu.cpp

    src/graphics/command_pool.cpp
    src/graphics/gpu_memory_allocator.cpp
    src/graphics/gpu_resource_manager.cpp
    src/graphics/render_device.cpp
    src/graphics/shader.cpp
    src/graphics/swap_chain.cpp

    src/input/input.cpp
    
    src/io/file.cpp
    src/io/writer.cpp

    src/mem/utils.cpp
    src/mem/generic_allocator.cpp
    src/mem/page_allocator.cpp
    src/mem/stack_allocator.cpp

    src/platform/intrinsics.cpp

    src/resource/font.cpp
    src/resource/image.cpp
    src/resource/resource.cpp
    src/resource/resource_manager.cpp
    src/resource/sound.cpp
    src/resource/texture.cpp
)

file(GLOB VULKAN_ADAPTER_SRC
    src/gpu/vk/vk_adapter.cpp
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
    "include"
    "."
    "external/freetype/include"
    ${ANDROID_NDK}/sources/android/native_app_glue
    $ENV{VULKAN_SDK}/Include
)

target_link_libraries(
    "bread"
    ${PLATFORM_LIBRARIES}
)


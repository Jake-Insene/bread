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

file(GLOB ENGINE_SRC
    src/audio/audio_service.cpp
    src/audio/audio.cpp

    src/concurrency/job_queue.cpp

    src/Debug/Assertion.cpp

    src/display/display.cpp
    src/display/window.cpp
    
    src/engine/application.cpp

    src/gpu/gpu.cpp

    src/input/input.cpp
    
    src/IO/File.cpp
    src/IO/Writer.cpp

    src/Mem/Utils.cpp
    src/Mem/GenericAllocator.cpp
    src/Mem/PageAllocator.cpp
    src/Mem/LinearAllocator.cpp

    src/platform/intrinsics.cpp

    src/Main.cpp
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


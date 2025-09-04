project("bread")

# Configuration variables
set(BREAD_BUILD_DEFINITIONS "BREAD" "_HAS_EXCEPTIONS=0")
set(BREAD_COMPILE_OPTIONS "")
set(BREAD_LINK_OPTIONS "")
set(BREAD_EXE_LINK_OPTIONS "")

set(BREAD_DEBUG_DEFINITIONS "-DDEBUG -DSHOW_DEBUG_INFO -DENABLE_DEBUG_OPTIONS")
set(CMAKE_MSVC_RUNTIME_CHECKS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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

    set(
        BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} 
        "/W3"  "/Oi" "/Zl" "/GS-" "/GR-" "/EHs-" "/EHc-" "/Zc:threadSafeInit-"
        # Some warnings needs to be treated as error because we eliminated the compiler runtime,
        # so now is more easy to create errors.
        "/we4701" "/we4700" "/we4101" "/we4703" "/we4189"
    )
    set(BREAD_LINK_OPTIONS ${BREAD_LINK_OPTIONS} "/NODEFAULTLIBS")
    set(BREAD_EXE_LINK_OPTIONS ${BREAD_EXE_LINK_OPTIONS} "/NODEFAULTLIBS" "/ENTRY:WinMain")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -Wall -Wextra"
    )
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} ${BREAD_DEBUG_DEFINITIONS})
    set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} "/MTd")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DRELEASE")
    set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} "/MT")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} ${BREAD_DEBUG_DEFINITIONS})
    set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} "/MT")
endif()

set(BREAD_MSVC_RUNTIME_LIBRARY ${BREAD_MSVC_RUNTIME_LIBRARY})

# Dependencies
# External sources
file(GLOB EXTERNAL_SRC
    # stb
    external/external.cpp
)

file(GLOB ENGINE_SRC
    src/audio/audio.cpp

    src/canvas/button.cpp
    src/canvas/canvas_object.cpp
    src/canvas/canvas_sprite.cpp
    src/canvas/label.cpp
    src/canvas/progress_bar.cpp

    src/2d/animated_sprite_2d.cpp
    src/2d/camera_2d.cpp
    src/2d/object_2d.cpp
    src/2d/sprite_2d.cpp
    src/2d/text_2d.cpp
    src/2d/tile_map.cpp

    src/collections/job_queue.cpp
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

    src/object/audio_player.cpp
    src/object/object.cpp
    src/object/object_allocator.cpp
    src/object/scene_manager.cpp
    src/object/timer.cpp

    src/physics/area_2d.cpp
    src/physics/body_2d.cpp
    src/physics/physics_2d.cpp

    # Default 2d physics driver
    src/physics/p2d/p2d_driver.cpp

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
        src/audio/xaudio2/xaudio2_driver.cpp

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

set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} PARENT_SCOPE)
set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} PARENT_SCOPE)
set(BREAD_LINK_OPTIONS ${BREAD_LINK_OPTIONS} PARENT_SCOPE)
set(BREAD_EXE_LINK_OPTIONS ${BREAD_EXE_LINK_OPTIONS} PARENT_SCOPE)

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Useful for language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_CHECKS OFF)

# Configurating output directories
string(TOLOWER ${BREAD_BUILD_TYPE} BUILD_TYPE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/lib/${BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/lib/${BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/bin/${BUILD_TYPE}")

# Configuration variables
set(BREAD_BUILD_DEFINITIONS "BREAD")
set(BREAD_COMPILE_OPTIONS "")
set(BREAD_LINK_OPTIONS "")
set(BREAD_EXE_LINK_OPTIONS "")
set(BREAD_DEBUG_DEFINITIONS "-DDEBUG -DSHOW_DEBUG_INFO -DENABLE_DEBUG_OPTIONS")

# Detecting target arch
if(DEFINED BREAD_ANDROID)
    string(FIND "${ANDROID_ABI}" "arm64-v8a" IS_ARM)
    if("${IS_ARM}" STREQUAL "-1")
        set(BREAD_TARGET_ARCH ${BREAD_TARGET_ARCH} "X64")
    else()
        set(BREAD_TARGET_ARCH ${BREAD_TARGET_ARCH} "ARM64")
    endif()
endif()

if("${BREAD_TARGET_ARCH}" STREQUAL "X64")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "BREAD_ENABLE_INTRISICS" "BREAD_X64")
elseif("${BREAD_TARGET_ARCH}" STREQUAL "ARM64")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "BREAD_ENABLE_INTRISICS" "BREAD_ARM64")
endif()

# Configuration per compiler
if(MSVC)
    set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} 
        "/W3" "/Oi" "/Zl" "/GS-" "/GR-" "/EHs-" "/EHc-" "/Zc:threadSafeInit-" "/Zc:preprocessor"
        # Some warnings needs to be treated as error because we eliminated the compiler runtime,
        # so now is more easy to create bugs.
        "/we4701" "/we4700" "/we4101" "/we4703" "/we4189"
    )
    set(BREAD_LINK_OPTIONS ${BREAD_LINK_OPTIONS} "/NODEFAULTLIB")
    set(BREAD_EXE_LINK_OPTIONS ${BREAD_EXE_LINK_OPTIONS} "/NODEFAULTLIB" "/ENTRY:WinMain")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DBREAD_MSVC")
else()
    if("${BREAD_TARGET_ARCH}" STREQUAL "X64")
        set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} "-msse4.1")
    endif()
    set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS}
        "-fno-rtti" "-fno-exceptions" "-Wall" "-Wextra" "-ffast-math"
    )
    
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DBREAD_CLANG")
endif()

# Build type
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} ${BREAD_DEBUG_DEFINITIONS})
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    if(NOT MSVC)
        set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} "-O3" "-ffast-math")
    endif()
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DRELEASE")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
    if(NOT MSVC)
        set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} "-O3" "-ffast-math")
    endif()
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} ${BREAD_DEBUG_DEFINITIONS})
endif()

# Configuration per platform
if(ANDROID)
    set(CMAKE_ANDROID_STL_TYPE "c++_static")
    set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
    )
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DBREAD_ANDROID")
elseif(WIN32)
    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DBREAD_WIN32")
endif()

# Utility functions

function(bread_project name)
    if(ANDROID)
        add_library(${name} SHARED ${ARGN})
    elseif(WIN32)
        add_executable(${name} WIN32 ${ARGN})
    endif()
    
    # Copying the bread assets to the game assets folder by default
    add_custom_command(
        TARGET ${name}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/bread/assets ${CMAKE_SOURCE_DIR}/assets
    )

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

    target_compile_definitions(${name} PUBLIC ${BREAD_BUILD_DEFINITIONS})
    target_compile_options(${name} PUBLIC ${BREAD_COMPILE_OPTIONS})
    target_link_options(${name} PUBLIC ${BREAD_EXE_LINK_OPTIONS})
    
    target_include_directories(
        ${name}
        PUBLIC
        "${CMAKE_SOURCE_DIR}/bread/"
        "${CMAKE_SOURCE_DIR}/bread/src"
        "${CMAKE_SOURCE_DIR}"
    )

    target_link_libraries(${name} "bread")

endfunction()
# C++ Standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Useful for language servers
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_CHECKS OFF)
# Expected to be the top directory of the current project
set(CMAKE_PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Configurating output directories
if(DEFINED BREAD_ANDROID)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(BREAD_BUILD_TYPE "debugbuild")
    elseif(${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
        set(BREAD_BUILD_TYPE "releasebuild")
    elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(BREAD_BUILD_TYPE "gamebuild")
    endif()
endif()

string(TOLOWER ${BREAD_BUILD_TYPE} BUILD_TYPE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/lib/${BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/lib/${BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/bin/${BUILD_TYPE}")

# Configuration variables
set(BREAD_BUILD_DEFINITIONS "BREAD")
set(BREAD_COMPILE_OPTIONS "")
set(BREAD_LINK_OPTIONS "")
set(BREAD_EXE_LINK_OPTIONS "")
set(BREAD_EXE_BUILD_OPTIONS "")
set(BREAD_DEBUG_DEFINITIONS "-DDEBUG -DBREAD_SHOW_DEBUG_INFO -DBREAD_ENABLE_DEBUG_OPTIONS")

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
# Some warnings needs to be treated as error because we eliminated the compiler runtime,
# so now is more easy to create bugs.
if(MSVC)
    set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} 
        "/W4" "/Oi" "/Zl" "/GS-" "/GR-" "/EHs-" "/EHc-" "/Zc:threadSafeInit-" "/Zc:preprocessor"
        "/wd4201" "/we4244" "/we4701" "/we4700" "/we4101" "/we4703" "/we4189" "/we4834" "/we4100"
        "/we26493"
    )

    set(BREAD_BUILD_DEFINITIONS ${BREAD_BUILD_DEFINITIONS} "-DBREAD_MSVC")
else()
    if("${BREAD_TARGET_ARCH}" STREQUAL "X64")
        set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS} "-msse4.1")
    endif()
    set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS}
        "-fno-rtti" "-fno-exceptions" "-Wall" "-Wextra" "-ffast-math" "-Werror" "-Wpedantic"
        "-Wno-language-extension-token" "-Wno-gnu-anonymous-struct" "-Wno-error=gnu-anonymous-struct"
        "-Wuninitialized" "-Wconditional-uninitialized" "-Wno-strict-prototypes"
    )
    if(BREAD_BUILD_TYPE STREQUAL "debugbuild")
        set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS}
            "-fsanitize-trap=all"
        )
        set(BREAD_LINK_OPTIONS ${BREAD_COMPILE_OPTIONS}
            "-fsanitize-trap=all"
        )
    endif()
    if(NOT ANDROID)
        set(BREAD_COMPILE_OPTIONS ${BREAD_COMPILE_OPTIONS}
            "-fuse-ld=lld-link" "-flto"
        )
        set(BREAD_LINK_OPTIONS ${BREAD_LINK_OPTIONS} "-flto")
    endif()

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
    if(MSVC)
        set(BREAD_LINK_OPTIONS ${BREAD_LINK_OPTIONS} /NODEFAULTLIB)
        set(BREAD_EXE_LINK_OPTIONS ${BREAD_EXE_LINK_OPTIONS} /NODEFAULTLIB /ENTRY:WinMain)
    else()
        set(BREAD_LINK_OPTIONS ${BREAD_LINK_OPTIONS} -nostdlib)
        set(BREAD_EXE_LINK_OPTIONS ${BREAD_EXE_LINK_OPTIONS} -Wl,/ENTRY:WinMain)
    endif()
endif()

# Utility functions

function(bread_project)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs PACKAGES SOURCES)

    cmake_parse_arguments(PROJECT
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})
    elseif(WIN32)
        add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCES})
    endif()

    # Copying the bread assets to the game assets folder by default
    #add_custom_command(
    #    TARGET ${PROJECT_NAME}
    #    POST_BUILD
    #    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/bread/assets ${CMAKE_SOURCE_DIR}/assets
    #)

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

    target_compile_definitions(${PROJECT_NAME} PUBLIC ${BREAD_BUILD_DEFINITIONS} BREAD_PROJECT_COMPILATION)

    target_compile_options(${PROJECT_NAME} PUBLIC ${BREAD_COMPILE_OPTIONS} ${BREAD_EXE_BUILD_OPTIONS})
    target_link_options(${PROJECT_NAME} PUBLIC ${BREAD_EXE_LINK_OPTIONS})

    target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/bread/include"
    )

    target_link_libraries(${PROJECT_NAME} "bread" ${PROJECT_SUBMODULES} ${PROJECT_PACKAGES})
endfunction()

function(bread_package)
    set(options)
    set(oneValueArgs NAME ASSETS)
    set(multiValueArgs INCLUDES SOURCES)

    cmake_parse_arguments(PACKAGE
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    add_library(${PACKAGE_NAME} STATIC ${PACKAGE_SOURCES})

    if("${PACKAGE_ASSETS}" STREQUAL "")
        set(PACKAGE_ASSETS ${CMAKE_SOURCE_DIR}/assets)
    elseif(NOT IS_ABSOLUTE "${PACKAGE_ASSETS}")
        set(PACKAGE_ASSETS ${CMAKE_SOURCE_DIR}/${PACKAGE_ASSETS})
    endif()
    
    add_custom_command(
        TARGET ${PACKAGE_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${PACKAGE_ASSETS}
    )

    target_compile_definitions(${PACKAGE_NAME} PUBLIC ${BREAD_BUILD_DEFINITIONS} BREAD_MODULE_COMPILATION)
    target_compile_options(${PACKAGE_NAME} PUBLIC ${BREAD_COMPILE_OPTIONS} ${BREAD_EXE_BUILD_OPTIONS})
    target_link_options(${PACKAGE_NAME} PUBLIC ${BREAD_EXE_LINK_OPTIONS})
    target_include_directories(
        ${PACKAGE_NAME}
        PUBLIC
        "${PACKAGE_INCLUDES}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_PROJECT_SOURCE_DIR}/bread/include"
    )
endfunction()


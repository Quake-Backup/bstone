#[[
SDL2W - CMake wrapper for SDL2.

Copyright (c) 2018-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.
SPDX-License-Identifier: MIT


Virtual components:
    - static - uses static version of SDL2.

Required variables:
    - SDL2W_SDL2_DIR - the directory with CMake files or
                       the directory with MSVC / MinGW development build.
                       Leave empty to search automatically.

Targets:
    - SDL2W::SDL2Wmain - SDL2main.
    - SDL2W::SDL2W - SDL2 or SDL2-static.
]]

cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

set(SDL2W_VERSION "1.0.6")
message(STATUS "[SDL2W] Version: ${SDL2W_VERSION}")

set(SDL2W_TMP_TARGET "${CMAKE_FIND_PACKAGE_NAME}::${CMAKE_FIND_PACKAGE_NAME}")
set(SDL2W_TMP_TARGET_MAIN "${SDL2W_TMP_TARGET}main")

set(SDL2W_SDL2_DIR "" CACHE PATH "The directory with CMake files or the directory with MSVC / MinGW development builds. Leave empty to figure out the location of SDL2.")

set(SDL2_DIR "" CACHE PATH "The directory with CMake files for SDL2." FORCE)

find_package(SDL2 QUIET HINTS ${SDL2W_SDL2_DIR})

if (SDL2W_SDL2_DIR)
    message(STATUS "[SDL2W] Custom path: ${SDL2W_SDL2_DIR}")
endif ()


# Parse components.
#
set(SDL2W_TMP_USE_STATIC FALSE)

if (${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
    message(STATUS "[SDL2W] Selected components:")

    foreach (SDL2W_TMP_COMP ${${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS})
        message(STATUS "[SDL2W]     \"${SDL2W_TMP_COMP}\"")

        if (SDL2W_TMP_COMP STREQUAL "static")
            set(SDL2W_TMP_USE_STATIC TRUE)
        endif ()
    endforeach ()
else ()
    message(STATUS "[SDL2W] No components were selected.")
endif ()

set(SDL2W_TMP_SDL2_INC_DIRS "")
set(SDL2W_TMP_SDL2_LINK_LIBS "")
set(SDL2W_TMP_SDL2MAIN_LINK_LIBS "")

# Fix MinGW's WinMain error.
#
if (MINGW)
    list(APPEND SDL2W_TMP_SDL2MAIN_LINK_LIBS "mingw32")
endif ()

#
# Check for Visual C++ / MinGW development libraries.
#
set(SDL2W_TMP_FOUND_DEV_LIBS FALSE)
set(SDL2W_TMP_FOUND_MSVC_LIBS FALSE)
set(SDL2W_TMP_FOUND_MINGW_LIBS FALSE)

if (WIN32 AND SDL2W_SDL2_DIR)
    set(SDL2W_TMP_MSVC_INCLUDE_DIR ${SDL2W_SDL2_DIR}/include)

    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SDL2W_TMP_MSVC_LIBRARIES_DIR ${SDL2W_SDL2_DIR}/lib/x64)

        set(SDL2W_TMP_MINGW_CPU_ARCH x86_64-w64-mingw32)
        set(SDL2W_TMP_MINGW_INCLUDE_DIR ${SDL2W_SDL2_DIR}/${SDL2W_TMP_MINGW_CPU_ARCH}/include/SDL2)
        set(SDL2W_TMP_MINGW_LIBRARIES_DIR ${SDL2W_SDL2_DIR}/${SDL2W_TMP_MINGW_CPU_ARCH}/lib)
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(SDL2W_TMP_MSVC_LIBRARIES_DIR ${SDL2W_SDL2_DIR}/lib/x86)

        set(SDL2W_TMP_MINGW_CPU_ARCH i686-w64-mingw32)
        set(SDL2W_TMP_MINGW_INCLUDE_DIR ${SDL2W_SDL2_DIR}/${SDL2W_TMP_MINGW_CPU_ARCH}/include/SDL2)
        set(SDL2W_TMP_MINGW_LIBRARIES_DIR ${SDL2W_SDL2_DIR}/${SDL2W_TMP_MINGW_CPU_ARCH}/lib)
    else ()
        message(FATAL_ERROR "[SDL2W] Unsupported CPU architecture.")
    endif ()

    set(SDL2W_TMP_MSVC_SDL_H ${SDL2W_TMP_MSVC_INCLUDE_DIR}/SDL.h)
    set(SDL2W_TMP_MSVC_SDL2_LIB ${SDL2W_TMP_MSVC_LIBRARIES_DIR}/SDL2.lib)
    set(SDL2W_TMP_MSVC_SDL2MAIN_LIB ${SDL2W_TMP_MSVC_LIBRARIES_DIR}/SDL2main.lib)

    set(SDL2W_TMP_MINGW_SDL_H ${SDL2W_TMP_MINGW_INCLUDE_DIR}/SDL.h)
    set(SDL2W_TMP_MINGW_LIBSDL2_A ${SDL2W_TMP_MINGW_LIBRARIES_DIR}/libSDL2.a)
    set(SDL2W_TMP_MINGW_LIBSDL2_DLL_A ${SDL2W_TMP_MINGW_LIBRARIES_DIR}/libSDL2.dll.a)
    set(SDL2W_TMP_MINGW_LIBSDL2MAIN_A ${SDL2W_TMP_MINGW_LIBRARIES_DIR}/libSDL2main.a)

    if (EXISTS ${SDL2W_TMP_MSVC_SDL_H} AND
        EXISTS ${SDL2W_TMP_MSVC_SDL2_LIB} AND
        EXISTS ${SDL2W_TMP_MSVC_SDL2MAIN_LIB}
        )
        message(STATUS "[SDL2W] Found Visual C++ development libraries.")

        set(SDL2W_TMP_FOUND_DEV_LIBS TRUE)
        set(SDL2W_TMP_FOUND_MSVC_LIBS TRUE)
    endif ()

    if (EXISTS ${SDL2W_TMP_MINGW_SDL_H} AND
        EXISTS ${SDL2W_TMP_MINGW_LIBSDL2_A} AND
        EXISTS ${SDL2W_TMP_MINGW_LIBSDL2_DLL_A} AND
        EXISTS ${SDL2W_TMP_MINGW_LIBSDL2MAIN_A}
        )
        message(STATUS "[SDL2W] Found MinGW development libraries.")

        set(SDL2W_TMP_FOUND_DEV_LIBS TRUE)
        set(SDL2W_TMP_FOUND_MINGW_LIBS TRUE)
    endif ()
endif ()

set(SDL2W_TMP_FOUND_CONFIG FALSE)
set(SDL2W_TMP_FOUND_TARGETS FALSE)
set(SDL2W_TMP_FOUND_STATIC_TARGET FALSE)

if ((NOT SDL2W_TMP_FOUND_DEV_LIBS) AND SDL2_FOUND)
    if (TARGET SDL2::SDL2main AND (TARGET SDL2::SDL2 OR TARGET SDL2::SDL2-static))
        message(STATUS "[SDL2W] Found CMake targets.")

        set(SDL2W_TMP_FOUND_TARGETS TRUE)

        if (TARGET SDL2::SDL2-static)
            set(SDL2W_TMP_FOUND_STATIC_TARGET TRUE)
        endif ()
    elseif (SDL2_INCLUDE_DIRS AND SDL2_LIBRARIES)
        message(STATUS "[SDL2W] Found CMake config.")

        set(SDL2W_TMP_FOUND_CONFIG TRUE)
    else ()
        message(FATAL_ERROR "[SDL2W] CMake targets / config not found.")
    endif ()
endif ()


set(SDL2W_TMP_VERSION_STRING "")

# Get library locations.
#
message(STATUS "[SDL2W] Libraries:")

if (SDL2W_TMP_FOUND_TARGETS)
    # SDL2
    #
    if (SDL2W_TMP_USE_STATIC AND SDL2W_TMP_FOUND_STATIC_TARGET)
        get_target_property(SDL2W_TMP_LOCATION SDL2::SDL2-static LOCATION)
    else ()
        get_target_property(SDL2W_TMP_LOCATION SDL2::SDL2 LOCATION)
    endif ()

    list(APPEND SDL2W_TMP_SDL2_LINK_LIBS ${SDL2W_TMP_LOCATION})
    message(STATUS "[SDL2W]     \"${SDL2W_TMP_LOCATION}\"")

    # SDL2main
    #
    get_target_property(SDL2W_TMP_LOCATION SDL2::SDL2main LOCATION)
    list(APPEND SDL2W_TMP_SDL2MAIN_LINK_LIBS ${SDL2W_TMP_LOCATION})
    message(STATUS "[SDL2W]     \"${SDL2W_TMP_LOCATION}\"")
elseif (SDL2W_TMP_FOUND_CONFIG)
    set(SDL2W_TMP_SDL2_STATIC_LIB_LOCATION "")
    set(SDL2W_TMP_SDL2MAIN_LIB_LOCATION "")
    set(SDL2W_TMP_SDL2_LIB_LOCATION "")

    foreach (SDL2W_TMP_LOCATION IN LISTS SDL2_LIBRARIES)
        message(STATUS "[SDL2W]     \"${SDL2W_TMP_LOCATION}\"")

        string(FIND ${SDL2W_TMP_LOCATION} "SDL2-static" SDL2W_TMP_SDL2_STATIC_POS REVERSE)
        string(FIND ${SDL2W_TMP_LOCATION} "SDL2main" SDL2W_TMP_SDL2MAIN_POS REVERSE)
        string(FIND ${SDL2W_TMP_LOCATION} "SDL2" SDL2W_TMP_SDL2_POS REVERSE)

        if (NOT (SDL2W_TMP_SDL2_STATIC_POS LESS 0))
            set(SDL2W_TMP_SDL2_STATIC_LIB_LOCATION ${SDL2W_TMP_LOCATION})
        elseif (NOT (SDL2W_TMP_SDL2MAIN_POS LESS 0))
            set(SDL2W_TMP_SDL2MAIN_LIB_LOCATION ${SDL2W_TMP_LOCATION})
        elseif (NOT (SDL2W_TMP_SDL2_POS LESS 0))
            set(SDL2W_TMP_SDL2_LIB_LOCATION ${SDL2W_TMP_LOCATION})
        endif ()
    endforeach ()

    if (SDL2W_TMP_SDL2_STATIC_LIB_LOCATION AND SDL2W_TMP_USE_STATIC)
        list(APPEND SDL2W_TMP_SDL2_LINK_LIBS ${SDL2W_TMP_SDL2_STATIC_LIB_LOCATION})
    else ()
        list(APPEND SDL2W_TMP_SDL2_LINK_LIBS ${SDL2W_TMP_SDL2_LIB_LOCATION})
    endif ()

    list(APPEND SDL2W_TMP_SDL2MAIN_LINK_LIBS ${SDL2W_TMP_SDL2MAIN_LIB_LOCATION})
elseif (SDL2W_TMP_FOUND_MSVC_LIBS)
    message(STATUS "[SDL2W]     \"${SDL2W_TMP_MSVC_SDL2_LIB}\"")
    list(APPEND SDL2W_TMP_SDL2_LINK_LIBS ${SDL2W_TMP_MSVC_SDL2_LIB})

    message(STATUS "[SDL2W]     \"${SDL2W_TMP_MSVC_SDL2MAIN_LIB}\"")
    list(APPEND SDL2W_TMP_SDL2MAIN_LINK_LIBS ${SDL2W_TMP_MSVC_SDL2MAIN_LIB})
elseif (SDL2W_TMP_FOUND_MINGW_LIBS)
    if (SDL2W_TMP_USE_STATIC)
        message(STATUS "[SDL2W]     \"${SDL2W_TMP_MINGW_LIBSDL2_A}\"")
        list(APPEND SDL2W_TMP_SDL2_LINK_LIBS ${SDL2W_TMP_MINGW_LIBSDL2_A})
    else ()
        message(STATUS "[SDL2W]     \"${SDL2W_TMP_MINGW_LIBSDL2_DLL_A}\"")
        list(APPEND SDL2W_TMP_SDL2_LINK_LIBS ${SDL2W_TMP_MINGW_LIBSDL2_DLL_A})
    endif ()

    message(STATUS "[SDL2W]     \"${SDL2W_TMP_MINGW_LIBSDL2MAIN_A}\"")
    list(APPEND SDL2W_TMP_SDL2MAIN_LINK_LIBS ${SDL2W_TMP_MINGW_LIBSDL2MAIN_A})
else ()
    message(FATAL_ERROR "[SDL2W] Unsupported configuration.")
endif ()


# Get includedirectories.
#
if (SDL2W_TMP_FOUND_TARGETS)
    if (SDL2W_TMP_USE_STATIC AND SDL2W_TMP_FOUND_STATIC_TARGET)
        get_target_property(SDL2W_TMP_SDL2_INC_DIRS SDL2::SDL2-static INTERFACE_INCLUDE_DIRECTORIES)
    else ()
        get_target_property(SDL2W_TMP_SDL2_INC_DIRS SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)
    endif ()
elseif (SDL2W_TMP_FOUND_CONFIG)
    set(SDL2W_TMP_SDL2_INC_DIRS ${SDL2_INCLUDE_DIRS})
elseif (SDL2W_TMP_FOUND_MSVC_LIBS)
    set(SDL2W_TMP_SDL2_INC_DIRS ${SDL2W_TMP_MSVC_INCLUDE_DIR})
elseif (SDL2W_TMP_FOUND_MINGW_LIBS)
    set(SDL2W_TMP_SDL2_INC_DIRS ${SDL2W_TMP_MINGW_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "[SDL2W] Include directories not found.")
endif ()


# Find header filewith version.
#
if (NOT SDL2W_TMP_SDL2_INC_DIRS)
    message(FATAL_ERROR "[SDL2W] Empty includedirs list.")
endif ()

message(STATUS "[SDL2W] Include directories:")

foreach (SDL2W_TMP_INCLUDE_DIR IN LISTS SDL2W_TMP_SDL2_INC_DIRS)
    message(STATUS "[SDL2W]    \"${SDL2W_TMP_INCLUDE_DIR}\"")
endforeach ()

foreach (SDL2W_TMP_INCLUDE_DIR IN LISTS SDL2W_TMP_SDL2_INC_DIRS)
    set(SDL2W_TMP_SDL_VERSION_H ${SDL2W_TMP_INCLUDE_DIR}/SDL_version.h)

    # [MSYS2] Convert POSIX path to Windows one.
    #
    if (MINGW)
        unset(SDL2W_TMP_SDL_VERSION_H_MSYS2)

        execute_process(
            COMMAND "sh" "-c" "cmd //c echo ${SDL2W_TMP_SDL_VERSION_H}"
            TIMEOUT 7
            OUTPUT_VARIABLE SDL2W_TMP_SDL_VERSION_H_MSYS2
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if (SDL2W_TMP_SDL_VERSION_H_MSYS2 MATCHES "SDL_version\\.h$")
            set(SDL2W_TMP_SDL_VERSION_H ${SDL2W_TMP_SDL_VERSION_H_MSYS2})
        endif ()
    endif ()

    # Extract version.
    #
    if (EXISTS ${SDL2W_TMP_SDL_VERSION_H})
        set(SDL2W_TMP_MAJOR_REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$")
        set(SDL2W_TMP_MINOR_REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$")
        set(SDL2W_TMP_PATCH_REGEX"^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$")

        file(STRINGS
            ${SDL2W_TMP_SDL_VERSION_H}
            SDL2W_TMP_MAJOR_VERSION_STRING
            REGEX ${SDL2W_TMP_MAJOR_REGEX}
        )

        file(STRINGS
            ${SDL2W_TMP_SDL_VERSION_H}
            SDL2W_TMP_MINOR_VERSION_STRING
            REGEX ${SDL2W_TMP_MINOR_REGEX}
        )

        file(STRINGS
            ${SDL2W_TMP_SDL_VERSION_H}
            SDL2W_TMP_PATCH_VERSION_STRING
            REGEX ${SDL2W_TMP_PATCH_REGEX}
        )

        string(REGEX REPLACE
            ${SDL2W_TMP_MAJOR_REGEX}
            "\\1"
            SDL2W_TMP_MAJOR_VERSION
            ${SDL2W_TMP_MAJOR_VERSION_STRING}
        )

        string(REGEX REPLACE
            ${SDL2W_TMP_MINOR_REGEX}
            "\\1"
            SDL2W_TMP_MINOR_VERSION
            ${SDL2W_TMP_MINOR_VERSION_STRING}
        )

        string(REGEX REPLACE
            ${SDL2W_TMP_PATCH_REGEX}
            "\\1"
            SDL2W_TMP_PATCH_VERSION
            ${SDL2W_TMP_PATCH_VERSION_STRING}
        )

        set(SDL2W_TMP_DIGIT_REGEX "^[0-9]+$")

        if (SDL2W_TMP_MAJOR_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX} AND
            SDL2W_TMP_MINOR_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX} AND
            SDL2W_TMP_PATCH_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX}
            )
            if (NOT ${SDL2W_TMP_MAJOR_VERSION} EQUAL 2)
                message(FATAL_ERROR "[SDL2W] Unsupported major version (got: ${SDL2W_TMP_MAJOR_VERSION}; expected: 2).")
            endif ()

            set(SDL2W_TMP_VERSION_STRING
                ${SDL2W_TMP_MAJOR_VERSION}.${SDL2W_TMP_MINOR_VERSION}.${SDL2W_TMP_PATCH_VERSION}
            )
        endif ()

        break()
    endif ()
endforeach ()

if (SDL2W_TMP_VERSION_STRING)
    message(STATUS "[SDL2W] Found SDL version: ${SDL2W_TMP_VERSION_STRING}")
else ()
    message(FATAL_ERROR "[SDL2W] Failed to detect SDL version.")
endif ()


# Default handler.
#
include(FindPackageHandleStandardArgs)

set(SDL2W_TMP_REQUIRED_VARS "")

if (SDL2W_TMP_FOUND_TARGETS)
    set(SDL2W_TMP_REQUIRED_VARS SDL2_FOUND)
elseif (SDL2W_TMP_FOUND_CONFIG)
    list(APPEND SDL2W_TMP_REQUIRED_VARS SDL2_INCLUDE_DIRS)
    list(APPEND SDL2W_TMP_REQUIRED_VARS SDL2_LIBRARIES)
elseif (SDL2W_TMP_FOUND_MSVC_LIBS)
    list(APPEND SDL2W_TMP_REQUIRED_VARS SDL2W_TMP_MSVC_INCLUDE_DIR)
    list(APPEND SDL2W_TMP_REQUIRED_VARS SDL2W_TMP_MSVC_SDL2_LIB)
    list(APPEND SDL2W_TMP_REQUIRED_VARS SDL2W_TMP_MSVC_SDL2MAIN_LIB)
elseif (SDL2W_TMP_FOUND_MINGW_LIBS)
    list(APPEND SDL2W_TMP_REQUIRED_VARS SDL2W_TMP_MINGW_INCLUDE_DIR)
    list(APPEND SDL2W_TMP_REQUIRED_VARS SDL2W_TMP_MINGW_LIBSDL2_A)
    list(APPEND SDL2W_TMP_REQUIRED_VARS SDL2W_TMP_MINGW_LIBSDL2_DLL_A)
    list(APPEND SDL2W_TMP_REQUIRED_VARS SDL2W_TMP_MINGW_LIBSDL2MAIN_A)
else ()
    message(FATAL_ERROR "[SDL2W] Invalid configuration.")
endif ()

find_package_handle_standard_args(
    ${CMAKE_FIND_PACKAGE_NAME}
    REQUIRED_VARS ${SDL2W_TMP_REQUIRED_VARS}
    VERSION_VAR SDL2W_TMP_VERSION_STRING
)

# Add additional system libraries for static linking.
#
if (SDL2W_TMP_USE_STATIC)
    list(APPEND SDL2W_TMP_SDL2_LINK_LIBS "${CMAKE_DL_LIBS}")
endif ()

if (WIN32 AND SDL2W_TMP_USE_STATIC)
    list(APPEND SDL2W_TMP_SDL2_LINK_LIBS "imm32")
    list(APPEND SDL2W_TMP_SDL2_LINK_LIBS "setupapi")
    list(APPEND SDL2W_TMP_SDL2_LINK_LIBS "version")
    list(APPEND SDL2W_TMP_SDL2_LINK_LIBS "winmm")
endif ()

# Add target.
#
if (NOT TARGET ${SDL2W_TMP_TARGET})
    add_library(${SDL2W_TMP_TARGET} INTERFACE IMPORTED)

    set_target_properties(${SDL2W_TMP_TARGET} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2W_TMP_SDL2_INC_DIRS}"
    )

    set_target_properties(${SDL2W_TMP_TARGET} PROPERTIES
        INTERFACE_LINK_LIBRARIES "${SDL2W_TMP_SDL2_LINK_LIBS}"
    )
endif ()

# Add main target.
#
if (NOT TARGET ${SDL2W_TMP_TARGET_MAIN})
    add_library(${SDL2W_TMP_TARGET_MAIN} INTERFACE IMPORTED)

    set_target_properties(${SDL2W_TMP_TARGET_MAIN} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2W_TMP_SDL2_INC_DIRS}"
    )

    if (SDL2W_TMP_SDL2MAIN_LINK_LIBS)
        set_target_properties(${SDL2W_TMP_TARGET_MAIN} PROPERTIES
            INTERFACE_LINK_LIBRARIES "${SDL2W_TMP_SDL2MAIN_LINK_LIBS}"
        )
    endif ()
endif ()

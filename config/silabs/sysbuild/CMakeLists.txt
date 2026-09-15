#
#   Copyright (c) 2026 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

# Sysbuild hooks for the connectedhomeip Zephyr module.
#
# ExternalZephyrProject_Add only forwards a fixed variable list; USE_CCACHE is not among
# them (only a SYSBUILD_CACHE target property, loaded after ccache.cmake runs). Without
# this hook, -DUSE_CCACHE=0 is ignored for mcuboot and the app.
#
# App CMakeLists silences -Wdocumentation/-Wc99-extensions on mbedtls for the app image
# only. MCUboot is a separate project, so inject via EXTRA_CFLAGS/EXTRA_CXXFLAGS.

function(connectedhomeip_PRE_CMAKE)
    cmake_parse_arguments(ARG "" "" "IMAGES" ${ARGN})

    set(_mcuboot_extra_cflags "-Wno-documentation -Wno-c99-extensions")

    set(_disable_ccache FALSE)
    if("$CACHE{USE_CCACHE}" STREQUAL "0" OR "${USE_CCACHE}" STREQUAL "0")
        set(_disable_ccache TRUE)
        set(USE_CCACHE "0" CACHE STRING "Disable ccache for all sysbuild images" FORCE)
        message(STATUS "USE_CCACHE=0: forwarding -DUSE_CCACHE:STRING=0 to sysbuild images")
    endif()

    foreach(image ${ARG_IMAGES})
        if(NOT TARGET ${image})
            continue()
        endif()
        get_property(_cmake_args TARGET ${image} PROPERTY _EP_CMAKE_ARGS)

        set(_already_disables_ccache FALSE)
        if("-DUSE_CCACHE:STRING=0" IN_LIST _cmake_args)
            set(_already_disables_ccache TRUE)
        endif()
        if(_disable_ccache AND NOT _already_disables_ccache)
            set_property(TARGET ${image} APPEND PROPERTY _EP_CMAKE_ARGS "-DUSE_CCACHE:STRING=0")
        endif()

        set(_already_has_extra_cflags FALSE)
        if("-DEXTRA_CFLAGS:STRING=${_mcuboot_extra_cflags}" IN_LIST _cmake_args)
            set(_already_has_extra_cflags TRUE)
        endif()
        if(image STREQUAL "mcuboot" AND NOT _already_has_extra_cflags)
            set_property(TARGET ${image} APPEND PROPERTY _EP_CMAKE_ARGS
                "-DEXTRA_CFLAGS:STRING=${_mcuboot_extra_cflags}"
                "-DEXTRA_CXXFLAGS:STRING=${_mcuboot_extra_cflags}")
        endif()
    endforeach()
endfunction()

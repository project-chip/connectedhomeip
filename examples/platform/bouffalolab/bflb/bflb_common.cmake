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

option(BFLB_LTO_OFF
       "Disable Bouffalo SDK LTO paths that fail with the current T-Head GCC toolchain on macOS Apple Silicon"
       ON)

function(bflb_remove_lto_option target property)
  get_target_property(_opts ${target} ${property})
  if(_opts)
    list(FILTER _opts EXCLUDE REGEX "^-flto(=.*)?$")
    set_target_properties(${target} PROPERTIES ${property} "${_opts}")
  endif()
endfunction()

macro(bflb_remove_lto)
  if(BFLB_LTO_OFF)
    # The current T-Head GCC 10.2 LTO flow fails on macOS Apple Silicon:
    # - Matter LTO objects fail during static library archiving.
    # - BL616 prebuilt Wi-Fi archives can fail lto-wrapper with
    #   "original not compressed with zstd" at final link.
    # Linux hosts have not reproduced these failures. When the toolchain is
    # updated, disable BFLB_LTO_OFF to restore LTO quickly.
    bflb_remove_lto_option(sdk_intf_lib INTERFACE_COMPILE_OPTIONS)
    bflb_remove_lto_option(sdk_intf_lib INTERFACE_LINK_OPTIONS)

    foreach(_flags_var MATTER_CFLAGS MATTER_CFLAGS_C MATTER_CFLAGS_CC)
      if(DEFINED ${_flags_var})
        list(FILTER ${_flags_var} EXCLUDE REGEX "^-flto(=.*)?$")
      endif()
    endforeach()

    sdk_add_link_options(-fno-use-linker-plugin)
  endif()
endmacro()

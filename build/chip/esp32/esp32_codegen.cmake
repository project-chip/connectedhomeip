#
# SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#



macro(chip_app_component_codegen IDL_NAME)
  include("${CHIP_ROOT}/build/chip/chip_codegen.cmake")

  # The IDF build system performs a two-pass expansion to determine
  # component expansion. The first pass runs in script-mode
  # to determine idf_component_register REQUIRES and PRIV_REQUIRES.
  #
  # We can only set up code generation during the 2nd pass
  #
  # see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html
  if (NOT CMAKE_BUILD_EARLY_EXPANSION)
  
    chip_codegen(app-codegen
      INPUT     "${IDL_NAME}"
      GENERATOR "cpp-app"
      OUTPUTS
            "app/PluginApplicationCallbacks.h"
      OUTPUT_PATH   APP_GEN_DIR
      OUTPUT_FILES  APP_GEN_FILES
    )

    target_include_directories(${COMPONENT_LIB} PUBLIC "${APP_GEN_DIR}")

    add_dependencies(${COMPONENT_LIB} app-codegen)
  endif()
endmacro()

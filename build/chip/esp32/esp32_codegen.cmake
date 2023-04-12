#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
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
            "app/callback-stub.cpp"
      OUTPUT_PATH   APP_GEN_DIR
      OUTPUT_FILES  APP_GEN_FILES
    )

    target_include_directories(${COMPONENT_LIB} PUBLIC "${APP_GEN_DIR}")

    add_dependencies(${COMPONENT_LIB} app-codegen)

    target_sources(${COMPONENT_LIB} PRIVATE ${APP_GEN_FILES})
  endif()
endmacro()

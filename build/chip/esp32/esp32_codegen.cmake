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
  include("${CHIP_ROOT}/src/app/codegen-data-model-provider/model.cmake")

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
            "app/cluster-init-callback.cpp"
      OUTPUT_PATH   APP_GEN_DIR
      OUTPUT_FILES  APP_GEN_FILES
    )

    add_dependencies(${COMPONENT_LIB} app-codegen)
    target_include_directories(${COMPONENT_LIB} PUBLIC "${APP_GEN_DIR}")
    target_sources(${COMPONENT_LIB} PRIVATE ${APP_GEN_FILES})
  endif()
endmacro()

macro(chip_app_component_zapgen ZAP_NAME)
  include("${CHIP_ROOT}/build/chip/chip_codegen.cmake")

  # The IDF build system performs a two-pass expansion to determine
  # component expansion. The first pass runs in script-mode
  # to determine idf_component_register REQUIRES and PRIV_REQUIRES.
  #
  # We can only set up code generation during the 2nd pass
  #
  # see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html
  if (NOT CMAKE_BUILD_EARLY_EXPANSION)

    chip_zapgen(app-zapgen
      INPUT     "${ZAP_NAME}"
      GENERATOR "app-templates"
      OUTPUTS
            "zap-generated/access.h"
            "zap-generated/endpoint_config.h"
            "zap-generated/gen_config.h"
            "zap-generated/IMClusterCommandHandler.cpp"
      OUTPUT_PATH   APP_TEMPLATE_GEN_DIR
      OUTPUT_FILES  APP_TEMPLATE_GEN_FILES
    )

    add_dependencies(${COMPONENT_LIB} app-zapgen)
    target_include_directories(${COMPONENT_LIB} PUBLIC "${APP_TEMPLATE_GEN_DIR}")
    target_sources(${COMPONENT_LIB} PRIVATE ${APP_TEMPLATE_GEN_FILES})

    # When data model interface is used, provide a default code-generation data model as
    # part of zapgen. See `chip_data_model.cmake` for similar logic
    set(CHIP_DATA_MODEL_INTERFACE "disabled" CACHE STRING "Data model interface option to use: enabled or disabled")

    if ("${CHIP_DATA_MODEL_INTERFACE}" STREQUAL "enabled")
      target_sources(${COMPONENT_LIB} PRIVATE ${CODEGEN_DATA_MODEL_SOURCES})
    endif()

  endif()
endmacro()

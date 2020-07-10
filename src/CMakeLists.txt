#
#
#    Copyright (c) 2020 Project CHIP Authors
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

#
#    Description:
#      This file is the CMake template for the CHIP SDK source code.
#

add_subdirectory(include)

add_subdirectory(app)
add_subdirectory(ble)
add_subdirectory(crypto)
add_subdirectory(inet)
add_subdirectory(lib)
add_subdirectory(lwip)
add_subdirectory(system)
add_subdirectory(transport)

if(CONFIG_DEVICE_LAYER)
add_subdirectory(platform)
endif()

if(CHIP_TARGET_STYLE_UNIX)
add_subdirectory(setup_payload)
endif()

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

option(CHIP_BIG_ENDIAN "host platform uses big-endian byte order")
if(CHIP_BIG_ENDIAN)
    list(APPEND CHIP_PRIVATE_DEFINES "BYTE_ORDER_BIG_ENDIAN=1")
endif()

option(CHIP_EXTERNAL_HEAP "enable external heap support")
if(CHIP_EXTERNAL_HEAP)
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_EXTERNAL_HEAP_ENABLE=1")
endif()

option(CHIP_SETTINGS_RAM "enable volatile-only storage of settings")
if(CHIP_SETTINGS_RAM)
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_SETTINGS_RAM=1")
endif()

option(CHIP_FULL_LOGS "enable full logs")
if(CHIP_FULL_LOGS)
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_LEVEL=CHIP_LOG_LEVEL_DEBG")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_API=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_ARP=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_CLI=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_COAP=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_ICMP=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_IP6=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_MAC=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_MEM=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_MLE=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_NETDATA=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_NETDIAG=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_PKT_DUMP=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_PLATFORM=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_PREPEND_LEVEL=1")
    list(APPEND CHIP_PRIVATE_DEFINES "CHIP_CONFIG_LOG_PREPEND_REGION=1")
endif()

option(CHIP_BUILD_NLFRIENDS_WITH_AUTOTOOLS
    "Use autotools to build third_party/nlfriends. Otherwise, use native cmake."
    OFF
)

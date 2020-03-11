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

option(CHIP_BUILD_NLFRIENDS_WITH_AUTOTOOLS
    "Use autotools to build third_party/nlfriends. Otherwise, use native cmake."
    OFF
)

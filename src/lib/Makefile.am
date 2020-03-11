#
#    <COPYRIGHT>
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
#      This file is the GNU automake template for the CHIP library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

# Pull in the sources that comprise the Chip library.

include ../system/SystemLayer.am
include ../inet/InetLayer.am
include ../ble/BleLayer.am
include core/CoreLayer.am
include support/SupportLayer.am

lib_LIBRARIES                       = libCHIP.a

libCHIP_a_CPPFLAGS                  =                         \
    -I$(top_srcdir)/src                                       \
    -I$(top_srcdir)/src/include                               \
    -I$(top_srcdir)/src/inet                                  \
    -I$(top_srcdir)/src/lib                                   \
    -I$(top_srcdir)/src/lib/core                              \
    -I$(top_srcdir)/src/system                                \
    $(LWIP_CPPFLAGS)                                          \
    $(SOCKETS_CPPFLAGS)                                       \
    $(NULL)

libCHIP_a_SOURCES                   = $(chip_SystemLayer_sources)
libCHIP_a_SOURCES                  += $(chip_InetLayer_sources)
libCHIP_a_SOURCES                  += $(chip_CoreLayer_sources)
libCHIP_a_SOURCES                  += $(chip_SupportLayer_sources)

if CONFIG_NETWORK_LAYER_BLE
libCHIP_a_SOURCES                  += $(chip_BleLayer_sources)
endif # CONFIG_NETWORK_LAYER_BLE

libCHIP_adir                        = $(includedir)/lib/core
dist_libCHIP_a_HEADERS              = $(chip_CoreLayer_headers)
dist_libCHIP_a_HEADERS             += $(chip_SupportLayer_headers)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

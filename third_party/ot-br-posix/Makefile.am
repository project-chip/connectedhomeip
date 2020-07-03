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
#    Description:
#      This file is the GNU automake template for the CHIP in-package,
#      otbr clientlibrary.
#
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

if CHIP_DEVICE_LAYER_TARGET_LINUX
if CHIP_WITH_OT_BR_POSIX

lib_LIBRARIES = libot_br_client.a

nodist_libot_br_client_a_SOURCES                          = \
    repo/src/dbus/client/client_error.cpp                   \
    repo/src/dbus/client/thread_api_dbus.cpp                \
    repo/src/dbus/common/error.cpp                          \
    repo/src/dbus/common/dbus_message_helper.cpp            \
    repo/src/dbus/common/dbus_message_helper_openthread.cpp \
    $(NULL)

libot_br_client_a_CPPFLAGS                                = \
    $(OT_BR_POSIX_CPPFLAGS)                                 \
    $(DBUS_CFLAGS)                                          \
    $(NULL)

endif # CHIP_WITH_OT_BR_POSIX
endif # CHIP_DEVICE_LAYER_TARGET_LINUX

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

#
#    Copyright (c) 2020 Project CHIP Authors
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
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

include $(MAKE_INCLUDE_GEN)

.PHONY: all clean

#*****************************************************************************#
#                               VARIABLES	                              #
#*****************************************************************************#

DIR = $(SRCDIR)
DIR += $(CHIPDIR)/src/app
DIR += $(CHIPDIR)/src/app/server
DIR += $(CHIPDIR)/examples/all-clusters-app/all-clusters-common
DIR += $(CHIPDIR)/examples/all-clusters-app/ambd/main

vpath %.cpp $(DIR) $(shell find $(DIR) -type d)
vpath %.c $(DIR) $(shell find $(DIR) -type d)
#*****************************************************************************#
#                               CFLAGS                                        #
#*****************************************************************************#

GLOBAL_CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_LWIP=1
GLOBAL_CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_SOCKETS=0
GLOBAL_CFLAGS += -DCHIP_SYSTEM_CONFIG_POSIX_LOCKING=0
GLOBAL_CFLAGS += -DINET_CONFIG_ENABLE_IPV4=1
GLOBAL_CFLAGS += -DCHIP_PROJECT=1
GLOBAL_CFLAGS += -DCHIP_DEVICE_LAYER_TARGET=AMBD
GLOBAL_CFLAGS += -DUSE_ZAP_CONFIG
GLOBAL_CFLAGS += -DCHIP_HAVE_CONFIG_H
GLOBAL_CFLAGS += -DLWIP_IPV6_SCOPES=0

GLOBAL_CFLAGS += -DLWIP_IPV6_ND=0
GLOBAL_CFLAGS += -DLWIP_IPV6_SCOPES=0
GLOBAL_CFLAGS += -DLWIP_PBUF_FROM_CUSTOM_POOLS=0

GLOBAL_CFLAGS += -DCHIP_DEVICE_LAYER_NONE=0
GLOBAL_CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF=0
GLOBAL_CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS=0
GLOBAL_CFLAGS += -DCHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS=0

#CPPFLAGS += -std=gnu++11
#CPPFLAGS += -std=c++14
#CPPFLAGS += -fno-rtti
CPPFLAGS += -Wno-unused-parameter

#*****************************************************************************#
#                               Include FILE LIST                             #
#*****************************************************************************#

#IFLAGS = -I$(DIR)./
IFLAGS += -I$(CHIPDIR)/src/include/
IFLAGS += -I$(CHIPDIR)/src/lib/
IFLAGS += -I$(CHIPDIR)/src/
IFLAGS += -I$(CHIPDIR)/third_party/nlassert/repo/include/
IFLAGS += -I$(CHIPDIR)/src/app/
IFLAGS += -I$(CHIPDIR)/src/app/util/
IFLAGS += -I$(CHIPDIR)/src/app/server/
IFLAGS += -I$(CHIPDIR)/third_party/nlio/repo/include/
IFLAGS += -I$(CHIPDIR)/examples/all-clusters-app/all-clusters-common
IFLAGS += -I$(CHIPDIR)/examples/all-clusters-app/all-clusters-common/gen
IFLAGS += -I$(CHIPDIR)/examples/all-clusters-app/ambd/main/include
IFLAGS += -I$(CHIPDIR)/examples/all-clusters-app/ambd/build/chip/gen/include
IFLAGS += -I$(CHIPDIR)/third_party/nlunit-test/repo/src


#*****************************************************************************#
#                               Source FILE LIST                              #
#*****************************************************************************#

#chip app
CPPSRC += $(CHIPDIR)/src/app/Command.cpp
CPPSRC += $(CHIPDIR)/src/app/CommandHandler.cpp
CPPSRC += $(CHIPDIR)/src/app/CommandSender.cpp
CPPSRC += $(CHIPDIR)/src/app/decoder.cpp
CPPSRC += $(CHIPDIR)/src/app/encoder.cpp
CPPSRC += $(CHIPDIR)/src/app/InteractionModelEngine.cpp
CPPSRC += $(CHIPDIR)/src/app/ReadClient.cpp
CPPSRC += $(CHIPDIR)/src/app/ReadHandler.cpp

CPPSRC += $(CHIPDIR)/src/app/server/Server.cpp
CPPSRC += $(CHIPDIR)/src/app/server/DataModelHandler.cpp
CPPSRC += $(CHIPDIR)/src/app/server/RendezvousServer.cpp
CPPSRC += $(CHIPDIR)/src/app/server/Mdns.cpp
CPPSRC += $(CHIPDIR)/src/app/server/EchoHandler.cpp
CPPSRC += $(CHIPDIR)/src/app/server/OnboardingCodesUtil.cpp
CPPSRC += $(CHIPDIR)/src/app/server/StorablePeerConnection.cpp         #Base0505

CPPSRC += $(CHIPDIR)/src/app/util/af-event.cpp
CPPSRC += $(CHIPDIR)/src/app/util/af-main-common.cpp
CPPSRC += $(CHIPDIR)/src/app/util/attribute-size.cpp                   #Base0505 fix conflicting file name 'all-clusters-common/gen/attribute-size.cpp'
CPPSRC += $(CHIPDIR)/src/app/util/attribute-storage.cpp
CPPSRC += $(CHIPDIR)/src/app/util/attribute-table.cpp
CPPSRC += $(CHIPDIR)/src/app/util/attribute-list-byte-span.cpp         #Base0518
CPPSRC += $(CHIPDIR)/src/app/util/attribute-size-util.cpp              #Base0617
CPPSRC += $(CHIPDIR)/src/app/util/binding-table.cpp
CPPSRC += $(CHIPDIR)/src/app/util/CHIPDeviceCallbacksMgr.cpp
CPPSRC += $(CHIPDIR)/src/app/util/chip-message-send.cpp
CPPSRC += $(CHIPDIR)/src/app/util/client-api.cpp
CPPSRC += $(CHIPDIR)/src/app/util/ember-compatibility-functions.cpp
CPPSRC += $(CHIPDIR)/src/app/util/ember-print.cpp
CPPSRC += $(CHIPDIR)/src/app/util/esi-management.cpp
CPPSRC += $(CHIPDIR)/src/app/util/message.cpp
CPPSRC += $(CHIPDIR)/src/app/util/process-cluster-message.cpp
CPPSRC += $(CHIPDIR)/src/app/util/process-global-message.cpp
CPPSRC += $(CHIPDIR)/src/app/util/util.cpp

CPPSRC += $(CHIPDIR)/src/app/clusters/on-off-server/on-off-server.cpp                                         #Base0429
CPPSRC += $(CHIPDIR)/src/app/clusters/level-control/level-control.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/identify/identify.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/barrier-control-server/barrier-control-server.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/groups-server/groups-server.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/color-control-server/color-control-server.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/content-launch-server/content-launch-server.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/media-playback-server/media-playback-server.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/temperature-measurement-server/temperature-measurement-server.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/scenes/scenes.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/basic/basic.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/bindings/bindings.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/door-lock-server/door-lock-server-core.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/door-lock-server/door-lock-server-logging.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/door-lock-server/door-lock-server-schedule.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/door-lock-server/door-lock-server-user.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/ias-zone-server/ias-zone-server.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/general-commissioning-server/general-commissioning-server.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/low-power-server/low-power-server.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/network-commissioning/network-commissioning.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/network-commissioning/network-commissioning-ember.cpp
CPPSRC += $(CHIPDIR)/src/app/clusters/descriptor/descriptor.cpp                                               #Base0429
CPPSRC += $(CHIPDIR)/src/app/clusters/operational-credentials-server/operational-credentials-server.cpp       #Base0429 + #Base0505
CPPSRC += $(CHIPDIR)/src/app/clusters/ota-server/ota-server.cpp                                               #Base0429
CPPSRC += $(CHIPDIR)/src/app/clusters/test-cluster-server/test-cluster-server.cpp                             #Base0505
#CPPSRC += $(CHIPDIR)/src/app/clusters/trusted-root-certificates-server/trusted-root-certificates-server.cpp   #Base0518 + Base0614
CPPSRC += $(CHIPDIR)/src/app/clusters/occupancy-sensor-server/occupancy-sensor-server.cpp                     #Base0531
CPPSRC += $(CHIPDIR)/src/app/clusters/pump-configuration-and-control-server/pump-configuration-and-control-server.cpp #Base0622

CPPSRC += $(CHIPDIR)/src/app/reporting/reporting-default-configuration.cpp
CPPSRC += $(CHIPDIR)/src/app/reporting/reporting.cpp

#copy from examples/all-clusters-app/all-clusters-common/gen/attribute-size.cpp
#Resolved same file name issue. (src/app/util/attribute-size.cpp)
CPPSRC += $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/gen/attribute-size_copy.cpp                #Base0429 + Base0505
CPPSRC += $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/gen/CHIPClientCallbacks.cpp
#CPPSRC += $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/gen/call-command-handler.cpp              #Base0608
CPPSRC += $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/gen/callback-stub.cpp
CPPSRC += $(CHIPDIR)/examples/all-clusters-app/all-clusters-common/gen/IMClusterCommandHandler.cpp

CPPSRC += $(CHIPDIR)/examples/all-clusters-app/ambd/main/chipinterface.cpp
CPPSRC += $(CHIPDIR)/examples/all-clusters-app/ambd/main/DeviceCallbacks.cpp
CPPSRC += $(CHIPDIR)/examples/all-clusters-app/ambd/main/CHIPDeviceManager.cpp

#*****************************************************************************#
#                               Object FILE LIST                              #
#*****************************************************************************#
CPPOBJS = $(notdir $(CPPSRC:.cpp=.o))
COBJS = $(notdir $(CSRC:.c=.o))
OBJS = $(CPPOBJS)
OBJS += $(COBJS)
STATIC_LIB = lib_main.a

#*****************************************************************************#
#                               Include Dependency                            #
#*****************************************************************************#
-include $(OBJS:.o=.d)


#*****************************************************************************#
#                        RULES TO GENERATE TARGETS                            #
#*****************************************************************************#

# Define the Rules to build the core targets
all: CORE_TARGETS

#*****************************************************************************#
#              GENERATE OBJECT FILE
#*****************************************************************************#
#CORE_TARGETS:	$(CPPOBJS) $(COBJS)
CORE_TARGETS:	$(OBJS) $(STATIC_LIB)
$(STATIC_LIB):$(OBJS)
	$(AR) rvs $@ $^
	$(MOVE) -f $@ $(ROOTDIR)/lib/application

#*****************************************************************************#
#              CLEAN GENERATED FILES                                          #
#*****************************************************************************#
clean:
	rm -f $(ROOTDIR)/make/chip_main/*.o
	rm -f $(ROOTDIR)/make/chip_main/*.i
	rm -f $(ROOTDIR)/make/chip_main/*.s
	rm -f $(ROOTDIR)/make/chip_main/*.d
	rm -f $(ROOTDIR)/make/chip_main/*.ii
	rm -f $(ROOTDIR)/make/chip_main/*.su

-include $(DEPS)

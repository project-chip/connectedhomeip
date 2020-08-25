#
#    Copyright (c) 2014-2017 Nest Labs, Inc.
#    Copyright (c) 2018 Google LLC
#	   Copyright (c) 2020 Project CHIP Authors
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

#
#    Description:
#      This file is the GNU automake template for the CHIP Setup Payload Parser.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

#if CHIP_WITH_JAVA

EXTRA_DIST                                        = \
    $(srcdir)/src                                   \
    $(NULL)

lib_LTLIBRARIES                                   = libSetupPayloadParser.la

libSetupPayloadParser_la_CPPFLAGS                       = \
    -I$(top_srcdir)/src                                   \
    -I$(top_srcdir)/src/lib                               \
    -I$(top_srcdir)/src/lib/core                          \
    -I$(top_srcdir)/src/setup_payload                     \
    -I$(top_srcdir)/src/system                            \
    -I$(top_srcdir)/src/include                           \
    $(NLASSERT_CPPFLAGS)                                  \
    $(NLFAULTINJECTION_CPPFLAGS)                          \
    $(NLIO_CPPFLAGS)                                      \
    $(JNI_CPPFLAGS)                                       \
    $(LWIP_CPPFLAGS)                                      \
    $(SOCKETS_CPPFLAGS)                                   \
		$(NULL)

libSetupPayloadParser_la_CXXFLAGS                       = \
    $(NULL)

libSetupPayloadParser_la_LDFLAGS                        = \
    $(AM_LDFLAGS)                                         \
    -avoid-version                                        \
    -module                                               \
    -export-dynamic                                       \
    $(NULL)

libSetupPayloadParser_la_SOURCES                        = \
    SetupPayloadParser-JNI.cpp                            \
    $(NULL)

libSetupPayloadParser_la_LIBADD                               = \
    -L$(top_builddir)/src/setup_payload -lSetupPayload          \
    -L$(top_builddir)/src/lib -lCHIP                            \
    $(LWIP_LDFLAGS) $(LWIP_LIBS)                                \
    $(SOCKETS_LDFLAGS) $(SOCKETS_LIBS)                          \
    $(NULL)

if CHIP_WITH_ANDROID
libSetupPayloadParser_la_LIBADD                              += \
    -llog                                                       \
    $(NULL)
endif # CHIP_WITH_ANDROID

libSetupPayloadParser_la_DEPENDENCIES                         = \
    $(top_builddir)/src/setup_payload/libSetupPayload.a         \
    $(top_builddir)/src/lib/libCHIP.a                           \
    $(NULL)

JARS                                                          = \
    SetupPayloadParser.jar                                      \
    $(NULL)

SetupPayloadParser_jar_JAVA_SRCS                              = \
    chip/setuppayload/SetupPayload.java                         \
    chip/setuppayload/SetupPayloadParser.java                   \
    chip/setuppayload/OptionalQRCodeInfo.java                   \
    $(NULL)

SetupPayloadParser_jar_JFLAGS                      = -source 8 -target 8

if CHIP_WITH_ANDROID
SetupPayloadParser_jar_JAVA_CLASSPATHS             = $(ANDROID_HOME)/platforms/android-21/android.jar
endif # CHIP_WITH_ANDROID

include $(abs_top_srcdir)/scripts/make/java.mk

all-local: all-JARS
install-data-local: install-data-JARS
uninstall-local: uninstall-JARS
mostlyclean-local: mostlyclean-local-JARS

#endif # CHIP_WITH_JAVA

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

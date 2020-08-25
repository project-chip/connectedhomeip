#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2014-2017 Nest Labs, Inc.
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
#      This file is the GNU automake template for the CHIP
#      Java Device Controller library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

if CHIP_WITH_JAVA

EXTRA_DIST                                   = \
    $(srcdir)/src                              \
    $(NULL)

lib_LTLIBRARIES                              = libCHIPController.la

libCHIPController_la_CPPFLAGS                              = \
    -I$(top_srcdir)/src                                      \
    -I$(top_srcdir)/src/lib                                  \
    -I$(top_srcdir)/src/lib/core                             \
    -I$(top_srcdir)/src/controller                           \
    -I$(top_srcdir)/src/system                               \
    -I$(top_srcdir)/src/include                              \
    $(NLASSERT_CPPFLAGS)                                     \
    $(JNI_CPPFLAGS)                                          \
    $(LWIP_CPPFLAGS)                                         \
    $(SOCKETS_CPPFLAGS)                                      \
    $(NULL)

libCHIPController_la_CXXFLAGS                = \
    $(PTHREAD_CFLAGS)                          \
    $(NULL)

libCHIPController_la_LDFLAGS                 = \
    $(AM_LDFLAGS)                              \
    -avoid-version                             \
    -module                                    \
    -export-dynamic                            \
    $(NULL)

libCHIPController_la_SOURCES                 = \
    CHIPDeviceController-JNI.cpp               \
    $(NULL)

libCHIPController_la_LIBADD                  = \
    -L$(top_builddir)/src/lib -lCHIP           \
    $(LWIP_LDFLAGS) $(LWIP_LIBS)               \
    $(SOCKETS_LDFLAGS) $(SOCKETS_LIBS)         \
    $(PTHREAD_LIBS)                            \
    $(NULL)

if CHIP_WITH_ANDROID
libCHIPController_la_LIBADD                 += \
    -llog                                      \
    $(NULL)
endif # CHIP_WITH_ANDROID

libCHIPController_la_DEPENDENCIES            = \
    $(top_builddir)/src/lib/libCHIP.a          \
    $(NULL)

JARS                                         = \
    CHIPController.jar                         \
    $(NULL)

CHIPController_jar_JAVA_SRCS                                 = \
    chip/devicecontroller/ChipDeviceController.java            \
    chip/devicecontroller/ChipDeviceControllerException.java   \
    chip/devicecontroller/ChipCommandType.java                 \
    $(NULL)

CHIPController_jar_JFLAGS                    = -source 8 -target 8

if CHIP_WITH_ANDROID
CHIPController_jar_JAVA_CLASSPATHS           = $(ANDROID_HOME)/platforms/android-21/android.jar
endif # CHIP_WITH_ANDROID

include $(abs_top_srcdir)/scripts/make/java.mk

all-local: all-JARS
install-data-local: install-data-JARS
uninstall-local: uninstall-JARS
mostlyclean-local: mostlyclean-local-JARS

endif # CHIP_WITH_JAVA

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

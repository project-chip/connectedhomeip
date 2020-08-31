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
#      This file is the GNU automake template for the CHIP shell framework.
#
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

lib_LIBRARIES         = libChipShell.a

SUBDIRS               = $(NULL)

BUILT_SOURCES         = $(top_builddir)/src/include/CHIPVersion.h 

libChipShell_adir     = $(includedir)/shell

libChipShell_a_CPPFLAGS                                           = \
    -I$(top_srcdir)/src                                             \
    -I$(top_srcdir)/src/lib                                         \
    -I$(top_srcdir)/src/lib/shell                                   \
    $(NLASSERT_CPPFLAGS)                                            \
    $(NULL)

dist_libChipShell_a_HEADERS                                       = \
    @top_builddir@/src/lib/shell/shell.h                            \
    @top_builddir@/src/lib/shell/streamer.h                         \
    @top_builddir@/src/lib/shell/commands.h                         \
    $(NULL)

libChipShell_a_SOURCES                                            = \
    commands.cpp                                                    \
    shell.cpp                                                       \
    streamer.cpp                                                    \
    $(NULL)

if CHIP_TARGET_STYLE_UNIX

SUBDIRS                                                          += tests

libChipShell_a_SOURCES                                           += \
    streamer_stdio.cpp                                              \
    $(NULL)

endif

if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT

libChipShell_a_SOURCES                                           += \
    streamer_zephyr.cpp                                             \
    $(NULL)

endif

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

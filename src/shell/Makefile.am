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
#      pigweed embedded utility framework.
#
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                                                           = tests

lib_LIBRARIES                                                     = libChipShell.a

libChipShell_adir                                                 = $(includedir)/shell

nodist_libChipShell_a_SOURCES                                     = \
    commands.c                                                      \
    shell.c                                                         \
    streamer.c                                                      \
    streamer_stdio.c                                                \
    $(NULL)

libChipShell_a_CPPFLAGS                                           = \
    -I$(top_srcdir)/src                                             \
    -I$(top_srcdir)/src/shell                                       \
    $(NULL)

dist_libChipShell_a_HEADERS                                       = \
    @top_builddir@/src/shell/shell.h                                \
    @top_builddir@/src/shell/streamer.h                             \
    @top_builddir@/src/shell/commands.h                             \
    $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

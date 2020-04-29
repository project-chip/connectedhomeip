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
#      This file is the GNU automake template for the CHIP OSAL library
#      (Operating System Abstraction Layer).

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                                             = $(NULL)

lib_LIBRARIES                                       = libChipOsal.a

libChipOsal_adir                                    = $(includedir)/osal

dist_libChipOsal_a_HEADERS                          = \
    @top_builddir@/src/include/chip/osal.h            \
    @top_builddir@/src/osal/include/Ring.h            \
    @top_builddir@/src/osal/tests/test_util.h         \
    $(NULL)

libChipOsal_a_CPPFLAGS                              = \
    -I$(top_srcdir)/src/include                       \
    -I$(top_srcdir)/src/osal/include                  \
    $(NULL)

libChipOsal_a_SOURCES                              = $(NULL)

SUBDIRS                                            += tests

libChipOsal_a_CPPFLAGS                             += \
    -D_GNU_SOURCE                                     \
    -I$(top_srcdir)/src/osal/posix                    \
    $(NULL)

libChipOsal_a_SOURCES                              += \
    posix/os_callout.c                                \
    posix/os_eventq.cc                                \
    posix/os_queue.cc                                 \
    posix/os_mutex.c                                  \
    posix/os_sem.c                                    \
    posix/os_task.c                                   \
    posix/os_time.c                                   \
    posix/os_timer.c                                  \
    $(NULL)

dist_libChipOsal_a_HEADERS                         += \
    @top_builddir@/src/osal/posix/chip/os_types.h     \
    @top_builddir@/src/osal/posix/chip/os_port.h      \
    @top_builddir@/src/osal/posix/RingPthread.h       \
    $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

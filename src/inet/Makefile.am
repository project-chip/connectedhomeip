#
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
#      This file is the GNU automake template for the Nest InetLayer
#      library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

include InetLayer.am

EXTRA_DIST                          = \
    arpa-inet-compatibility.h         \
    $(NULL)

lib_LIBRARIES                       = libInetLayer.a

libInetLayer_a_CPPFLAGS             = \
    -I$(top_srcdir)/src/include       \
    $(nl_InetLayer_CPPFLAGS)          \
    $(LWIP_CPPFLAGS)                  \
    $(SOCKETS_CPPFLAGS)               \
    $(NULL)

libInetLayer_a_SOURCES              = $(nl_InetLayer_sources)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

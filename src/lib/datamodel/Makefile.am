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
#      This file is the GNU automake template for the CHIP system layer
#      library.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

include DataModel.am

SUBDIRS                             = \
    tests                             \
    $(NULL)

EXTRA_DIST                          = \
    $(NULL)

lib_LIBRARIES                       = libDataModel.a

libDataModel_a_CPPFLAGS             = \
    -I$(top_srcdir)/src               \
    -I$(top_srcdir)/src/include       \
    -I$(top_srcdir)/src/lib           \
    $(NULL)

libDataModel_a_SOURCES          = $(CHIP_BUILD_DATAMODEL_SOURCE_FILES)

libDataModel_adir               = $(includedir)/datamodel

dist_libDataModel_a_HEADERS     = $(CHIP_BUILD_DATAMODEL_HEADER_FILES)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

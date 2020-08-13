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
#      This file governs what comes out of the app directory, chiefly
#        the data model and application layer for the CHIP interaction
#        model.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

# Package (e.g. 'make dist') these subdirectories

DIST_SUBDIRS = \
    chip-zcl-zpro \
    clusters \
    util \
    $(NULL)

# Build (e.g for 'make all') these subdirectories

SUBDIRS                               = \
    chip-zcl-zpro                       \
    $(NULL)

include DataModel.am

lib_LIBRARIES                         = libCHIPDataModel.a

libCHIPDataModel_a_CPPFLAGS           = \
    -I$(top_srcdir)/src                 \
    -I$(top_srcdir)/src/lib             \
    $(NULL)

libCHIPDataModel_a_SOURCES            = \
    $(CHIP_BUILD_DATA_MODEL_SOURCE_FILES) \
    $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am

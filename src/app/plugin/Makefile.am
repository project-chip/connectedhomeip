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

SUBDIRS                               = \
    tests                               \
    $(NULL)

# this tree isn't included in a library, files are pulled into applications
# so: just distribute (maybe later install ;) )

plugins_EXTRA_DIST_ams = $(wildcard @top_srcdir@/src/app/plugin/*/extra_dist.am)

EXTRA_DIST=$(plugins_EXTRA_DIST_ams)

-include $(plugins_EXTRA_DIST_ams)


include $(abs_top_nlbuild_autotools_dir)/automake/post.am

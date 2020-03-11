#
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
#      This file is the CMake template for defining global build options for the CHIP SDK.
#

option(CHIP_BUILD_EXECUTABLES "Build executables" ON)

#
# This option chooses whether to optimize builds of third_party packages:
#
#   ON: If available, use optimized cmake scripts specified by the CHIP project.
#       This option will result in faster builds, but require updates when upstream
#       diverges. The optimization is applied for libs that don't change often and
#       can provide a measurable benefit from recasting the build using cmake.
#
#   OFF: Use the native build system provided the third_party project (autotools or other).
#
option(BUILD_EXTERNALS_WITH_CMAKE
    "Builds third_party packages with optimized cmake. Otherwise, use the native build system provided by the package."
    ON
)

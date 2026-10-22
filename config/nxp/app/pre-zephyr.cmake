#
#   Copyright (c) 2024 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

# Workaround: the nxp_iot_agent module has an if() with an unquoted
# $ENV{CONFIG_EL2GO_SIGN_USING_NXPIMAGE} that CMake >= 4.4.0 rejects when unset
# ("Unknown arguments specified"). Seed a value before find_package(Zephyr) so
# the token is non-empty; the guarded path stays disabled. Remove once fixed upstream.
if(NOT DEFINED ENV{CONFIG_EL2GO_SIGN_USING_NXPIMAGE})
    set(ENV{CONFIG_EL2GO_SIGN_USING_NXPIMAGE} "n")
endif()

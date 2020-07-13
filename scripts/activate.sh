#
# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
if [[ ! -d ${CHIP_ROOT} ]]; then
    if [[ -n ${BASH_SOURCE[0]} ]]; then
        CHIP_ROOT=$(cd "${BASH_SOURCE[0]%/*}/.." && pwd)
    else
        CHIP_ROOT=$(cd "${0%/*}/.." && pwd)
    fi
fi

export PW_BRANDING_BANNER="$CHIP_ROOT/.chip-banner.txt"
export PW_BRANDING_BANNER_COLOR="bold_white"

# shellcheck source=/dev/null
source "$CHIP_ROOT/third_party/pigweed/repo/activate.sh"

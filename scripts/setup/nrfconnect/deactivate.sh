#!/usr/bin/env bash
#
# Copyright (c) 2026 Project CHIP Authors
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

# Restore the shell environment saved before nrfconnect/activate.sh was sourced.

_NRFCONNECT_SETUP_DIR="$(cd "${BASH_SOURCE[0]%/*}" >/dev/null && pwd)"
_CHIP_ROOT="$(cd "${_NRFCONNECT_SETUP_DIR}/../../.." >/dev/null && pwd)"
_ENV_STATE_FILE="${_CHIP_ROOT}/.environment/nrfconnect/env_state.env"
_TOOLCHAINS_DIR="${_CHIP_ROOT}/.environment/nrfconnect/sdk/toolchains"

_remove_nrfutil_locks() {
    local lock_file
    if [ ! -d "${_TOOLCHAINS_DIR}" ]; then
        return 0
    fi

    while IFS= read -r lock_file; do
        rm -f "${lock_file}"
    done < <(find "${_TOOLCHAINS_DIR}" -path '*/nrfutil/home/locked' -type f 2>/dev/null)
}

_has_nrfutil_locks() {
    if [ ! -d "${_TOOLCHAINS_DIR}" ]; then
        return 1
    fi

    find "${_TOOLCHAINS_DIR}" -path '*/nrfutil/home/locked' -type f -print -quit 2>/dev/null | grep -q .
}

if [ ! -f "${_ENV_STATE_FILE}" ]; then
    if _has_nrfutil_locks; then
        _remove_nrfutil_locks
        echo "Removed stale nRF Connect environment lock."
        return 0 2>/dev/null || exit 0
    fi

    echo "No saved nRF Connect environment state found." >&2
    return 1 2>/dev/null || exit 1
fi

while IFS= read -r line || [ -n "${line}" ]; do
    if [ -n "${line}" ]; then
        eval "${line}"
    fi
done < "${_ENV_STATE_FILE}"

rm -f "${_ENV_STATE_FILE}"
_remove_nrfutil_locks
echo "nRF Connect environment deactivated."

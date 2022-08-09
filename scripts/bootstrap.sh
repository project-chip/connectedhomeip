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

_bootstrap_or_activate() {
    if [ -n "$BASH" ]; then
        local _BOOTSTRAP_PATH="${BASH_SOURCE[0]}"
    else
        local _BOOTSTRAP_PATH="$1"
    fi

    local _BOOTSTRAP_NAME="${_BOOTSTRAP_PATH##*/}"
    local _CHIP_ROOT="$(cd "${_BOOTSTRAP_PATH%/*}/.." && pwd)"

    local _CONFIG_FILE="scripts/environment.json"

    if [ ! -z "$PW_CONFIG_FILE" ]; then
        _CONFIG_FILE="$PW_CONFIG_FILE"
    fi

    if [ "$_BOOTSTRAP_NAME" = "bootstrap.sh" ] ||
        [ ! -f "$_CHIP_ROOT/third_party/pigweed/repo/pw_env_setup/util.sh" ]; then
        git submodule update --init
    fi

    PW_BRANDING_BANNER="$_CHIP_ROOT/scripts/matter_banner.txt"
    export PW_BRANDING_BANNER

    PW_PROJECT_ROOT="$_CHIP_ROOT"
    export PW_PROJECT_ROOT

    PW_ROOT="$_CHIP_ROOT/third_party/pigweed/repo"
    export PW_ROOT

    . "$_CHIP_ROOT/third_party/pigweed/repo/pw_env_setup/util.sh"

    _chip_bootstrap_banner() {
        if [ -z "$PW_ENVSETUP_QUIET" ] && [ -z "$PW_ENVSETUP_NO_BANNER" ]; then
            cat "$PW_BRANDING_BANNER"
            echo
        fi
    }

    local _PW_BANNER_FUNC="_chip_bootstrap_banner"

    export _PW_ACTUAL_ENVIRONMENT_ROOT="$(pw_get_env_root)"
    local _SETUP_SH="$_PW_ACTUAL_ENVIRONMENT_ROOT/activate.sh"

    export PW_DOCTOR_SKIP_CIPD_CHECKS=1
    export PATH # https://bugs.chromium.org/p/pigweed/issues/detail?id=281

    if [ "$_BOOTSTRAP_NAME" = "bootstrap.sh" ] ||
        [ ! -f "$_SETUP_SH" ] ||
        [ ! -s "$_SETUP_SH" ]; then
        pw_bootstrap --shell-file "$_SETUP_SH" \
            --install-dir "$_PW_ACTUAL_ENVIRONMENT_ROOT" \
            --config-file "$_CHIP_ROOT/$_CONFIG_FILE" \
            --virtualenv-gn-out-dir "$_PW_ACTUAL_ENVIRONMENT_ROOT/gn_out"
        pw_finalize bootstrap "$_SETUP_SH"
    else
        pw_activate
        pw_finalize activate "$_SETUP_SH"
    fi
}

_bootstrap_or_activate "$0"
unset -f _bootstrap_or_activate

pw_cleanup

unset PW_CIPD_INSTALL_DIR
unset CIPD_CACHE_DIR
unset _PW_BANNER_FUNC
unset _PW_TEXT
unset PW_DOCTOR_SKIP_CIPD_CHECKS

unset -f _chip_bootstrap_banner

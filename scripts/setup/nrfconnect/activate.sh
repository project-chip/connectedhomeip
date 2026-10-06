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

# Activate the nRF Connect SDK and Matter build environments for development.

_NRFCONNECT_SETUP_DIR="$(cd "${BASH_SOURCE[0]%/*}" >/dev/null && pwd)"
_CHIP_ROOT="$(cd "${_NRFCONNECT_SETUP_DIR}/../../.." >/dev/null && pwd)"
_ENV_STATE_FILE="${_CHIP_ROOT}/.environment/nrfconnect/env_state.env"

_NRF_ENV_VARS=(
    PATH
    GIT_EXEC_PATH
    GIT_TEMPLATE_DIR
    PERL5LIB
    PERLLIB
    TCL_LIBRARY
    TK_LIBRARY
    NRFUTIL_HOME
    ZEPHYR_TOOLCHAIN_VARIANT
    ZEPHYR_SDK_INSTALL_DIR
    ZEPHYR_BASE
    PYTHONHOME
    PYTHONPATH
)

_os="$(uname -s 2>/dev/null || echo unknown)"
case "${_os}" in
    Darwin)
        _NRF_ENV_VARS+=(DYLD_LIBRARY_PATH DYLD_FALLBACK_LIBRARY_PATH)
        ;;
    Linux)
        _NRF_ENV_VARS+=(LD_LIBRARY_PATH)
        ;;
esac

_save_env_state() {
    mkdir -p "$(dirname "${_ENV_STATE_FILE}")"
    : > "${_ENV_STATE_FILE}"

    for var in "${_NRF_ENV_VARS[@]}"; do
        if [ -n "${!var+x}" ]; then
            printf 'export %s=%q\n' "${var}" "${!var}" >> "${_ENV_STATE_FILE}"
        else
            printf 'unset %s\n' "${var}" >> "${_ENV_STATE_FILE}"
        fi
    done
}

_resolve_zephyr_base() {
    if [ -n "${ZEPHYR_BASE}" ] && [ -d "${ZEPHYR_BASE}" ]; then
        return 0
    fi

    local zephyrrc
    zephyrrc="$(find "${_CHIP_ROOT}/.environment/nrfconnect/sdk" -maxdepth 2 -name .zephyrrc 2>/dev/null | head -n 1)"
    if [ -z "${zephyrrc}" ]; then
        echo "ZEPHYR_BASE is not set and .zephyrrc was not found under .environment/nrfconnect/sdk." >&2
        echo "Run python3 scripts/setup/nrfconnect/setup.py first." >&2
        return 1
    fi

    export ZEPHYR_BASE="$(cd "$(dirname "${zephyrrc}")/zephyr" >/dev/null && pwd)"
}

if [ -f "${_ENV_STATE_FILE}" ] && [ -z "${NRFCONNECT_SKIP_ENV_STATE:-}" ]; then
    echo "nRF Connect environment appears to be active." >&2
    echo "Deactivate it first with:" >&2
    echo "  source \"${_NRFCONNECT_SETUP_DIR}/deactivate.sh\"" >&2
    return 1 2>/dev/null || exit 1
fi

if [ -z "${NRFCONNECT_SKIP_ENV_STATE:-}" ]; then
    _save_env_state
fi

if ! _resolve_zephyr_base; then
    return 1 2>/dev/null || exit 1
fi

if [ -f "${ZEPHYR_BASE}/../.zephyrrc" ]; then
    # shellcheck disable=SC1091
    source "${ZEPHYR_BASE}/../.zephyrrc"
else
    echo "Missing ${ZEPHYR_BASE}/../.zephyrrc. Run python3 scripts/setup/nrfconnect/setup.py." >&2
    return 1 2>/dev/null || exit 1
fi

export ZEPHYR_BASE

source "${_NRFCONNECT_SETUP_DIR}/../../activate.sh" "$@"

_apply_matter_python_env() {
    if [ -z "${VIRTUAL_ENV:-}" ]; then
        unset PYTHONHOME
        unset PYTHONPATH
        return
    fi

    # Match scripts/activate.sh: use the pigweed venv and do not keep the NCS
    # toolchain Python configuration from .zephyrrc.
    unset PYTHONHOME

    if [ -d "${VIRTUAL_ENV}/Lib/site-packages" ]; then
        export PYTHONPATH="${VIRTUAL_ENV}/Lib/site-packages"
        return
    fi

    local python_version
    python_version="$(python3 -c 'import sys; print(f"{sys.version_info.major}.{sys.version_info.minor}")')"
    export PYTHONPATH="${VIRTUAL_ENV}/lib/python${python_version}/site-packages"
}

_apply_matter_python_env

if [ -z "${NRFCONNECT_SKIP_ENV_STATE:-}" ]; then
    echo
    echo "To deactivate, run:"
    echo "  source \"${_NRFCONNECT_SETUP_DIR}/deactivate.sh\""
fi

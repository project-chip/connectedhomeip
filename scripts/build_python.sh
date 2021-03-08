#!/usr/bin/env bash

#
# Copyright (c) 2021 Project CHIP Authors
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

set -e

_normpath() {
    python -c "import os.path; print(os.path.normpath('$@'))"
}

_ensure_darwin_deployment_target() {
    if [[ `uname` != "Darwin" ]]; then
        return
    fi

    # Only override the deployment target if not set
    if [[ -n "${MACOSX_DEPLOYMENT_TARGET}" ]]; then
        return
    fi

    # This matches the deployment target for the current python binary.
    # platform.platform is of the form 'macOS-10.16-x86_64-i386-64bit'
    export MACOSX_DEPLOYMENT_TARGET="$(python -c "import platform; print(platform.platform().split('-')[1])")"
}

echo_green() {
    echo -e "\033[0;32m$*\033[0m"
}

echo_blue() {
    echo -e "\033[1;34m$*\033[0m"
}

echo_bold_white() {
    echo -e "\033[1;37m$*\033[0m"
}

CHIP_ROOT=$(_normpath "$(dirname "$0")/..")
OUTPUT_ROOT="$CHIP_ROOT/out/python_lib"
ENVIRONMENT_ROOT="$CHIP_ROOT/out/python_env"

# Ensure we have a compilation environment
source "$CHIP_ROOT/scripts/activate.sh"

# Deployment target set only post activate, to ensure the right
# python binary is used.
_ensure_darwin_deployment_target

# Generates ninja files
gn --root="$CHIP_ROOT" gen "$OUTPUT_ROOT"

# Compiles python files
ninja -C "$OUTPUT_ROOT" python

# Create a virtual environment that has access to the built python tools
virtualenv --clear "$ENVIRONMENT_ROOT"

# Activate the new enviroment to register the python WHL
source "$ENVIRONMENT_ROOT"/bin/activate
"$ENVIRONMENT_ROOT"/bin/python -m pip install --upgrade pip
"$ENVIRONMENT_ROOT"/bin/pip install "$OUTPUT_ROOT"/controller/python/chip-*.whl

echo ""
echo_green "Compilation completed and WHL package installed in: "
echo_blue "  $ENVIRONMENT_ROOT"
echo ""
echo_green "To use please run:"
echo_bold_white "  source $ENVIRONMENT_ROOT/bin/activate"

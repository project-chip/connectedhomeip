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

declare chip_detail_logging=false
declare enable_pybindings=false
declare chip_mdns

help() {

    echo "Usage: $file_name [ options ... ] [ -chip_detail_logging ChipDetailLoggingValue  ] [ -chip_mdns ChipMDNSValue  ] [-enable_pybindings EnableValue]"

    echo "General Options:
  -h, --help                Display this information.
Input Options:
  -d, --chip_detail_logging ChipDetailLoggingValue          Specify ChipDetailLoggingValue as true or false.
                                                            By default it is false.
  -m, --chip_mdns           ChipMDNSValue                   Specify ChipMDNSValue as platform or minimal.
                                                            By default it is minimal.
  -p, --enable_pybindings   EnableValue                     Specify whether to enable pybindings as python controller.
"
}

file_name=${0##*/}

while (($#)); do
    case $1 in
        --help | -h)
            help
            exit 1
            ;;
        --chip_detail_logging | -d)
            chip_detail_logging=$2
            shift
            ;;
        --chip_mdns | -m)
            chip_mdns=$2
            shift
            ;;
        --enable_pybindings | -p)
            enable_pybindings=$2
            shift
            ;;
        -*)
            help
            echo "Unknown Option \"$1\""
            exit 1
            ;;
    esac
    shift
done

# Print input values
echo "Input values: chip_detail_logging = $chip_detail_logging , chip_mdns = \"$chip_mdns\", enable_pybindings = $enable_pybindings"

# Ensure we have a compilation environment
source "$CHIP_ROOT/scripts/activate.sh"

# Generates ninja files
[[ -n "$chip_mdns" ]] && chip_mdns_arg="chip_mdns=\"$chip_mdns\"" || chip_mdns_arg=""

chip_data_model_arg="chip_data_model=\"///examples/lighting-app/lighting-common\""

gn --root="$CHIP_ROOT" gen "$OUTPUT_ROOT" --args="chip_detail_logging=$chip_detail_logging enable_pylib=$enable_pybindings enable_rtti=$enable_pybindings $chip_mdns_arg chip_controller=false $chip_data_model_arg"

# Compiles python files
# Check pybindings was requested
if [ "$enable_pybindings" == true ]; then
    ninja -v -C "$OUTPUT_ROOT" pycontroller
else
    ninja -v -C "$OUTPUT_ROOT" chip-core
fi

# Create a virtual environment that has access to the built python tools
virtualenv --clear "$ENVIRONMENT_ROOT"

# Activate the new environment to register the python WHL

if [ "$enable_pybindings" == true ]; then
    WHEEL=("$OUTPUT_ROOT"/pybindings/pycontroller/pychip-*.whl)
else
    WHEEL=("$OUTPUT_ROOT"/controller/python/chip_core*.whl)
fi

source "$ENVIRONMENT_ROOT"/bin/activate
"$ENVIRONMENT_ROOT"/bin/python -m pip install --upgrade pip
"$ENVIRONMENT_ROOT"/bin/pip install --upgrade --force-reinstall --no-cache-dir "${WHEEL[@]}"

echo ""
echo_green "Compilation completed and WHL package installed in: "
echo_blue "  $ENVIRONMENT_ROOT"
echo ""
echo_green "To use please run:"
echo_bold_white "  source $ENVIRONMENT_ROOT/bin/activate"

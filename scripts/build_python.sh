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

declare enable_ble=true
declare chip_detail_logging=false
declare chip_mdns
declare case_retry_delta
declare install_virtual_env
declare clean_virtual_env=yes
declare install_pytest_requirements=yes
declare install_jupyterlab=no

help() {

    echo "Usage: $file_name [ options ... ] [ -chip_detail_logging ChipDetailLoggingValue  ] [ -chip_mdns ChipMDNSValue  ]"

    echo "General Options:
  -h, --help                Display this information.
Input Options:
  -b, --enable_ble          <true/false>                    Enable BLE in the controller (default=true)
  -d, --chip_detail_logging <true/false>                    Specify ChipDetailLoggingValue as true or false.
                                                            By default it is false.
  -m, --chip_mdns           ChipMDNSValue                   Specify ChipMDNSValue as platform or minimal.
                                                            By default it is minimal.
  -t --time_between_case_retries MRPActiveRetryInterval     Specify MRPActiveRetryInterval value
                                                            Default is 300 ms
  -i, --install_virtual_env <path>                          Create a virtual environment with the wheels installed
                                                            <path> represents where the virtual environment is to be created.
  -c, --clean_virtual_env  <yes|no>                         When installing a virtual environment, create/clean it first.
                                                            Defaults to yes.
  --include_pytest_deps  <yes|no>                           Install requirements.txt for running scripts/tests and
                                                            src/python_testing scripts.
                                                            Defaults to yes.
  -j, --jupyter-lab                                         Install jupyterlab requirements.
  --extra_packages PACKAGES                                 Install extra Python packages from PyPI
  -z --pregen_dir DIRECTORY                                 Directory where generated zap files have been pre-generated.
"
}

file_name=${0##*/}

while (($#)); do
    case $1 in
        --help | -h)
            help
            exit 1
            ;;
        --enable_ble | -b)
            enable_ble=$2
            if [[ "$enable_ble" != "true" && "$enable_ble" != "false" ]]; then
                echo "chip_detail_logging should have a true/false value, not '$enable_ble'"
                exit
            fi
            shift
            ;;
        --chip_detail_logging | -d)
            chip_detail_logging=$2
            if [[ "$chip_detail_logging" != "true" && "$chip_detail_logging" != "false" ]]; then
                echo "chip_detail_logging should have a true/false value, not '$chip_detail_logging'"
                exit
            fi
            shift
            ;;
        --chip_mdns | -m)
            chip_mdns=$2
            shift
            ;;
        --time_between_case_retries | -t)
            chip_case_retry_delta=$2
            shift
            ;;
        --install_virtual_env | -i)
            install_virtual_env=$2
            shift
            ;;
        --clean_virtual_env | -c)
            clean_virtual_env=$2
            if [[ "$clean_virtual_env" != "yes" && "$clean_virtual_env" != "no" ]]; then
                echo "clean_virtual_env should have a yes/no value, not '$clean_virtual_env'"
                exit
            fi
            shift
            ;;
        --include_pytest_deps)
            install_pytest_requirements=$2
            if [[ "$install_pytest_requirements" != "yes" && "$install_pytest_requirements" != "no" ]]; then
                echo "install_pytest_requirements should have a yes/no value, not '$install_pytest_requirements'"
                exit
            fi
            shift
            ;;
        --extra_packages)
            extra_packages=$2
            shift
            ;;
        --pregen_dir | -z)
            pregen_dir=$2
            shift
            ;;
        --jupyter-lab | -j)
            install_jupyterlab=yes
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
echo "Input values: chip_detail_logging = $chip_detail_logging , chip_mdns = \"$chip_mdns\", chip_case_retry_delta=\"$chip_case_retry_delta\", pregen_dir=\"$pregen_dir\", enable_ble=\"$enable_ble\""

# Ensure we have a compilation environment
source "$CHIP_ROOT/scripts/activate.sh"

# This is to prevent python compiled for previous versions reporting 10.16 as a version
# which breaks the ability to install python wheels.
#
# See https://eclecticlight.co/2020/08/13/macos-version-numbering-isnt-so-simple/ for
# some explanation
#
# TLDR:
#
#   > import platform
#   > print(platform.mac_ver()[0])
#     11.7.3   // (example) if SYSTEM_VERSION_COMPAT is 0
#     10.16    // SYSTEM_VERSION_COMPAT is unset or 1
export SYSTEM_VERSION_COMPAT=0

# Generates ninja files
[[ -n "$chip_mdns" ]] && chip_mdns_arg="chip_mdns=\"$chip_mdns\"" || chip_mdns_arg=""
[[ -n "$chip_case_retry_delta" ]] && chip_case_retry_arg="chip_case_retry_delta=$chip_case_retry_delta" || chip_case_retry_arg=""
[[ -n "$pregen_dir" ]] && pregen_dir_arg="chip_code_pre_generated_directory=\"$pregen_dir\"" || pregen_dir_arg=""

# Make all possible human redable tracing available.
tracing_options="matter_log_json_payload_hex=true matter_log_json_payload_decode_full=true matter_enable_tracing_support=true"

gn --root="$CHIP_ROOT" gen "$OUTPUT_ROOT" --args="$tracing_options chip_detail_logging=$chip_detail_logging chip_project_config_include_dirs=[\"//config/python\"] $chip_mdns_arg $chip_case_retry_arg $pregen_dir_arg chip_config_network_layer_ble=$enable_ble chip_enable_ble=$enable_ble chip_crypto=\"boringssl\""

function ninja_target() {
    # Print the ninja target required to build a gn label.
    local GN_LABEL="$1"
    local NINJA_TARGET="$(gn ls "$OUTPUT_ROOT" --as=output "$GN_LABEL")"
    echo "$NINJA_TARGET"
}

function wheel_output_dir() {
    # Print the wheel output directory for a pw_python_package or
    # pw_python_distribution. The label must end in "._build_wheel".
    local GN_LABEL="$1"
    local NINJA_TARGET="$(ninja_target "$GN_LABEL")"
    local WHEEL_DIR="$OUTPUT_ROOT"/"$(dirname "$NINJA_TARGET")/$(basename -s .stamp "$NINJA_TARGET")"
    echo "$WHEEL_DIR"
}

# Compile Python wheels
ninja -C "$OUTPUT_ROOT" python_wheels

# Add wheels from chip_python_wheel_action templates.
WHEEL=("$OUTPUT_ROOT"/controller/python/chip*.whl)

# Add the matter_testing_infrastructure wheel
WHEEL+=("$OUTPUT_ROOT"/python/obj/src/python_testing/matter_testing_infrastructure/chip-testing._build_wheel/chip_testing-*.whl)

if [ -n "$extra_packages" ]; then
    WHEEL+=("$extra_packages")
fi

if [ -n "$install_virtual_env" ]; then
    ENVIRONMENT_ROOT="$install_virtual_env"

    if [ "$clean_virtual_env" = "yes" ]; then
        # Create a virtual environment that has access to the built python tools
        echo_blue "Creating a clear VirtualEnv in '$ENVIRONMENT_ROOT' ..."
        python -m venv --clear "$ENVIRONMENT_ROOT"
    elif [ ! -f "$ENVIRONMENT_ROOT"/bin/activate ]; then
        echo_blue "Creating a new VirtualEnv in '$ENVIRONMENT_ROOT' ..."
        python -m venv "$ENVIRONMENT_ROOT"
    fi

    source "$ENVIRONMENT_ROOT"/bin/activate
    "$ENVIRONMENT_ROOT"/bin/python -m ensurepip --upgrade
    "$ENVIRONMENT_ROOT"/bin/python -m pip install --upgrade "${WHEEL[@]}"

    if [ "$install_pytest_requirements" = "yes" ]; then
        YAMLTESTS_GN_LABEL="//scripts:matter_yamltests_distribution._build_wheel"
        # Add wheels from pw_python_package or pw_python_distribution templates.
        YAMLTEST_WHEEL=(
            "$(ls -tr "$(wheel_output_dir "$YAMLTESTS_GN_LABEL")"/*.whl | head -n 1)"
        )

        echo_blue "Installing python test dependencies ..."
        "$ENVIRONMENT_ROOT"/bin/pip install --upgrade "${YAMLTEST_WHEEL[@]}"
        "$ENVIRONMENT_ROOT"/bin/pip install -r "$CHIP_ROOT/scripts/tests/requirements.txt"
        "$ENVIRONMENT_ROOT"/bin/pip install -r "$CHIP_ROOT/src/python_testing/requirements.txt"
    fi

    if [ "$install_jupyterlab" = "yes" ]; then
        echo_blue "Installing JupyterLab kernels and lsp..."
        "$ENVIRONMENT_ROOT"/bin/pip install -r "$CHIP_ROOT/scripts/jupyterlab_requirements.txt"
    fi

    echo ""
    echo_green "Compilation completed and WHL package installed in: "
    echo_blue "  $ENVIRONMENT_ROOT"
    echo ""
    echo_green "To use please run:"
    echo_bold_white "  source $ENVIRONMENT_ROOT/bin/activate"

    if [ "$install_jupyterlab" = "yes" ]; then
        echo_bold_white "  jupyter-lab"
    fi
fi

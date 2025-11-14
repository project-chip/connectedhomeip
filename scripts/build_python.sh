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

declare enable_ble=true
declare enable_nfc=false
declare enable_ipv4=true
declare wifi_paf_config=""
declare chip_detail_logging=false
declare chip_mdns=minimal
declare chip_case_retry_delta
declare install_virtual_env
declare output_root="$CHIP_ROOT/out/python_lib"
declare clean_virtual_env=yes
declare install_pytest_deps=yes
declare install_jupyterlab=no
declare -a extra_packages
declare -a extra_gn_args
declare chip_build_controller_dynamic_server=true
declare enable_pw_rpc=false
declare enable_ccache=no
declare enable_webrtc=true

help() {

    echo "Usage: $file_name [ options ... ]"

    echo "General Options:
  -h, --help                Display this information.
Input Options:
  -g, --gn_args ARGS                                        Additional verbatim arguments to pass to the gn command.
                                                            May be specified multiple times.
  -b, --enable_ble          <true/false>                    Enable BLE in the controller (default=$enable_ble)
  -p, --enable_wifi_paf     <true/false>                    Enable Wi-Fi PAF discovery in the controller (default=SDK default behavior)
  -4, --enable_ipv4         <true/false>                    Enable IPv4 in the controller (default=$enable_ipv4)
  -d, --chip_detail_logging <true/false>                    Specify ChipDetailLoggingValue as true or false.
                                                            By default it is $chip_detail_logging.
  -m, --chip_mdns           ChipMDNSValue                   Specify ChipMDNSValue as platform or minimal.
                                                            By default it is $chip_mdns.
  -n, --enable_nfc          <true/false>                    Enable NFC in the controller (default=$enable_nfc)
  -w, --enable_webrtc       <true/false>                    Enable WebRTC support in the controller (default=$enable_webrtc)
  -t --time_between_case_retries MRPActiveRetryInterval     Specify MRPActiveRetryInterval value
                                                            Default is 300 ms
  -O, --output_root         <path>                          Path for the generated bindings (default=$output_root)
  -i, --install_virtual_env <path>                          Create a virtual environment with the wheels installed
                                                            <path> represents where the virtual environment is to be created.
  -c, --clean_virtual_env  <yes|no>                         When installing a virtual environment, create/clean it first.
                                                            Defaults to $clean_virtual_env.
  --include_pytest_deps  <yes|no>                           Install requirements.txt for running scripts/tests and
                                                            src/python_testing scripts.
                                                            Defaults to $install_pytest_deps.
  -j, --jupyter-lab                                         Install jupyterlab requirements.
  -E, --extra_packages PACKAGE                              Install extra Python packages from PyPI.
                                                            May be specified multiple times.
  -z, --pregen_dir DIRECTORY                                Directory where generated zap files have been pre-generated.
  -ds, --chip_build_controller_dynamic_server <true/false>  Enable dynamic server in controller.
                                                            Defaults to $chip_build_controller_dynamic_server.
  -pw  --enable_pw_rpc <true/false>                         Build Pw Python wheels. Defaults to $enable_pw_rpc.
  --enable-ccache                                           Use ccache for building python wheels. Defaults to $enable_ccache.
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
                echo "Error: --enable_ble/-b should have a true/false value, not '$enable_ble'" >&2
                exit 1
            fi
            shift
            ;;
        --enable_nfc | -n)
            enable_nfc=$2
            if [[ "$enable_nfc" != "true" && "$enable_nfc" != "false" ]]; then
                echo "Error: --enable_nfc/-n should have a true/false value, not '$enable_nfc'" >&2
                exit 1
            fi
            shift
            ;;
        --enable_wifi_paf | -p)
            declare wifi_paf_arg="$2"
            if [[ "$wifi_paf_arg" != "true" && "$wifi_paf_arg" != "false" ]]; then
                echo "Error: --enable_wifi_paf/-p should have a true/false value, not '$wifi_paf_arg'" >&2
                exit 1
            fi
            wifi_paf_config="chip_device_config_enable_wifipaf=$wifi_paf_arg"
            shift
            ;;
        --enable_ipv4 | -4)
            enable_ipv4=$2
            if [[ "$enable_ipv4" != "true" && "$enable_ipv4" != "false" ]]; then
                echo "Error: --enable_ipv4/-4 should have a true/false value, not '$enable_ipv4'" >&2
                exit 1
            fi
            shift
            ;;
        --enable_webrtc | -w)
            enable_webrtc=$2
            if [[ "$enable_webrtc" != "true" && "$enable_webrtc" != "false" ]]; then
                echo "Error: --enable_webrtc/-w should have a true/false value, not '$enable_webrtc'" >&2
                exit 1
            fi
            shift
            ;;
        --chip_detail_logging | -d)
            chip_detail_logging=$2
            if [[ "$chip_detail_logging" != "true" && "$chip_detail_logging" != "false" ]]; then
                echo "Error: --chip_detail_logging/-d should have a true/false value, not '$chip_detail_logging'" >&2
                exit 1
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
        --output_root | -O)
            output_root=$2
            shift
            ;;
        --install_virtual_env | -i)
            install_virtual_env=$2
            shift
            ;;
        --clean_virtual_env | -c)
            clean_virtual_env=$2
            if [[ "$clean_virtual_env" != "yes" && "$clean_virtual_env" != "no" ]]; then
                echo "Error: --clean_virtual_env/-c should have a yes/no value, not '$clean_virtual_env'" >&2
                exit 1
            fi
            shift
            ;;
        --include_pytest_deps)
            install_pytest_deps=$2
            if [[ "$install_pytest_deps" != "yes" && "$install_pytest_deps" != "no" ]]; then
                echo "Error: --include_pytest_deps should have a yes/no value, not '$install_pytest_deps'" >&2
                exit 1
            fi
            shift
            ;;
        --extra_packages | -E)
            extra_packages+=("$2")
            shift
            ;;
        --gn_args | -g)
            extra_gn_args+=("$2")
            shift
            ;;
        --pregen_dir | -z)
            pregen_dir=$2
            shift
            ;;
        --jupyter-lab | -j)
            install_jupyterlab=yes
            ;;
        --chip_build_controller_dynamic_server | -ds)
            chip_build_controller_dynamic_server=$2
            shift
            ;;
        --enable_pw_rpc | -pw)
            enable_pw_rpc=$2
            if [[ "$enable_pw_rpc" != "true" && "$enable_pw_rpc" != "false" ]]; then
                echo "Error: --enable_pw_rpc/-pw should have a true/false value, not '$enable_pw_rpc'" >&2
                exit 1
            fi
            shift
            ;;
        --enable-ccache)
            enable_ccache=yes
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
echo "Building Python environment with the following configuration:"
echo "  chip_detail_logging=\"$chip_detail_logging\""
echo "  chip_mdns=\"$chip_mdns\""
echo "  chip_case_retry_delta=\"$chip_case_retry_delta\""
echo "  pregen_dir=\"$pregen_dir\""
echo "  enable_ble=\"$enable_ble\""
echo "  enable_nfc=\"$enable_nfc\""
if [[ -n $wifi_paf_config ]]; then
    echo "  $wifi_paf_config"
fi
echo "  enable_ipv4=\"$enable_ipv4\""
echo "  chip_build_controller_dynamic_server=\"$chip_build_controller_dynamic_server\""
echo "  chip_support_webrtc_python_bindings=\"$enable_webrtc\""
echo "  enable_pw_rpc=\"$enable_pw_rpc\""
echo "  enable_ccache=\"$enable_ccache\""

if [[ ${#extra_gn_args[@]} -gt 0 ]]; then
    echo "In addition, the following extra args will added to gn command line: ${extra_gn_args[*]}"
fi

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

# Set default crypto to BoringSSL
chip_crypto="boringssl"

# Disable WebRTC by default only on Darwin due to libdatachannel limitations
if [[ "$(uname)" == "Darwin" ]]; then
    echo "Warning: WebRTC is not supported on Darwin. Disabling WebRTC to avoid build errors."
    enable_webrtc="false"
fi

# If WebRTC is enabled, switch chip_crypto to OpenSSL,
# because WebRTC depends on OpenSSL, which must be installed and available.
if [[ "$enable_webrtc" == "true" ]]; then
    chip_crypto="openssl"
fi

# Generates ninja files
gn_args=(
    # Make all possible human readable tracing available.
    "matter_log_json_payload_hex=true"
    "matter_log_json_payload_decode_full=true"
    "matter_enable_tracing_support=true"
    # Setup selected configuration.
    "chip_detail_logging=$chip_detail_logging"
    "chip_project_config_include_dirs=[\"//config/python\"]"
    "chip_config_network_layer_ble=$enable_ble"
    "chip_enable_ble=$enable_ble"
    "chip_enable_nfc_based_commissioning=$enable_nfc"
    "chip_inet_config_enable_ipv4=$enable_ipv4"
    "chip_crypto=\"$chip_crypto\""
    "chip_build_controller_dynamic_server=$chip_build_controller_dynamic_server"
    "chip_support_webrtc_python_bindings=$enable_webrtc"
    "chip_device_config_enable_joint_fabric=true"
)
# Add ccache support through pw_command_launcher when enabled
if [[ "$enable_ccache" == "yes" ]]; then
    gn_args+=("pw_command_launcher=\"ccache\"")
fi
if [[ -n "$chip_mdns" ]]; then
    gn_args+=("chip_mdns=\"$chip_mdns\"")
fi
if [[ -n "$chip_case_retry_delta" ]]; then
    gn_args+=("chip_case_retry_delta=$chip_case_retry_delta")
fi
if [[ -n "$pregen_dir" ]]; then
    gn_args+=("chip_code_pre_generated_directory=\"$pregen_dir\"")
fi
if [[ -n $wifi_paf_config ]]; then
    gn_args+=("$wifi_paf_config")
fi
# Append extra arguments provided by the user.
gn_args+=("${extra_gn_args[@]}")

gn --root="$CHIP_ROOT" gen "$output_root" --args="${gn_args[*]}"

# Set up ccache environment for compilation
if [[ "$enable_ccache" == "yes" ]]; then
    # Only wrap if not already wrapped with ccache
    if [[ -n "$CC" ]] && [[ "$CC" != ccache* ]]; then
        export CC="ccache $CC"
    fi
    if [[ -n "$CXX" ]] && [[ "$CXX" != ccache* ]]; then
        export CXX="ccache $CXX"
    fi
fi

# Compile Python wheels
ninja -C "$output_root" python_wheels

# Add wheels from matter_python_wheel_action templates.
WHEEL=("$output_root"/controller/python/matter*.whl)

# Add the matter_testing_infrastructure wheel
WHEEL+=("$output_root"/obj/src/python_testing/matter_testing_infrastructure/matter-testing._build_wheel/matter_testing*.whl)

if [ "$install_pytest_deps" = "yes" ]; then
    # Add wheels with YAML testing support.
    WHEEL+=(
        # Add matter-idl as well as matter-yamltests depends on it.
        "$output_root"/python/obj/scripts/py_matter_idl/matter-idl._build_wheel/matter_idl-*.whl
        "$output_root"/python/obj/scripts/py_matter_yamltests/matter-yamltests._build_wheel/matter_yamltests-*.whl
    )
fi

if [ -n "$extra_packages" ]; then
    WHEEL+=("${extra_packages[@]}")
fi

if [[ "$enable_pw_rpc" == "true" ]]; then
    echo "Installing Pw RPC Python wheels"
    PWRPC_ROOT="$CHIP_ROOT/examples/common/pigweed/rpc_console"
    PWRPC_OUTPUT_ROOT="$output_root/pwrpc"
    gn --root="$PWRPC_ROOT" gen "$PWRPC_OUTPUT_ROOT"
    # Compile Python wheels
    ninja -C "$PWRPC_OUTPUT_ROOT" chip_rpc_wheel
    WHEEL+=("$PWRPC_OUTPUT_ROOT"/chip_rpc_console_wheels/*.whl)
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

    "$ENVIRONMENT_ROOT"/bin/pip install -r "$CHIP_ROOT/scripts/setup/requirements.build.txt"

    if [ "$install_pytest_deps" = "yes" ]; then
        echo_blue "Installing python test dependencies ..."
        "$ENVIRONMENT_ROOT"/bin/pip install -r "$CHIP_ROOT/scripts/tests/requirements.txt"
        "$ENVIRONMENT_ROOT"/bin/pip install -r "$CHIP_ROOT/src/python_testing/requirements.txt"

        if [ "$enable_nfc" = "true" ]; then
            echo_blue "Installing python nfc dependencies ..."
            OS_TYPE="$(uname -s)"

            if [[ "$OS_TYPE" == "Linux" ]]; then

                # Only run dpkg check if dpkg exists (Debian/Ubuntu)
                if command -v dpkg >/dev/null 2>&1; then
                    if ! dpkg -s libpcsclite-dev >/dev/null 2>&1; then
                        echo "Error: The package 'libpcsclite-dev' is not installed."
                        echo "Please install it with: sudo apt-get install libpcsclite-dev"
                        exit 1
                    fi
                else
                    echo "Warning: Non-Debian Linux detected. Skipping dpkg check."
                    echo "Ensure PCSC development libraries are installed for your distro."
                fi
            fi
            
            "$ENVIRONMENT_ROOT"/bin/pip install -r "$CHIP_ROOT/src/python_testing/requirements.nfc.txt"
        fi

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

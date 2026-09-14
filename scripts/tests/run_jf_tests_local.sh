#!/usr/bin/env bash

# Copyright (c) 2024 Project CHIP Authors
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

# Local runner for Joint Fabric Python tests (TC_JFDS_*, TC_JF_*).
#
# Usage:
#   scripts/tests/run_jf_tests_local.sh [--test-filter PATTERN] [extra local.py args...]
#
# Examples:
#   scripts/tests/run_jf_tests_local.sh --test-filter TC_JFDS_2_3
#   scripts/tests/run_jf_tests_local.sh --test-filter "TC_JFDS_*"
#
# Prerequisites (see AGENTS.md "Joint Fabric Python Tests - Local"):
#   1. mDNS must work on the loopback interface. Pick one:
#        a) sudo apt install avahi-daemon && sudo systemctl start avahi-daemon
#        b) UTM: set VM NIC to Bridged mode
#        c) Rebuild venv: scripts/build_python.sh -i out/venv --enable_ipv4 true
#   2. Build the venv (if not already done):
#        scripts/build_python.sh -i out/venv --enable_ipv4 true
#   3. Build the JFA and JFC apps (if not already done):
#        scripts/run_in_build_env.sh \
#          "./scripts/build/build_examples.py --target linux-arm64-jf-admin-app-clang build"
#        scripts/run_in_build_env.sh \
#          "./scripts/build/build_examples.py --target linux-arm64-jf-control-app-clang build"

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CHIP_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
VENV="${CHIP_ROOT}/out/venv"
JFA_APP="${CHIP_ROOT}/out/linux-arm64-jf-admin-app-clang/jfa-app"
JFC_APP="${CHIP_ROOT}/out/linux-arm64-jf-control-app-clang/jfc-app"

if [[ ! -f "${VENV}/bin/activate" ]]; then
    echo "ERROR: Python venv not found at ${VENV}"
    echo "       Build it with: scripts/build_python.sh -i out/venv --enable_ipv4 true"
    exit 1
fi

if [[ ! -f "${JFA_APP}" ]]; then
    echo "ERROR: JFA app not found at ${JFA_APP}"
    echo "       Build: scripts/run_in_build_env.sh \"./scripts/build/build_examples.py --target linux-arm64-jf-admin-app-clang build\""
    exit 1
fi

if [[ ! -f "${JFC_APP}" ]]; then
    echo "ERROR: JFC app not found at ${JFC_APP}"
    echo "       Build: scripts/run_in_build_env.sh \"./scripts/build/build_examples.py --target linux-arm64-jf-control-app build\""
    exit 1
fi

# Sync the installed matter.testing package with the source tree so that
# run_python_test.py picks up any API additions not yet rebuilt into the venv.
MATTER_TESTING_SRC="${CHIP_ROOT}/src/python_testing/matter_testing_infrastructure/matter/testing"
MATTER_TESTING_VENV="${VENV}/lib/python3.12/site-packages/matter/testing"
if [[ -d "${MATTER_TESTING_SRC}" && -d "${MATTER_TESTING_VENV}" ]]; then
    cp -r "${MATTER_TESTING_SRC}/." "${MATTER_TESTING_VENV}/"
fi

source "${VENV}/bin/activate"

cd "${CHIP_ROOT}"
exec scripts/tests/local.py python-tests \
    --override-binary-path JF_ADMIN_APP "${JFA_APP}" \
    --override-binary-path JF_CONTROL_APP "${JFC_APP}" \
    "$@"

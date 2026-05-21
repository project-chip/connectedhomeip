"""
Copyright (c) 2026 Project CHIP Authors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

from pathlib import Path

CHIP_REPO = Path(__file__).resolve().parents[4]
CHIP_REPO_STR = str(CHIP_REPO)
CONTROLLER_TEST_SCRIPTS_DIR = CHIP_REPO / "src/controller/python/tests/scripts"
MATTER_DEVELOPMENT_PAA_ROOT_CERTS = CHIP_REPO / "credentials/development/paa-root-certs"

CHIP_ALL_CLUSTERS_APP = CHIP_REPO / "out/debug/standalone/chip-all-clusters-app"
CHIP_ECHO_REQUESTER = CHIP_REPO / "out/debug/linux_x64_gcc/chip-echo-requester"
CHIP_ECHO_RESPONDER = CHIP_REPO / "out/debug/linux_x64_gcc/chip-echo-responder"
CHIP_IM_INITIATOR = CHIP_REPO / "out/debug/linux_x64_gcc/chip-im-initiator"
CHIP_IM_RESPONDER = CHIP_REPO / "out/debug/linux_x64_gcc/chip-im-responder"
CHIP_LIGHTING_APP = CHIP_REPO / "out/debug/standalone/chip-lighting-app"
CHIP_TOOL = CHIP_REPO / "out/debug/standalone/chip-tool"
LIT_ICD_APP = CHIP_REPO / "out/debug/lit_icd/lit-icd-app"

MATTER_CONTROLLER_WHEEL_DIR = CHIP_REPO / "out/debug/linux_x64_gcc/controller/python"
MATTER_CONTROLLER_WHEELS = list(MATTER_CONTROLLER_WHEEL_DIR.glob("*.whl"))
MATTER_CONTROLLER_WHEELS_STR = " ".join(str(wheel) for wheel in MATTER_CONTROLLER_WHEELS)

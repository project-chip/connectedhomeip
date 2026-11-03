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

import shlex
from pathlib import Path

# _PATH is the pathlib.Path representation of the path.
# _STR are string representation of the path.
# _ESC are the escaped string representation of the path, which can be used in shell commands.

CHIP_REPO_PATH = next(filter(lambda p: (p / 'SPECIFICATION_VERSION').is_file(), Path(__file__).parents))
CHIP_REPO_STR = str(CHIP_REPO_PATH)
CONTROLLER_TEST_SCRIPTS_DIR_PATH = CHIP_REPO_PATH / "src/controller/python/tests/scripts"
MATTER_DEVELOPMENT_PAA_ROOT_CERTS_ESC = shlex.quote(str(CHIP_REPO_PATH / "credentials/development/paa-root-certs"))

CHIP_ALL_CLUSTERS_APP_FRAGMENT = "standalone/chip-all-clusters-app"
CHIP_ALL_CLUSTERS_APP_ESC = shlex.quote(str(CHIP_REPO_PATH / "out/debug" / CHIP_ALL_CLUSTERS_APP_FRAGMENT))
CHIP_ECHO_REQUESTER_ESC = shlex.quote(str(CHIP_REPO_PATH / "out/debug/linux_x64_gcc/chip-echo-requester"))
CHIP_ECHO_RESPONDER_ESC = shlex.quote(str(CHIP_REPO_PATH / "out/debug/linux_x64_gcc/chip-echo-responder"))
CHIP_IM_INITIATOR_ESC = shlex.quote(str(CHIP_REPO_PATH / "out/debug/linux_x64_gcc/chip-im-initiator"))
CHIP_IM_RESPONDER_ESC = shlex.quote(str(CHIP_REPO_PATH / "out/debug/linux_x64_gcc/chip-im-responder"))
CHIP_LIGHTING_APP_ESC = shlex.quote(str(CHIP_REPO_PATH / "out/debug/standalone/chip-lighting-app"))
CHIP_TOOL_ESC = shlex.quote(str(CHIP_REPO_PATH / "out/debug/standalone/chip-tool"))
LIT_ICD_APP_ESC = shlex.quote(str(CHIP_REPO_PATH / "out/debug/lit_icd/lit-icd-app"))

MATTER_CONTROLLER_WHEEL_DIR_PATH = CHIP_REPO_PATH / "out/debug/linux_x64_gcc/obj/src/controller/python/matter-controller-wheels"
MATTER_CONTROLLER_WHEELS = ["matter_clusters", "matter_core", "matter_repl"]
MATTER_CONTROLLER_INSTALL_WHEELS = (
    f"pip3 install --break-system-packages --find-links {shlex.quote(str(MATTER_CONTROLLER_WHEEL_DIR_PATH))} "
    f"{' '.join(MATTER_CONTROLLER_WHEELS)}")

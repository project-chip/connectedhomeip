#
#    Copyright (c) 2021 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import os
import subprocess
from pathlib import Path

from . import linux, runner
from .test_definition import ApplicationPaths, TestDefinition, TestTarget

_DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", ".."))
_YAML_TEST_SUITE_PATH = os.path.abspath(
    os.path.join(_DEFAULT_CHIP_ROOT, "src/app/tests/suites"))


def _FindYamlTestPath(name: str):
    yaml_test_suite_path = Path(_YAML_TEST_SUITE_PATH)
    if not yaml_test_suite_path.exists():
        raise FileNotFoundError(f"Expected directory {_YAML_TEST_SUITE_PATH} to exist")
    for path in yaml_test_suite_path.rglob(name):
        if not path.is_file():
            continue
        if path.name != name:
            continue
        return str(path)
    return None


def target_for_name(name: str):
    if name.startswith("TV_") or name.startswith("Test_TC_MC_") or name.startswith("Test_TC_LOWPOWER_") or name.startswith("Test_TC_KEYPADINPUT_") or name.startswith("Test_TC_APPLAUNCHER_") or name.startswith("Test_TC_MEDIAINPUT_") or name.startswith("Test_TC_WAKEONLAN_") or name.startswith("Test_TC_CHANNEL_") or name.startswith("Test_TC_MEDIAPLAYBACK_") or name.startswith("Test_TC_AUDIOOUTPUT_") or name.startswith("Test_TC_TGTNAV_") or name.startswith("Test_TC_APBSC_") or name.startswith("Test_TC_CONTENTLAUNCHER_") or name.startswith("Test_TC_ALOGIN_"):
        return TestTarget.TV
    if name.startswith("DL_") or name.startswith("Test_TC_DRLK_"):
        return TestTarget.LOCK
    if name.startswith("OTA_"):
        return TestTarget.OTA
    if name.startswith("Test_TC_BRBINFO_"):
        return TestTarget.BRIDGE
    return TestTarget.ALL_CLUSTERS


def tests_with_command(chip_tool: str, is_manual: bool):
    """Executes `chip_tool` binary to see what tests are available, using cmd
    to get the list.
    """
    cmd = "list"
    if is_manual:
        cmd += "-manual"

    result = subprocess.run([chip_tool, "tests", cmd], capture_output=True)

    for name in result.stdout.decode("utf8").split("\n"):
        if not name:
            continue

        target = target_for_name(name)

        yield TestDefinition(
            run_name=name, name=name, target=target, is_manual=is_manual
        )


# TODO We will move away from hardcoded list of yamltests to run all file when yamltests
# parser/runner reaches parity with the code gen version.
def _hardcoded_python_yaml_tests():
    currently_supported_yaml_tests = ["TestConstraints.yaml"]

    for name in currently_supported_yaml_tests:
        yaml_test_path = _FindYamlTestPath(name)
        if not yaml_test_path:
            raise FileNotFoundError(f"Could not find YAML test {name}")

        target = target_for_name(name)

        yield TestDefinition(
            run_name=yaml_test_path, name=name, target=target, is_manual=False, use_chip_repl_yaml_tester=True
        )


def AllTests(chip_tool: str, run_yamltests_with_chip_repl: bool):
    if run_yamltests_with_chip_repl:
        for test in _hardcoded_python_yaml_tests():
            yield test
        return

    for test in tests_with_command(chip_tool, is_manual=False):
        yield test

    for test in tests_with_command(chip_tool, is_manual=True):
        yield test


__all__ = [
    "TestTarget",
    "TestDefinition",
    "AllTests",
    "ApplicationPaths",
    "linux",
    "runner",
]

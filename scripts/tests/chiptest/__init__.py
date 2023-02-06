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

import json
import os
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Iterator, Set

from . import linux, runner
from .test_definition import ApplicationPaths, TestDefinition, TestTarget

_DEFAULT_CHIP_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", ".."))
_YAML_TEST_SUITE_PATH = os.path.abspath(
    os.path.join(_DEFAULT_CHIP_ROOT, "src/app/tests/suites"))


@dataclass(eq=True, frozen=True)
class ManualTest:
    yaml: str
    reason: str


INVALID_TESTS = {
    "tests.yaml",  # certification/tests.yaml is not a real test
    "PICS.yaml",  # certification/PICS.yaml is not a real test
}


def _LoadManualTestsJson(json_file_path: str) -> Iterator[ManualTest]:
    with open(json_file_path, 'rt') as f:
        data = json.load(f)
        for c in data["collection"]:
            for name in data[c]:
                yield ManualTest(yaml="%s.yaml" % name, reason=json_file_path)


def _GetManualTests() -> Set[ManualTest]:
    manualtests = set()

    # TODO:
    #
    # These are NOT manual tests, but rather "tests that fail in yaml and
    # for this reason are marked as manual".
    #
    # We are working to get this list down to 0.
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_10.yaml", reason="TODO Event Not Supported Yet"))
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_7.yaml", reason="TODO Event Not Supported Yet"))
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_8.yaml", reason="TODO Event Not Supported Yet"))
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_9.yaml", reason="TODO Event Not Supported Yet"))
    manualtests.add(ManualTest(yaml="TestEvents.yaml", reason="TODO Event Not Supported Yet"))

    manualtests.add(ManualTest(yaml="Test_TC_ACE_1_1.yaml", reason="TODO GetCommissionerNodeId Not Supported Yet"))
    manualtests.add(ManualTest(yaml="Test_TC_ACE_1_5.yaml", reason="TODO GetCommissionerNodeId Not Supported Yet"))
    manualtests.add(ManualTest(yaml="Test_TC_SC_5_1.yaml", reason="TODO GetCommissionerNodeId Not Supported Yet"))
    manualtests.add(ManualTest(yaml="Test_TC_SC_5_2.yaml", reason="TODO GetCommissionerNodeId Not Supported Yet"))
    manualtests.add(ManualTest(yaml="TestCommissionerNodeId.yaml", reason="TODO GetCommissionerNodeId Not Supported Yet"))

    manualtests.add(ManualTest(yaml="TestClusterMultiFabric.yaml", reason="TODO Enum Mismatch"))
    manualtests.add(ManualTest(yaml="TestGroupMessaging.yaml", reason="TODO Group Message Not Supported in chip-repl yet"))
    manualtests.add(ManualTest(yaml="TestMultiAdmin.yaml", reason="TODO chip-repl hangs on command expected to fail"))

    # Failing, unclear why. Likely repl specific, used to pass however first
    # failure point seems unrelated. Historically this seems (very?) flaky
    # in repl.
    manualtests.add(ManualTest(yaml="Test_TC_OO_2_4.yaml", reason="Flaky"))

    # Examples:
    #
    # Currently these are not in ciTests.json, however yaml logic currently
    # does NOT use allowlist json but rather finds all yaml files.
    #
    # This is on purpose for now to make it harder to orphan files, however
    # we can reconsider as things evolve.
    manualtests.add(ManualTest(yaml="Config_Example.yaml", reason="Example"))
    manualtests.add(ManualTest(yaml="Config_Variables_Example.yaml", reason="Example"))
    manualtests.add(ManualTest(yaml="PICS_Example.yaml", reason="Example"))
    manualtests.add(ManualTest(yaml="Response_Example.yaml", reason="Example"))
    manualtests.add(ManualTest(yaml="Test_Example.yaml", reason="Example"))

    # Flagged as manual from: src/app/tests/suites/manualTests.json
    for item in _LoadManualTestsJson(os.path.join(_YAML_TEST_SUITE_PATH, "manualTests.json")):
        manualtests.add(item)

    return manualtests


def _AllYamlTests():
    yaml_test_suite_path = Path(_YAML_TEST_SUITE_PATH)

    if not yaml_test_suite_path.exists():
        raise FileNotFoundError(
            f"Expected directory {_YAML_TEST_SUITE_PATH} to exist")

    for path in yaml_test_suite_path.rglob("*.yaml"):
        if not path.is_file():
            continue

        if path.name.endswith('_Simulated.yaml'):
            # Simulated tests are not runnable by repl tests, need
            # separate infrastructure. Exclude theml completely (they are
            # not even manual)
            continue

        yield path


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
    manual_tests = set([b.yaml for b in _GetManualTests()])

    for path in _AllYamlTests():
        if path.name in INVALID_TESTS:
            continue

        yield TestDefinition(
            run_name=str(path),
            name=path.stem,  # `path.stem` converts "some/path/Test_ABC_1.2.yaml" to "Test_ABC.1.2"
            target=target_for_name(path.name),
            is_manual=path.name in manual_tests,
            use_chip_repl_yaml_tester=True
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

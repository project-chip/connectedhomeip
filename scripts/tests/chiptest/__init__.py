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
from .test_definition import ApplicationPaths, TestDefinition, TestTag, TestTarget

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

    # The items below are examples and will never work (likely)
    # completely exclude them
    "Config_Example.yaml",
    "Config_Variables_Example.yaml",
    "PICS_Example.yaml",
    "Response_Example.yaml",
    "Test_Example.yaml",
}


def _IsValidYamlTest(name: str) -> bool:
    """Check if the given file name is a valid YAML test.

    This returns invalid for examples, simulated and other specific tests.
    """

    # Simulated tests are not runnable by repl tests, need
    # separate infrastructure. Exclude them completely (they are
    # not even manual)
    if name.endswith('_Simulated.yaml'):
        return False

    return name not in INVALID_TESTS


def _LoadManualTestsJson(json_file_path: str) -> Iterator[str]:
    with open(json_file_path, 'rt') as f:
        data = json.load(f)
        for c in data["collection"]:
            for name in data[c]:
                yield f"{name}.yaml"


def _GetManualTests() -> Set[str]:
    manualtests = set()

    # Flagged as manual from: src/app/tests/suites/manualTests.json
    for item in _LoadManualTestsJson(os.path.join(_YAML_TEST_SUITE_PATH, "manualTests.json")):
        manualtests.add(item)

    return manualtests


def _GetFlakyTests() -> Set[str]:
    """List of flaky tests.

    While this list is empty, it remains here in case we need to quickly add a new test
    that is flaky.
    """
    return set()


def _GetSlowTests() -> Set[str]:
    """Generally tests using sleep() a bit too freely.

       10s seems like a good threshold to consider something slow
    """
    return {
        "DL_LockUnlock.yaml",                             # ~ 10 seconds
        "TestSubscribe_AdministratorCommissioning.yaml",  # ~ 15 seconds
        "Test_TC_CC_5_1.yaml",                            # ~ 30 seconds
        "Test_TC_CC_5_2.yaml",                            # ~ 30 seconds
        "Test_TC_CC_5_3.yaml",                            # ~ 25 seconds
        "Test_TC_CC_6_1.yaml",                            # ~ 35 seconds
        "Test_TC_CC_6_2.yaml",                            # ~ 60 seconds
        "Test_TC_CC_6_3.yaml",                            # ~ 50 seconds
        "Test_TC_CC_7_2.yaml",                            # ~ 65 seconds
        "Test_TC_CC_7_3.yaml",                            # ~ 70 seconds
        "Test_TC_CC_7_4.yaml",                            # ~ 25 seconds
        "Test_TC_CC_8_1.yaml",                            # ~ 60 seconds
        "Test_TC_DRLK_2_4.yaml",                          # ~ 60 seconds
        "Test_TC_I_2_2.yaml",                             # ~ 15 seconds
        "Test_TC_LVL_3_1.yaml",                           # ~ 35 seconds
        "Test_TC_LVL_4_1.yaml",                           # ~ 55 seconds
        "Test_TC_LVL_5_1.yaml",                           # ~ 35 seconds
        "Test_TC_LVL_6_1.yaml",                           # ~ 10 seconds
        "Test_TC_WNCV_3_1.yaml",                          # ~ 20 seconds
        "Test_TC_WNCV_3_2.yaml",                          # ~ 20 seconds
        "Test_TC_WNCV_3_3.yaml",                          # ~ 15 seconds
        "Test_TC_WNCV_3_4.yaml",                          # ~ 10 seconds
        "Test_TC_WNCV_3_5.yaml",                          # ~ 10 seconds
        "Test_TC_WNCV_4_1.yaml",                          # ~ 20 seconds
        "Test_TC_WNCV_4_2.yaml",                          # ~ 20 seconds
        "Test_TC_WNCV_4_5.yaml",                          # ~ 12 seconds
    }


def _GetInDevelopmentTests() -> Set[str]:
    """Tests that fail in YAML for some reason."""
    return {
        "Test_AddNewFabricFromExistingFabric.yaml",     # chip-repl does not support GetCommissionerRootCertificate and IssueNocChain command
        "TestEqualities.yaml",              # chip-repl does not support pseudo-cluster commands that return a value
        "TestExampleCluster.yaml",          # chip-repl does not load custom pseudo clusters
        "Test_TC_TIMESYNC_1_1.yaml",         # Time sync SDK is not yet ready
        "Test_TC_TIMESYNC_2_3.yaml",         # Time sync SDK is not yet ready
        "TestAttributesById.yaml",           # chip-repl does not support AnyCommands (06/06/2023)
        "TestCommandsById.yaml",             # chip-repl does not support AnyCommands (06/06/2023)
        "TestEventsById.yaml",               # chip-repl does not support AnyCommands (06/06/2023)
        "TestReadNoneSubscribeNone.yaml",    # chip-repl does not support AnyCommands (07/27/2023)
        "Test_TC_DRLK_2_8.yaml",   # Test fails only in chip-repl: Refer--> https://github.com/project-chip/connectedhomeip/pull/27011#issuecomment-1593339855
        "Test_TC_ACE_1_6.yaml",    # Test fails only in chip-repl: Refer--> https://github.com/project-chip/connectedhomeip/pull/27910#issuecomment-1632485584
        "Test_TC_PSCFG_1_1.yaml",  # Power source configuration cluster is deprecated and removed from all-clusters
        "Test_TC_PSCFG_2_1.yaml",  # Power source configuration cluster is deprecated and removed from all-clusters
        "Test_TC_PSCFG_2_2.yaml",  # Power source configuration cluster is deprecated and removed from all-clusters
        "Test_TC_SMOKECO_2_2.yaml",          # chip-repl does not support local timeout (07/20/2023)
        "Test_TC_SMOKECO_2_3.yaml",          # chip-repl does not support local timeout (07/20/2023)
        "Test_TC_SMOKECO_2_4.yaml",          # chip-repl does not support local timeout (07/20/2023)
        "Test_TC_SMOKECO_2_5.yaml",          # chip-repl does not support local timeout (07/20/2023)
        "Test_TC_SMOKECO_2_6.yaml",          # chip-repl does not support local timeout (07/20/2023)
        "Test_TC_IDM_1_2.yaml",              # chip-repl does not support AnyCommands (19/07/2023)
    }


def _AllYamlTests():
    yaml_test_suite_path = Path(_YAML_TEST_SUITE_PATH)

    if not yaml_test_suite_path.exists():
        raise FileNotFoundError(
            f"Expected directory {_YAML_TEST_SUITE_PATH} to exist")

    for path in yaml_test_suite_path.rglob("*.yaml"):
        if not path.is_file():
            continue

        yield path


def target_for_name(name: str):
    if (name.startswith("TV_") or name.startswith("Test_TC_MC_") or
            name.startswith("Test_TC_LOWPOWER_") or name.startswith("Test_TC_KEYPADINPUT_") or
            name.startswith("Test_TC_APPLAUNCHER_") or name.startswith("Test_TC_MEDIAINPUT_") or
            name.startswith("Test_TC_WAKEONLAN_") or name.startswith("Test_TC_CHANNEL_") or
            name.startswith("Test_TC_MEDIAPLAYBACK_") or name.startswith("Test_TC_AUDIOOUTPUT_") or
            name.startswith("Test_TC_TGTNAV_") or name.startswith("Test_TC_APBSC_") or
            name.startswith("Test_TC_CONTENTLAUNCHER_") or name.startswith("Test_TC_ALOGIN_")):
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

    test_tags = set()
    if is_manual:
        test_tags.add(TestTag.MANUAL)

    in_development_tests = [s.replace(".yaml", "") for s in _GetInDevelopmentTests()]

    for name in result.stdout.decode("utf8").split("\n"):
        if not name:
            continue

        target = target_for_name(name)
        tags = test_tags.copy()
        if name in in_development_tests:
            tags.add(TestTag.IN_DEVELOPMENT)

        yield TestDefinition(
            run_name=name, name=name, target=target, tags=tags
        )


# TODO We will move away from hardcoded list of yamltests to run all file when yamltests
# parser/runner reaches parity with the code gen version.
def _hardcoded_python_yaml_tests():
    manual_tests = _GetManualTests()
    flaky_tests = _GetFlakyTests()
    slow_tests = _GetSlowTests()
    in_development_tests = _GetInDevelopmentTests()

    for path in _AllYamlTests():
        if not _IsValidYamlTest(path.name):
            continue

        tags = set()
        if path.name in manual_tests:
            tags.add(TestTag.MANUAL)

        if path.name in flaky_tests:
            tags.add(TestTag.FLAKY)

        if path.name in slow_tests:
            tags.add(TestTag.SLOW)

        if path.name in in_development_tests:
            tags.add(TestTag.IN_DEVELOPMENT)

        yield TestDefinition(
            run_name=str(path),
            name=path.stem,  # `path.stem` converts "some/path/Test_ABC_1.2.yaml" to "Test_ABC.1.2"
            target=target_for_name(path.name),
            tags=tags,
        )


def AllYamlTests():
    for test in _hardcoded_python_yaml_tests():
        yield test


def AllChipToolTests(chip_tool: str):
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

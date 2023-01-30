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
    manualtests.add(ManualTest(yaml="DL_LockUnlock.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="OTA_SuccessfulTransfer.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_Example_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_Example_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_Example_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ACE_1_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ACE_1_5.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_10.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_4.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_7.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_8.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ACL_2_9.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ACT_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_APBSC_9_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_APPLAUNCHER_3_8.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_APPLAUNCHER_3_9.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_AUDIOOUTPUT_7_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_BINFO_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_BINFO_2_3_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_BOOL_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_BRBINFO_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_3_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_3_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_4_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_4_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_4_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_4_4.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_5_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_5_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_5_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_6_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_6_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_6_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_7_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_7_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_7_4.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CC_8_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CGEN_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CHANNEL_5_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_CHANNEL_5_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DESC_2_2_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DGETH_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DGETH_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DGETH_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DGGEN_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DGSW_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DGTHREAD_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DGTHREAD_2_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DGWIFI_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DGWIFI_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DRLK_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DRLK_2_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DRLK_2_4.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DRLK_2_5.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DRLK_2_7.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_DRLK_2_9.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_FLABEL_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_FLABEL_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_FLW_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_G_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_I_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_I_2_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_I_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ILL_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LCFG_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LOWPOWER_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LTIME_1_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LUNIT_1_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LUNIT_2_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LUNIT_3_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LVL_3_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LVL_4_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LVL_5_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_LVL_6_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_MEDIAINPUT_3_12.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_MEDIAINPUT_3_13.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_MEDIAPLAYBACK_6_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_MEDIAPLAYBACK_6_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_MEDIAPLAYBACK_6_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_MEDIAPLAYBACK_6_4.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_OCC_2_2_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_OCC_2_4_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_OO_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_OO_2_4.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_PCC_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_PRS_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_PRS_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_PS_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_PS_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_PSCFG_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_RH_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_SC_5_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_SC_5_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_SWTCH_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_TSTAT_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_TSTAT_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_TSUIC_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_ULABEL_3_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WAKEONLAN_4_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_2_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_2_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_2_4.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_2_5.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_3_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_3_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_3_3.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_3_4.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_3_5.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_4_1.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_4_2.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_4_5.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="Test_TC_WNCV_5_1_Simulated.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestArmFailSafe.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestBasicInformation.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestCASERecovery.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestClusterMultiFabric.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestCommissionerNodeId.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestDelayCommands.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestDiscovery.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestEvents.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestGeneralCommissioning.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestGroupMessaging.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestLevelControlWithOnOffDependency.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestLogCommands.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestModeSelectCluster.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestMultiAdmin.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestSystemCommands.yaml", reason="TODO"))
    manualtests.add(ManualTest(yaml="TestUserLabelCluster.yaml", reason="TODO"))

    # Flaky tests? these seem to pass though
    manualtests.add(ManualTest(yaml="DL_Schedules.yaml", reason="Flaky"))
    manualtests.add(ManualTest(yaml="Test_TC_LVL_2_1.yaml", reason="Flaky"))

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

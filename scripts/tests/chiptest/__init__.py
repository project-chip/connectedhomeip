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
    currently_supported_yaml_tests = {
        "Test_TC_ACL_1_1.yaml",
        "Test_TC_ACL_2_1.yaml",
        "Test_TC_BOOL_1_1.yaml",
        "Test_TC_ACT_1_1.yaml",
        "Test_TC_BIND_1_1.yaml",
        "Test_TC_OPCREDS_1_2.yaml",
        "Test_TC_BINFO_1_1.yaml",
        "Test_TC_DESC_1_1.yaml",
        "Test_TC_DLOG_1_1.yaml",
        "Test_TC_FLW_1_1.yaml",
        "Test_TC_FLW_2_1.yaml",
        "Test_TC_FLABEL_1_1.yaml",
        "Test_TC_CGEN_1_1.yaml",
        "Test_TC_DGGEN_1_1.yaml",
        "Test_TC_I_1_1.yaml",
        "Test_TC_I_2_1.yaml",
        "Test_TC_ILL_1_1.yaml",
        "Test_TC_ILL_2_1.yaml",
        # "Test_TC_LVL_2_1.yaml", # TODO: Fix flakyness
        "Test_TC_LVL_2_2.yaml",
        "Test_TC_LCFG_1_1.yaml",
        "Test_TC_LTIME_1_2.yaml",
        "Test_TC_LOWPOWER_1_1.yaml",
        "Test_TC_WAKEONLAN_1_5.yaml",
        "Test_TC_AUDIOOUTPUT_1_8.yaml",
        "Test_TC_TGTNAV_1_9.yaml",
        "Test_TC_TGTNAV_8_2.yaml",
        "Test_TC_APBSC_1_10.yaml",
        "Test_TC_ALOGIN_1_12.yaml",
        "Test_TC_KEYPADINPUT_3_2.yaml",
        "Test_TC_KEYPADINPUT_3_3.yaml",
        "Test_TC_APPLAUNCHER_3_5.yaml",
        "Test_TC_APPLAUNCHER_3_6.yaml",
        "Test_TC_MEDIAINPUT_3_10.yaml",
        "Test_TC_MEDIAINPUT_3_11.yaml",
        "Test_TC_CHANNEL_5_1.yaml",
        "Test_TC_CONTENTLAUNCHER_10_1.yaml",
        "Test_TC_OCC_1_1.yaml",
        "Test_TC_PSCFG_1_1.yaml",
        "Test_TC_PSCFG_2_1.yaml",
        "Test_TC_RH_1_1.yaml",
        "Test_TC_RH_2_1.yaml",
        "Test_TC_SWTCH_2_1.yaml",
        "Test_TC_TMP_1_1.yaml",
        "Test_TC_TMP_2_1.yaml",
        "Test_TC_TSUIC_1_1.yaml",
        "Test_TC_TSUIC_2_1.yaml",
        "Test_TC_DGTHREAD_2_2.yaml",
        "Test_TC_DGTHREAD_2_4.yaml",
        "Test_TC_ULABEL_1_1.yaml",
        "Test_TC_ULABEL_2_1.yaml",
        "Test_TC_ULABEL_2_2.yaml",
        "Test_TC_ULABEL_2_3.yaml",
        "Test_TC_ULABEL_2_4.yaml",
        "Test_TC_DGWIFI_2_3.yaml",
        "TV_TargetNavigatorCluster.yaml",
        "TV_AudioOutputCluster.yaml",
        "TV_ApplicationLauncherCluster.yaml",
        "TV_KeypadInputCluster.yaml",
        "TV_AccountLoginCluster.yaml",
        "TV_WakeOnLanCluster.yaml",
        "TV_ApplicationBasicCluster.yaml",
        "TV_ChannelCluster.yaml",
        "TV_LowPowerCluster.yaml",
        "TV_ContentLauncherCluster.yaml",
        "TV_MediaInputCluster.yaml",
        "TestCluster.yaml",
        "TestConstraints.yaml",
        "TestSaveAs.yaml",
        "TestConfigVariables.yaml",
        "TestFabricRemovalWhileSubscribed.yaml",
        "TestIdentifyCluster.yaml",
        "TestSelfFabricRemoval.yaml",
        "TestBinding.yaml",
        "TestUserLabelClusterConstraints.yaml",
        "TestFanControl.yaml",
        "TestAccessControlConstraints.yaml",
        "TestCommissioningWindow.yaml",
        "TestSubscribe_OnOff.yaml",
        "TestClusterComplexTypes.yaml",
        "TestGroupsCluster.yaml",
        "TestOperationalCredentialsCluster.yaml",
        "Test_TC_AUDIOOUTPUT_7_1.yaml",
        "Test_TC_BOOL_2_1.yaml",
        "Test_TC_OO_2_1.yaml",
        "Test_TC_TGTNAV_8_1.yaml",
        "Test_TC_WNCV_2_3.yaml",
        "Test_TC_WNCV_4_3.yaml",
        "Test_TC_WNCV_4_4.yaml",
        # "DL_Schedules.yaml",  # TODO: Fix flakyness
        "DL_UsersAndCredentials.yaml",
    }

    invalid_tests = {
        "tests.yaml",  # src/app/tests/suites/certification/tests.yaml is not a real test
        "PICS.yaml",  # src/app/tests/suites/certification/PICS.yaml is not a real test
    }

    # By default assume all yaml files are valid test cases, however only a
    # smaller subset is known to pass, all the rest are marked "manual"
    # For sanity check, all known supported tests MUST exist
    found_supported_tests = set()
    for path in _AllYamlTests():
        if path.name in invalid_tests:
            continue

        is_supported = path.name in currently_supported_yaml_tests

        if is_supported:
            found_supported_tests.add(path.name)

        file_path = str(path)

        target = target_for_name(path.name)

        # `path.stem` converts "some/path/Test_ABC_1.2.yaml" to "Test_ABC.1.2"
        yield TestDefinition(
            run_name=file_path, name=path.stem, target=target, is_manual=not is_supported, use_chip_repl_yaml_tester=True
        )

    if found_supported_tests != currently_supported_yaml_tests:
        raise Exception("Did not find YAMLs for all supported tests: %r" % (
            currently_supported_yaml_tests - found_supported_tests))


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

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
import logging
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
    "PICS.yaml",  # certification/PICS.yaml is not a real test

    # The items below are examples and will never work (likely)
    # completely exclude them
    "Config_Example.yaml",
    "Config_Variables_Example.yaml",
    "PICS_Example.yaml",
    "Response_Example.yaml",
    "Test_Example.yaml",
    "Test_Example_1.yaml",
    "Test_Example_2.yaml",
    "Test_Example_3.yaml",
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


def _GetExtraSlowTests() -> Set[str]:
    """Generally tests using sleep() so much they should never run in CI.

       1 minute seems like a good threshold to consider something extra slow
    """
    return {
        "Test_TC_DGGEN_2_1.yaml",                         # > 2 hours
    }


def _GetInDevelopmentTests() -> Set[str]:
    """Tests that fail in YAML for some reason."""
    return {
        "Test_TC_PSCFG_1_1.yaml",  # Power source configuration cluster is deprecated and removed from all-clusters
        "Test_TC_PSCFG_2_1.yaml",  # Power source configuration cluster is deprecated and removed from all-clusters
        "Test_TC_PSCFG_2_2.yaml",  # Power source configuration cluster is deprecated and removed from all-clusters
        "Test_TC_SMOKECO_2_2.yaml",          # chip-repl does not support local timeout (07/20/2023) and test assumes
                                             # TestEventTriggersEnabled is true, which it's not in CI.
        "Test_TC_SMOKECO_2_3.yaml",          # chip-repl does not support local timeout (07/20/2023) and test assumes
                                             # TestEventTriggersEnabled is true, which it's not in CI.
        "Test_TC_SMOKECO_2_4.yaml",          # chip-repl does not support local timeout (07/20/2023) and test assumes
                                             # TestEventTriggersEnabled is true, which it's not in CI.
        "Test_TC_SMOKECO_2_5.yaml",          # chip-repl does not support local timeout (07/20/2023) and test assumes
                                             # TestEventTriggersEnabled is true, which it's not in CI.
        "Test_TC_SMOKECO_2_6.yaml",          # chip-repl does not support local timeout (07/20/2023) and test assumes
                                             # TestEventTriggersEnabled is true, which it's not in CI.
        "Test_TC_BR_5.yaml",                 # [TODO] Fabric Sync example app has not been integrated into CI yet.
    }


def _GetChipToolUnsupportedTests() -> Set[str]:
    """Tests that fail in chip-tool for some reason"""
    return {
        "TestDiagnosticLogsDownloadCommand",  # chip-tool does not implement a bdx download command.
    }


def _GetDarwinFrameworkToolUnsupportedTests() -> Set[str]:
    """Tests that fail in darwin-framework-tool for some reason"""
    return {
        "DL_LockUnlock",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "DL_UsersAndCredentials",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_AddNewFabricFromExistingFabric",  # darwin-framework-tool does not support the GetCommissionerRootCertificate command.
        # The name of the arguments once converted differs for chip-tool and darwin-framework-tool (attribute-ids vs attribute-id. See #31934)
        "TestAttributesById",
        "TestBasicInformation",  # darwin-framework-tool does not support writing readonly attributes by name
        "TestClusterComplexTypes",  # Darwin framework has no way to represent a present but null optional nullable field.
        # When reading TestFabricScoped in TestClusterMultiFabric, the result differs because of missing fields that have been declared in the YAML step with null value to workaround some limitation of the test harness (#29110)
        "TestClusterMultiFabric",
        "TestCommandsById",  # darwin-framework-tool does not support writing readonly attributes by name
        "TestDiagnosticLogs",  # darwin-framework-tool does not implement a BDXTransferServerDelegate
        "TestDiscovery",  # darwin-framework-tool does not support dns-sd commands.
        "TestEvents",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "TestEventsById",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "TestGroupMessaging",  # darwin-framework-tool does not support group commands.
        "TestIcdManagementCluster",  # darwin-framework-tool does not support ICD registration
        "TestUnitTestingClusterMei",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "TestReadNoneSubscribeNone",  # darwin-framework-tool does not supports those commands.
        "TestDiagnosticLogsDownloadCommand",  # test is flaky in darwin. Please see #32636

        "Test_TC_ACE_1_6",  # darwin-framework-tool does not support group commands.
        "Test_TC_ACL_2_5",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_ACL_2_6",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_ACL_2_7",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_ACL_2_8",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_ACL_2_9",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_ACL_2_10",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_BINFO_2_1",  # darwin-framework-tool does not support writing readonly attributes by name
        "Test_TC_BINFO_2_2",  # darwin-framework-tool does not currently support reading or subscribing to Events
        # The name of the arguments once converted differs for chip-tool and darwin-framework-tool (attribute-ids vs attribute-id. See #31934)
        "Test_TC_BRBINFO_2_1",
        "Test_TC_DGGEN_2_3",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_DRLK_2_1",  # darwin-framework-tool does not support writing readonly attributes by name
        "Test_TC_DGTHREAD_2_1",  # Thread Network Diagnostics is not implemented under darwin.
        "Test_TC_DGTHREAD_2_2",  # Thread Network Diagnostics is not implemented under darwin.
        "Test_TC_DGTHREAD_2_3",  # Thread Network Diagnostics is not implemented under darwin.
        "Test_TC_DGTHREAD_2_4",  # Thread Network Diagnostics is not implemented under darwin.
        "Test_TC_GRPKEY_2_1",  # darwin-framework-tool does not support writing readonly attributes by name
        "Test_TC_LCFG_2_1",  # darwin-framework-tool does not support writing readonly attributes by name
        "Test_TC_OPCREDS_3_7",  # darwin-framework-tool does not support the GetCommissionerRootCertificate command.
        "Test_TC_SMOKECO_2_2",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_SMOKECO_2_3",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_SMOKECO_2_4",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_SMOKECO_2_5",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_SMOKECO_2_6",  # darwin-framework-tool does not currently support reading or subscribing to Events
        "Test_TC_SC_4_1",  # darwin-framework-tool does not support dns-sd commands.
        "Test_TC_SC_5_2",  # darwin-framework-tool does not support group commands.
        "Test_TC_S_2_3",  # darwin-framework-tool does not support group commands.
        "Test_TC_THNETDIR_2_2",  # darwin-framework-tool does not support negative timed-invoke tests
    }


def _GetChipReplUnsupportedTests() -> Set[str]:
    """Tests that fail in chip-repl for some reason"""
    return {
        "Test_AddNewFabricFromExistingFabric.yaml",     # chip-repl does not support GetCommissionerRootCertificate and IssueNocChain command
        "Test_TC_OPCREDS_3_7.yaml",         # chip-repl does not support GetCommissionerRootCertificate and IssueNocChain command
        "TestExampleCluster.yaml",          # chip-repl does not load custom pseudo clusters
        "TestAttributesById.yaml",           # chip-repl does not support AnyCommands (06/06/2023)
        "TestCommandsById.yaml",             # chip-repl does not support AnyCommands (06/06/2023)
        "TestEventsById.yaml",               # chip-repl does not support AnyCommands (06/06/2023)
        "TestReadNoneSubscribeNone.yaml",    # chip-repl does not support AnyCommands (07/27/2023)
        "Test_TC_IDM_1_2.yaml",              # chip-repl does not support AnyCommands (19/07/2023)
        "Test_TC_BRBINFO_2_1.yaml",          # chip-repl does not support AnyCommands (24/07/2024)
        "TestThermostat.yaml",               # chip-repl does not support AnyCommands (14/10/2024)
        "TestIcdManagementCluster.yaml",   # TODO(#30430): add ICD registration support in chip-repl
        "Test_TC_ICDM_3_4.yaml",           # chip-repl does not support ICD registration
        # chip-repl and chip-tool disagree on what the YAML here should look like: https://github.com/project-chip/connectedhomeip/issues/29110
        "TestClusterMultiFabric.yaml",
        "TestDiagnosticLogs.yaml",          # chip-repl does not implement a BDXTransferServerDelegate
        "TestDiagnosticLogsDownloadCommand.yaml",  # chip-repl does not implement the bdx download command
    }


def _GetPurposefulFailureTests() -> Set[str]:
    """Tests that fail in YAML on purpose."""
    return {
        "TestPurposefulFailureEqualities.yaml",
        "TestPurposefulFailureExtraReportingOnToggle.yaml",
        "TestPurposefulFailureNotNullConstraint.yaml",
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
    if name.startswith("TestFabricSync"):
        return TestTarget.FABRIC_SYNC
    if name.startswith("OTA_"):
        return TestTarget.OTA
    if name.startswith("Test_TC_BRBINFO_") or name.startswith("Test_TC_ACT_"):
        return TestTarget.BRIDGE
    if name.startswith("TestIcd") or name.startswith("Test_TC_ICDM_"):
        return TestTarget.LIT_ICD
    if name.startswith("Test_TC_MWOCTRL_") or name.startswith("Test_TC_MWOM_"):
        return TestTarget.MWO
    if name.startswith("Test_TC_RVCRUNM_") or name.startswith("Test_TC_RVCCLEANM_") or name.startswith("Test_TC_RVCOPSTATE_"):
        return TestTarget.RVC
    if name.startswith("Test_TC_TBRM_") or name.startswith("Test_TC_THNETDIR_") or name.startswith("Test_TC_WIFINM_"):
        return TestTarget.NETWORK_MANAGER
    return TestTarget.ALL_CLUSTERS


def tests_with_command(chip_tool: str, is_manual: bool):
    """Executes `chip_tool` binary to see what tests are available, using cmd
    to get the list.
    """
    cmd = "list"
    if is_manual:
        cmd += "-manual"

    cmd = [chip_tool, "tests", cmd]
    result = subprocess.run(cmd, capture_output=True, encoding="utf-8")
    if result.returncode != 0:
        logging.error(f'Failed to run {cmd}:')
        logging.error('STDOUT: ' + result.stdout)
        logging.error('STDERR: ' + result.stderr)
        result.check_returncode()

    test_tags = set()
    if is_manual:
        test_tags.add(TestTag.MANUAL)

    in_development_tests = [s.replace(".yaml", "") for s in _GetInDevelopmentTests()]

    for name in result.stdout.split("\n"):
        if not name:
            continue

        target = target_for_name(name)
        tags = test_tags.copy()
        if name in in_development_tests:
            tags.add(TestTag.IN_DEVELOPMENT)

        yield TestDefinition(
            run_name=name, name=name, target=target, tags=tags
        )


def _AllFoundYamlTests(treat_repl_unsupported_as_in_development: bool, treat_dft_unsupported_as_in_development: bool, treat_chip_tool_unsupported_as_in_development: bool, use_short_run_name: bool):
    """
    use_short_run_name should be true if we want the run_name to be "Test_ABC" instead of "some/path/Test_ABC.yaml"
    """
    manual_tests = _GetManualTests()
    flaky_tests = _GetFlakyTests()
    slow_tests = _GetSlowTests()
    extra_slow_tests = _GetExtraSlowTests()
    in_development_tests = _GetInDevelopmentTests()
    chip_repl_unsupported_tests = _GetChipReplUnsupportedTests()
    dft_unsupported_as_in_development_tests = _GetDarwinFrameworkToolUnsupportedTests()
    chip_tool_unsupported_as_in_development_tests = _GetChipToolUnsupportedTests()
    purposeful_failure_tests = _GetPurposefulFailureTests()

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

        if path.name in extra_slow_tests:
            tags.add(TestTag.EXTRA_SLOW)

        if path.name in in_development_tests:
            tags.add(TestTag.IN_DEVELOPMENT)

        if path.name in purposeful_failure_tests:
            tags.add(TestTag.PURPOSEFUL_FAILURE)

        if treat_repl_unsupported_as_in_development and path.name in chip_repl_unsupported_tests:
            tags.add(TestTag.IN_DEVELOPMENT)

        if use_short_run_name:
            run_name = path.stem  # `path.stem` converts "some/path/Test_ABC_1.2.yaml" to "Test_ABC.1.2"
        else:
            run_name = str(path)

        if treat_dft_unsupported_as_in_development and run_name in dft_unsupported_as_in_development_tests:
            tags.add(TestTag.IN_DEVELOPMENT)

        if treat_chip_tool_unsupported_as_in_development and run_name in chip_tool_unsupported_as_in_development_tests:
            tags.add(TestTag.IN_DEVELOPMENT)

        yield TestDefinition(
            run_name=run_name,
            name=path.stem,  # `path.stem` converts "some/path/Test_ABC_1.2.yaml" to "Test_ABC.1.2"
            target=target_for_name(path.name),
            tags=tags,
        )


def AllReplYamlTests():
    for test in _AllFoundYamlTests(treat_repl_unsupported_as_in_development=True, treat_dft_unsupported_as_in_development=False, treat_chip_tool_unsupported_as_in_development=False, use_short_run_name=False):
        yield test


def AllChipToolYamlTests(use_short_run_name: bool = True):
    for test in _AllFoundYamlTests(treat_repl_unsupported_as_in_development=False, treat_dft_unsupported_as_in_development=False, treat_chip_tool_unsupported_as_in_development=True, use_short_run_name=use_short_run_name):
        yield test


def AllDarwinFrameworkToolYamlTests():
    for test in _AllFoundYamlTests(treat_repl_unsupported_as_in_development=False, treat_dft_unsupported_as_in_development=True, treat_chip_tool_unsupported_as_in_development=False, use_short_run_name=True):
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

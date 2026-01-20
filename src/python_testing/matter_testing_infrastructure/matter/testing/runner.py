#
#    Copyright (c) 2024 Project CHIP Authors
#    All rights reserved.
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

# Add new imports for argument parsing functions
import argparse
import asyncio
import importlib
import json
import logging
import os
import pathlib
import re
import sys
import typing
from binascii import unhexlify
from dataclasses import asdict as dataclass_asdict
from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
from itertools import chain
from pathlib import Path
from typing import Any, List, Optional, Tuple
from unittest.mock import MagicMock

from mobly import signals, utils
from mobly.config_parser import ENV_MOBLY_LOGPATH, TestRunConfig
from mobly.test_runner import TestRunner

import matter.testing.global_stash as global_stash
from matter.clusters import Attribute
# Add imports for argument parsing dependencies
from matter.testing.defaults import TestingDefaults
# Add imports for argument parsing dependencies
from matter.testing.pics import read_pics_from_file

try:
    from matter_yamltests.hooks import TestRunnerHooks
except ImportError:
    class TestRunnerHooks:  # type: ignore[no-redef] # Conditional fallback, not a true redefinition
        pass
try:
    from matter.tracing import TracingContext
except ImportError:
    class TracingContext:  # type: ignore[no-redef] # Conditional fallback, not a true redefinition
        def __enter__(self):
            return self

        def __exit__(self, *args):
            pass

        def StartFromString(self, destination):
            pass

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from matter.testing.matter_test_config import MatterTestConfig

LOGGER = logging.getLogger(__name__)


def default_paa_rootstore_from_root(root_path: pathlib.Path) -> Optional[pathlib.Path]:
    """Attempt to find a PAA trust store following SDK convention at `root_path`

    This attempts to find {root_path}/credentials/development/paa-root-certs.

    Returns the fully resolved path on success or None if not found.
    """
    start_path = root_path.resolve()
    cred_path = start_path.joinpath("credentials")
    dev_path = cred_path.joinpath("development")
    paa_path = dev_path.joinpath("paa-root-certs")

    return paa_path.resolve() if all(path.exists() for path in [cred_path, dev_path, paa_path]) else None


def get_default_paa_trust_store(root_path: pathlib.Path) -> pathlib.Path:
    """Attempt to find a PAA trust store starting at `root_path`.

    This tries to find by various heuristics, and goes up one level at a time
    until found. After a given number of levels, it will stop.

    This returns `root_path` if not PAA store is not found.
    """
    # TODO: Add heuristics about TH default PAA location
    cur_dir = pathlib.Path.cwd()
    max_levels = 10

    for level in range(max_levels):
        paa_trust_store_path = default_paa_rootstore_from_root(cur_dir)
        if paa_trust_store_path is not None:
            return paa_trust_store_path

        # Go back one level
        cur_dir = cur_dir.joinpath("..")
    else:
        # On not having found a PAA dir, just return current dir to avoid blow-ups
        return pathlib.Path.cwd()


class InternalTestRunnerHooks(TestRunnerHooks):
    """
    Implementation of TestRunnerHooks that logs test execution progress.

    This class provides hooks for the test runner to report on test execution
    status, including test starts, stops, steps, and failures.
    """

    def start(self, count: int):
        """
        Called when the test runner starts a new test set.

        Args:
            count: The number of tests in the set.
        """
        LOGGER.info(f'Starting test set, running {count} tests')

    def stop(self, duration: int):
        """
        Called when the test runner finishes a test set.

        Args:
            duration: The duration of the test set in milliseconds.
        """
        LOGGER.info(f'Finished test set, ran for {duration}ms')

    def test_start(
            self,
            filename: str,
            name: str,
            count: int,
            steps: list[str] = []):
        """
        Called when an individual test starts.

        Args:
            filename: Source file containing the test
            name: Name of the test
            count: Number of steps in the test
            steps: List of step descriptions
        """
        LOGGER.info(f'Starting test from {filename}: {name} - {count} steps')

    def test_stop(self, exception: Exception, duration: int):
        """
        Called when an individual test completes.

        Args:
            exception: Exception raised during test execution, or None if successful
            duration: Test execution duration in milliseconds
        """
        LOGGER.info(f'Finished test in {duration}ms')

    def step_skipped(self, name: str, expression: str):
        """
        Called when a test step is skipped.

        Args:
            name: Name of the skipped step
            expression: Condition expression that caused the skip
        """
        # TODO: Do we really need the expression as a string? We can evaluate
        # this in code very easily
        LOGGER.info(f'\t\t**** Skipping: {name}')

    def step_start(self, name: str):
        """
        Called when a test step starts.

        Args:
            name: Name of the step including its number
        """
        # The way I'm calling this, the name is already includes the step
        # number, but it seems like it might be good to separate these
        LOGGER.info(f'\t\t***** Test Step {name}')

    def step_success(self, logger, logs, duration: int, request):
        """
        Called when a test step completes successfully.

        Args:
            logger: Logger instance
            logs: Captured logs during step execution
            duration: Step execution duration in milliseconds
            request: The original test request
        """
        pass

    def step_failure(self, logger, logs, duration: int, request, received):
        """
        Called when a test step fails.

        Args:
            logger: Logger instance
            logs: Captured logs during step execution
            duration: Step execution duration in milliseconds
            request: The original test request
            received: The actual response received
        """
        LOGGER.info('\t\t***** Test Failure : ')
        if received is not None:
            LOGGER.info(f'\t\t      Received: {received}')
        if request is not None:
            LOGGER.info(f'\t\t      Expected: {request}')

    def step_unknown(self):
        """
        This method is called when the result of running a step is unknown. For example during a dry-run.
        """
        pass

    def show_prompt(self,
                    msg: str,
                    placeholder: Optional[str] = None,
                    default_value: Optional[str] = None) -> None:
        """
        This method is called when the test runner needs to prompt the user for input.

        Args:
            msg: The message to display to the user
            placeholder: Optional placeholder for user input
            default_value: Optional default value for user input
        """
        pass

    def test_skipped(self, filename: str, name: str):
        """
        Called when a test is skipped.

        Args:
            filename: Source file containing the test
            name: Name of the test
        """
        LOGGER.info(f"Skipping test from {filename}: {name}")


@dataclass
class TestStep:
    test_plan_number: typing.Union[int, str]
    description: str
    expectation: str = ""
    is_commissioning: bool = False

    def __str__(self):
        return f'{self.test_plan_number}: {self.description}\tExpected outcome: {self.expectation}'


@dataclass
class TestInfo:
    function: str
    desc: str
    steps: list[TestStep]
    pics: list[str]


def generate_mobly_test_config(matter_test_config):
    """
    Generate a Mobly test configuration from Matter test configuration.

    Args:
        matter_test_config: Matter test configuration object

    Returns:
        TestRunConfig: Configured Mobly test run configuration
    """
    test_run_config = TestRunConfig()
    # We use a default name. We don't use Mobly YAML configs, so that we can be
    # freestanding without relying
    test_run_config.testbed_name = "MatterTest"

    log_path = matter_test_config.logs_path
    log_path = TestingDefaults.LOG_PATH if log_path is None else log_path
    if ENV_MOBLY_LOGPATH in os.environ:
        log_path = os.environ[ENV_MOBLY_LOGPATH]

    test_run_config.log_path = log_path
    # TODO: For later, configure controllers
    test_run_config.controller_configs = {}

    test_run_config.user_params = matter_test_config.global_test_params

    return test_run_config


def _find_test_class():
    """Finds the test class in a test script.
    Walk through module members and find the subclass of MatterBaseTest. Only
    one subclass is allowed in a test script.
    Returns:
      The test class in the test module.
    Raises:
      SystemExit: Raised if the number of test classes is not exactly one.
    """
    from matter.testing.matter_testing import MatterBaseTest

    def get_subclasses(cls: Any):
        subclasses = utils.find_subclasses_in_module([cls], sys.modules['__main__'])
        return [c for c in subclasses if c.__name__ != cls.__name__]

    def has_subclasses(cls: Any):
        return get_subclasses(cls) != []

    subclasses_matter_test_base = get_subclasses(MatterBaseTest)
    leaf_subclasses = [s for s in subclasses_matter_test_base if not has_subclasses(s)]

    if len(leaf_subclasses) != 1:
        print(
            'Exactly one subclass of `MatterBaseTest` should be in the main file. Found %s.' %
            str([subclass.__name__ for subclass in leaf_subclasses]))
        sys.exit(1)

    return leaf_subclasses[0]


def default_matter_test_main():
    """Execute the test class in a test module.
    This is the default entry point for running a test script file directly.
    In this case, only one test class in a test script is allowed.
    To make your test script executable, add the following to your file:
    .. code-block:: python
      from matter.testing.runner import default_matter_test_main
      ...
      if __name__ == '__main__':
        default_matter_test_main()
    """

    matter_test_config = parse_matter_test_args()

    # Find the test class in the test script.
    test_class = _find_test_class()

    hooks = InternalTestRunnerHooks()

    run_tests(test_class, matter_test_config, hooks)


def get_test_info(test_class, matter_test_config) -> list[TestInfo]:
    test_config = generate_mobly_test_config(matter_test_config)
    base = test_class(test_config)

    if len(matter_test_config.tests) > 0:
        tests = matter_test_config.tests
    else:
        tests = base.get_existing_test_names()

    info = []
    for t in tests:
        info.append(TestInfo(t, steps=base.get_test_steps(
            t), desc=base.get_test_desc(t), pics=base.get_test_pics(t)))

    return info


def run_tests_no_exit(
        test_class,
        matter_test_config,
        event_loop: asyncio.AbstractEventLoop,
        hooks: TestRunnerHooks,
        default_controller=None,
        external_stack=None) -> bool:
    """
    Run Matter tests without exiting the process on failure.

    This function sets up the test environment, runs the specified tests,
    and returns a boolean indicating success or failure.

    Args:
        test_class: The test class to run
        matter_test_config: Configuration for Matter tests
        event_loop: Asyncio event loop to use for async operations
        hooks: Test runner hooks for monitoring test progress
        default_controller: Optional pre-configured controller
        external_stack: Optional external Matter stack

    Returns:
        bool: True if all tests passed, False otherwise
    """

    # Lazy import to avoid circular dependency
    from typing import TYPE_CHECKING

    from matter.testing.matter_stack_state import MatterStackState
    if TYPE_CHECKING:
        from matter.testing.commissioning import CommissionDeviceTest
    else:
        CommissionDeviceTest = None  # Initial placeholder

    # Actual runtime import
    if CommissionDeviceTest is None:
        from matter.testing.commissioning import CommissionDeviceTest

    # NOTE: It's not possible to pass event loop via Mobly TestRunConfig user params, because the
    #       Mobly deep copies the user params before passing them to the test class and the event
    #       loop is not serializable. So, we are setting the event loop as a test class member.
    CommissionDeviceTest.event_loop = event_loop
    test_class.event_loop = event_loop

    # Load test config file.
    test_config = generate_mobly_test_config(matter_test_config)

    # Parse test specifiers if exist.
    tests = None
    if len(matter_test_config.tests) > 0:
        tests = matter_test_config.tests

    if external_stack:
        stack = external_stack
    else:
        stack = MatterStackState(matter_test_config)

    with TracingContext() as tracing_ctx:
        for destination in matter_test_config.trace_to:
            tracing_ctx.StartFromString(destination)

        test_config.user_params["matter_stack"] = global_stash.stash_globally(stack)

        # TODO: Steer to right FabricAdmin!
        # TODO: If CASE Admin Subject is a CAT tag range, then make sure to
        # issue NOC with that CAT tag
        if not default_controller:
            default_controller = stack.certificate_authorities[0].adminList[0].NewController(
                nodeId=matter_test_config.controller_node_id,
                paaTrustStorePath=str(
                    matter_test_config.paa_trust_store_path),
                catTags=matter_test_config.controller_cat_tags,
                dacRevocationSetPath=matter_test_config.dac_revocation_set_path if matter_test_config.dac_revocation_set_path else ""
            )
        test_config.user_params["default_controller"] = global_stash.stash_globally(
            default_controller)
        test_config.user_params["matter_test_config"] = global_stash.stash_globally(
            matter_test_config)
        test_config.user_params["hooks"] = global_stash.stash_globally(hooks)

        # Execute the test class with the config
        ok = True

        test_config.user_params["certificate_authority_manager"] = global_stash.stash_globally(
            stack.certificate_authority_manager)

        # Execute the test class with the config
        ok = True

        def _handler(loop, context):
            loop.default_exception_handler(context)
            nonlocal ok
            # Fail the test run on unhandled exceptions.
            ok = False

        # Set custom exception handler to catch unhandled exceptions.
        event_loop.set_exception_handler(_handler)

        runner = TestRunner(log_dir=test_config.log_path,
                            testbed_name=test_config.testbed_name)

        with runner.mobly_logger():
            if matter_test_config.commissioning_method is not None:
                runner.add_test_class(test_config, CommissionDeviceTest, None)

            # Add the tests selected unless we have a commission-only request
            if not matter_test_config.commission_only:
                runner.add_test_class(test_config, test_class, tests)

            if hooks:
                # Right now, we only support running a single test class at once,
                # but it's relatively easy to expand that to make the test process faster
                # TODO: support a list of tests
                hooks.start(count=1)
                # Mobly gives the test run time in seconds, lets be a bit more
                # precise
                runner_start_time = datetime.now(timezone.utc)

            try:
                runner.run()
                ok = runner.results.is_all_pass and ok
                if matter_test_config.fail_on_skipped_tests and runner.results.skipped:
                    ok = False
            except TimeoutError:
                ok = False
            except signals.TestAbortAll:
                ok = False
            except Exception:
                LOGGER.exception('Exception when executing %s.',
                                 test_config.testbed_name)
                ok = False

    if hooks:
        duration = (datetime.now(timezone.utc) -
                    runner_start_time) / timedelta(microseconds=1)
        hooks.stop(duration=duration)

    if not external_stack:
        async def shutdown():
            stack.Shutdown()
        # Shutdown the stack when all done. Use the async runner to ensure that
        # during the shutdown callbacks can use tha same async context which was used
        # during the initialization.
        event_loop.run_until_complete(shutdown())

    if ok:
        LOGGER.info("Final result: PASS !")
    else:
        LOGGER.error("Final result: FAIL !")
    return ok


def run_tests(
        test_class,
        matter_test_config,
        hooks: TestRunnerHooks,
        default_controller=None,
        external_stack=None) -> None:
    """
    Run Matter tests and exit the process with status code 1 on failure.

    This is a wrapper around run_tests_no_exit that exits the process
    if tests fail.

    Args:
        test_class: The test class to run
        matter_test_config: Configuration for Matter tests
        hooks: Test runner hooks for monitoring test progress
        default_controller: Optional pre-configured controller
        external_stack: Optional external Matter stack
    """
    with asyncio.Runner() as runner:
        if not run_tests_no_exit(
                test_class,
                matter_test_config,
                runner.get_loop(),
                hooks,
                default_controller,
                external_stack):
            sys.exit(1)


class AsyncMock(MagicMock):
    """
    Mock class for async methods that returns an awaitable.

    This is useful for testing async code without actual async execution.
    """

    async def __call__(self, *args, **kwargs):
        return super(AsyncMock, self).__call__(*args, **kwargs)


class MockTestRunner():
    """
    Test runner for mocking Matter device interactions.

    This class allows tests to run without actual device communication by
    mocking the controller's Read method and other interactions.
    """

    def __init__(self, abs_filename: str, classname: str, test: str, endpoint: Optional[int] = None,
                 pics: Optional[dict[str, bool]] = None, paa_trust_store_path=None):

        from matter.testing.matter_stack_state import MatterStackState
        from matter.testing.matter_test_config import MatterTestConfig

        self.kvs_storage = 'kvs_admin.json'

        self.config = MatterTestConfig(endpoint=endpoint, paa_trust_store_path=paa_trust_store_path,
                                       pics=pics or {}, storage_path=Path(self.kvs_storage))
        self.set_test(abs_filename, classname, test)

        self.set_test_config(self.config)

        self.stack = MatterStackState(self.config)
        self.default_controller = self.stack.certificate_authorities[0].adminList[0].NewController(
            nodeId=self.config.controller_node_id,
            paaTrustStorePath=str(self.config.paa_trust_store_path),
            catTags=self.config.controller_cat_tags
        )

    def set_test(self, abs_filename: str, classname: str, test: str):
        self.test = test
        self.config.tests = [self.test]

        try:
            filename_path = Path(abs_filename)
            module = importlib.import_module(filename_path.stem)
        except ModuleNotFoundError:
            sys.path.append(str(filename_path.parent.resolve()))
            module = importlib.import_module(filename_path.stem)

        self.test_class = getattr(module, classname)

    def set_test_config(self, test_config: Optional['MatterTestConfig'] = None):
        from matter.testing.matter_test_config import MatterTestConfig
        if test_config is None:
            test_config = MatterTestConfig()

        self.config = test_config
        self.config.tests = [self.test]
        self.config.storage_path = Path(self.kvs_storage)
        if not self.config.dut_node_ids:
            self.config.dut_node_ids = [1]

    def Shutdown(self):
        self.stack.Shutdown()

    def run_test_with_mock_read(self, read_cache: Attribute.AsyncReadTransaction.ReadResponse, hooks=None):
        self.default_controller.Read = AsyncMock(return_value=read_cache)
        # This doesn't need to do anything since we are overriding the read anyway
        self.default_controller.FindOrEstablishPASESession = AsyncMock(return_value=None)
        self.default_controller.GetConnectedDevice = AsyncMock(return_value=None)
        with asyncio.Runner() as runner:
            return run_tests_no_exit(self.test_class, self.config, runner.get_loop(),
                                     hooks, self.default_controller, self.stack)


# Argument parsing helper functions

def populate_commissioning_args(args: argparse.Namespace, config) -> bool:
    config.root_of_trust_index = args.root_index
    # Follow root of trust index if ID not provided to have same behavior as legacy
    # chip-tool that fabricID == commissioner_name == root of trust index
    config.fabric_id = args.fabric_id if args.fabric_id is not None else config.root_of_trust_index

    if args.chip_tool_credentials_path is not None and not args.chip_tool_credentials_path.exists():
        print("error: chip-tool credentials path %s doesn't exist!" % args.chip_tool_credentials_path)
        return False
    config.chip_tool_credentials_path = args.chip_tool_credentials_path

    if args.dut_node_ids is None:
        print("error: --dut-node-id is mandatory!")
        return False
    config.dut_node_ids = args.dut_node_ids

    config.commissioning_method = args.commissioning_method
    config.in_test_commissioning_method = args.in_test_commissioning_method
    config.commission_only = args.commission_only

    config.qr_code_content.extend(args.qr_code)
    config.manual_code.extend(args.manual_code)
    config.discriminators.extend(args.discriminators)
    config.setup_passcodes.extend(args.passcodes)

    if len(config.discriminators) != len(config.setup_passcodes):
        print("error: supplied number of discriminators does not match number of passcodes")
        return False

    device_descriptors = config.qr_code_content + config.manual_code + config.discriminators

    if not config.dut_node_ids:
        config.dut_node_ids = [TestingDefaults.DUT_NODE_ID]

    commissioning_method = args.in_test_commissioning_method or args.commissioning_method
    if not commissioning_method:
        return True

    # For NFC transport (when using the --commissioning-method argument), the NFC tag data is
    # read beforehand and commissioning data (QR code) is already populated from the tag.
    # Therefore, it does not need to be passed explicitly.
    #
    # However, during in-test commissioning, the user must manually read the NFC tag
    # (containing the commissioning credentials) within the main test body
    # and supply it later for commissioning with the DUT.
    #
    # For this reason, commissioning data validation is intentionally skipped in this scenario.

    if 'nfc' not in (args.in_test_commissioning_method or []):
        if len(config.dut_node_ids) > len(device_descriptors):
            print("error: More node IDs provided than discriminators")
            return False

        if len(config.dut_node_ids) < len(device_descriptors):
            # We generate new node IDs sequentially from the last one seen for all
            # missing NodeIDs when commissioning many nodes at once.
            missing = len(device_descriptors) - len(config.dut_node_ids)
            for i in range(missing):
                config.dut_node_ids.append(config.dut_node_ids[-1] + 1)

        if len(config.dut_node_ids) != len(set(config.dut_node_ids)):
            print("error: Duplicate values in node id list")
            return False

        if len(config.discriminators) != len(set(config.discriminators)):
            print("error: Duplicate value in discriminator list")
            return False

        if args.discriminators == [] and (args.qr_code == [] and args.manual_code == []):
            print("error: Missing --discriminator when no --qr-code/--manual-code present!")
            return False

        if args.passcodes == [] and (args.qr_code == [] and args.manual_code == []):
            print("error: Missing --passcode when no --qr-code/--manual-code present!")
            return False
    else:
        # For NFC in-test commissioning, we still need to ensure node IDs are unique if provided
        if len(config.dut_node_ids) != len(set(config.dut_node_ids)):
            print("error: Duplicate values in node id list")
            return False

    wifi_args = ['ble-wifi']
    thread_args = ['ble-thread', 'nfc-thread']
    if commissioning_method in wifi_args:
        if args.wifi_ssid is None:
            print("error: missing --wifi-ssid <SSID> for --commissioning-method "
                  "or --in-test-commissioning-method ble-wifi!")
            return False

        if args.wifi_passphrase is None:
            print("error: missing --wifi-passphrase <passphrase> for --commissioning-method or "
                  "--in-test-commissioning-method ble-wifi!")
            return False

        config.wifi_ssid = args.wifi_ssid
        config.wifi_passphrase = args.wifi_passphrase
    elif commissioning_method in thread_args:
        if args.thread_dataset_hex is None:
            print("error: missing --thread-dataset-hex <DATASET_HEX> for --commissioning-method or "
                  "--in-test-commissioning-method ble-thread or nfc-thread!")
            return False
        config.thread_operational_dataset = args.thread_dataset_hex
    elif config.commissioning_method == "on-network-ip":
        if args.ip_addr is None:
            print("error: missing --ip-addr <IP_ADDRESS> for --commissioning-method on-network-ip")
            return False
        config.commissionee_ip_address_just_for_testing = args.ip_addr

    if args.case_admin_subject is None:
        # Use controller node ID as CASE admin subject during commissioning if nothing provided
        config.case_admin_subject = config.controller_node_id
    else:
        # If a CASE admin subject is provided, then use that
        config.case_admin_subject = args.case_admin_subject

    return True


def convert_args_to_matter_config(args: argparse.Namespace):
    # Lazy import to avoid circular dependency
    from matter.testing.matter_test_config import MatterTestConfig

    config = MatterTestConfig()

    # Accumulate all command-line-passed named args
    all_global_args = []
    argsets = [item for item in (args.int_arg, args.float_arg, args.string_arg, args.json_arg,
                                 args.hex_arg, args.bool_arg) if item is not None]
    for argset in chain.from_iterable(argsets):
        all_global_args.extend(argset)

    config.global_test_params = {}
    for name, value in all_global_args:
        config.global_test_params[name] = value

    if "nfc" in (args.commissioning_method or []):

        if "NFC_Reader_index" not in config.global_test_params:
            LOGGER.error("Error: Missing required argument --int-arg NFC_Reader_index:<int-value> for "
                         "NFC commissioning tests")
            sys.exit(1)

        if any([args.passcodes, args.discriminators, args.manual_code, args.qr_code]):
            LOGGER.error("Error: Do not provide discriminator, passcode, manual code or qr-code for NFC commissioning. "
                         "The onboarding data is read directly from the NFC tag.")
            sys.exit(1)

        from matter.testing.nfc import NFCReader
        nfc_reader_index = config.global_test_params.get("NFC_Reader_index", 0)
        reader = NFCReader(nfc_reader_index)
        nfc_tag_data = reader.read_nfc_tag_data()
        args.qr_code.append(nfc_tag_data)

    # Populate commissioning config if present, exiting on error
    if not populate_commissioning_args(args, config):
        sys.exit(1)

    config.storage_path = pathlib.Path(TestingDefaults.STORAGE_PATH) if args.storage_path is None else args.storage_path
    config.logs_path = pathlib.Path(TestingDefaults.LOG_PATH) if args.logs_path is None else args.logs_path
    config.paa_trust_store_path = args.paa_trust_store_path
    config.ble_controller = args.ble_controller
    if args.PICS is None:
        config.pics = {}
    else:
        config.pics = read_pics_from_file(args.PICS)
    config.tests = list(chain.from_iterable(args.tests or []))
    config.timeout = args.timeout  # This can be none, we pull the default from the test if it's unspecified
    config.endpoint = args.endpoint  # This can be None, the get_endpoint function allows the tests to supply a default
    config.restart_flag_file = args.restart_flag_file
    config.debug = args.debug

    # Map CLI arg to the current config field name used by tests
    config.pipe_name = args.app_pipe
    if config.pipe_name is not None and not os.path.exists(config.pipe_name):
        # Named pipes are unique, so we MUST have consistent paths
        # Verify from start the named pipe exists.
        LOGGER.error("Named pipe %r does NOT exist" % config.pipe_name)
        raise FileNotFoundError("CANNOT FIND %r" % config.pipe_name)

    config.pipe_name_out = args.app_pipe_out
    if config.pipe_name_out is not None and not os.path.exists(config.pipe_name_out):
        LOGGER.error("Named pipe %r does NOT exist" % config.pipe_name_out)
        raise FileNotFoundError("CANNOT FIND %r" % config.pipe_name_out)

    config.fail_on_skipped_tests = args.fail_on_skipped

    config.legacy = args.use_legacy_test_event_triggers

    config.controller_node_id = args.controller_node_id
    config.trace_to = args.trace_to

    config.tc_version_to_simulate = args.tc_version_to_simulate
    config.tc_user_response_to_simulate = args.tc_user_response_to_simulate
    config.dac_revocation_set_path = args.dac_revocation_set_path

    # Embed the rest of the config in the global test params dict which will be passed to Mobly tests
    config.global_test_params["meta_config"] = {k: v for k, v in dataclass_asdict(config).items() if k != "global_test_params"}

    return config


def parse_matter_test_args(argv: Optional[List[str]] = None):
    parser = argparse.ArgumentParser(description='Matter standalone Python test')

    basic_group = parser.add_argument_group(title="Basic arguments", description="Overall test execution arguments")

    basic_group.add_argument('--tests', '--test-case', action='append', nargs='+', type=str, metavar='test_NAME',
                             help='A list of tests in the test class to execute.')
    basic_group.add_argument('--fail-on-skipped', action="store_true", default=False,
                             help="Fail the test if any test cases are skipped")
    basic_group.add_argument('--trace-to', nargs="*", default=[],
                             help="Where to trace (e.g perfetto, perfetto:path, json:log, json:path)")
    basic_group.add_argument('--storage-path', action="store", type=pathlib.Path,
                             metavar="PATH", help="Location for persisted storage of instance")
    basic_group.add_argument('--logs-path', action="store", type=pathlib.Path, metavar="PATH", help="Location for test logs")
    paa_path_default = get_default_paa_trust_store(pathlib.Path.cwd())
    basic_group.add_argument('--paa-trust-store-path', action="store", type=pathlib.Path, metavar="PATH", default=paa_path_default,
                             help="PAA trust store path (default: %s)" % str(paa_path_default))
    basic_group.add_argument('--dac-revocation-set-path', action="store", type=pathlib.Path, metavar="PATH",
                             help="Path to JSON file containing the device attestation revocation set.")
    basic_group.add_argument('--ble-controller', action="store", type=int,
                             metavar="CONTROLLER_ID", help="BLE controller selector, see example or platform docs for details")
    basic_group.add_argument('-N', '--controller-node-id', type=int_decimal_or_hex,
                             metavar='NODE_ID',
                             default=TestingDefaults.CONTROLLER_NODE_ID,
                             help='NodeID to use for initial/default controller (default: %d)' % TestingDefaults.CONTROLLER_NODE_ID)
    basic_group.add_argument('-n', '--dut-node-id', '--nodeId', type=int_decimal_or_hex,
                             metavar='NODE_ID', dest='dut_node_ids', default=[],
                             help='Node ID for primary DUT communication, '
                             'and NodeID to assign if commissioning (default: %d)' % TestingDefaults.DUT_NODE_ID, nargs="+")
    basic_group.add_argument('--endpoint', type=int, default=None, help="Endpoint under test")
    basic_group.add_argument('--app-pipe', type=str, default=None,
                             help="The full path of the app to send an out-of-band command from test to app")
    basic_group.add_argument('--app-pipe-out', type=str, default=None,
                             help="The full path of the app to read an out-of-band command from app to test")
    basic_group.add_argument('--restart-flag-file', type=str, default=None,
                             help="The full path of the file to use to signal a restart to the app")
    basic_group.add_argument('--debug', action="store_true", default=False,
                             help="Run the script in debug mode. This is needed to capture attribute dump at end of test modules if there are problems found during testing.")
    basic_group.add_argument('--timeout', type=int, help="Test timeout in seconds")
    basic_group.add_argument("--PICS", help="PICS file path", type=str)

    basic_group.add_argument("--use-legacy-test-event-triggers", action="store_true", default=False,
                             help="Send test event triggers with endpoint 0 for older devices")

    commission_group = parser.add_argument_group(title="Commissioning", description="Arguments to commission a node")

    commission_group.add_argument('-m', '--commissioning-method', type=str,
                                  metavar='METHOD_NAME',
                                  choices=["on-network", "ble-wifi", "ble-thread", "nfc-thread"],
                                  help='Name of commissioning method to use')
    commission_group.add_argument('--in-test-commissioning-method', type=str,
                                  metavar='METHOD_NAME',
                                  choices=["on-network", "ble-wifi", "ble-thread", "nfc-thread"],
                                  help='Name of commissioning method to use, for commissioning tests')
    commission_group.add_argument('-d', '--discriminator', type=int_decimal_or_hex,
                                  metavar='LONG_DISCRIMINATOR',
                                  dest='discriminators',
                                  default=[],
                                  help='Discriminator to use for commissioning', nargs="+")
    commission_group.add_argument('-p', '--passcode', type=int_decimal_or_hex,
                                  metavar='PASSCODE',
                                  dest='passcodes',
                                  default=[],
                                  help='PAKE passcode to use', nargs="+")

    commission_group.add_argument('--wifi-ssid', type=str,
                                  metavar='SSID',
                                  help='Wi-Fi SSID for ble-wifi commissioning')
    commission_group.add_argument('--wifi-passphrase', type=str,
                                  metavar='PASSPHRASE',
                                  help='Wi-Fi passphrase for ble-wifi commissioning')

    commission_group.add_argument('--thread-dataset-hex', type=byte_string_from_hex,
                                  metavar='OPERATIONAL_DATASET_HEX',
                                  help='Thread operational dataset as a hex string for ble-thread commissioning')

    commission_group.add_argument('--admin-vendor-id', action="store", type=int_decimal_or_hex, default=TestingDefaults.ADMIN_VENDOR_ID,
                                  metavar="VENDOR_ID",
                                  help="VendorID to use during commissioning (default 0x%04X)" % TestingDefaults.ADMIN_VENDOR_ID)
    commission_group.add_argument('--case-admin-subject', action="store", type=int_decimal_or_hex,
                                  metavar="CASE_ADMIN_SUBJECT",
                                  help="Set the CASE admin subject to an explicit value (default to commissioner Node ID)")

    commission_group.add_argument('--commission-only', action="store_true", default=False,
                                  help="If true, test exits after commissioning without running subsequent tests")

    commission_group.add_argument('--tc-version-to-simulate', type=int, help="Terms and conditions version")

    commission_group.add_argument('--tc-user-response-to-simulate', type=int, help="Terms and conditions acknowledgements")

    code_group = parser.add_argument_group(title="Setup codes")

    code_group.add_argument('-q', '--qr-code', type=str,
                            metavar="QR_CODE", default=[], help="QR setup code content (overrides passcode and discriminator)", nargs="+")
    code_group.add_argument('--manual-code', type=str_from_manual_code,
                            metavar="MANUAL_CODE", default=[], help="Manual setup code content (overrides passcode and discriminator)", nargs="+")

    fabric_group = parser.add_argument_group(
        title="Fabric selection", description="Fabric selection for single-fabric basic usage, and commissioning")
    fabric_group.add_argument('-f', '--fabric-id', type=int_decimal_or_hex,
                              metavar='FABRIC_ID',
                              help='Fabric ID on which to operate under the root of trust')

    fabric_group.add_argument('-r', '--root-index', type=root_index,
                              metavar='ROOT_INDEX_OR_NAME', default=TestingDefaults.TRUST_ROOT_INDEX,
                              help='Root of trust under which to operate/commission for single-fabric basic usage. '
                              'alpha/beta/gamma are aliases for 1/2/3. Default (%d)' % TestingDefaults.TRUST_ROOT_INDEX)

    fabric_group.add_argument('-c', '--chip-tool-credentials-path', type=pathlib.Path,
                              metavar='PATH',
                              help='Path to chip-tool credentials file root')

    args_group = parser.add_argument_group(title="Config arguments", description="Test configuration global arguments set")
    args_group.add_argument('--int-arg', nargs='+', action='append', type=int_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for an integer as hex or decimal (e.g. -2 or 0xFFFF_1234)")
    args_group.add_argument('--bool-arg', nargs='+', action='append', type=bool_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for an boolean value (e.g. true/false or 0/1)")
    args_group.add_argument('--float-arg', nargs='+', action='append', type=float_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for a floating point value (e.g. -2.1 or 6.022e23)")
    args_group.add_argument('--string-arg', nargs='+', action='append', type=str_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for a string value")
    args_group.add_argument('--json-arg', nargs='+', action='append', type=json_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for JSON stored as a list or dict")
    args_group.add_argument('--hex-arg', nargs='+', action='append', type=bytes_as_hex_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for an octet string in hex (e.g. 0011cafe or 00:11:CA:FE)")

    if not argv:
        argv = sys.argv[1:]

    return convert_args_to_matter_config(parser.parse_args(argv))


def int_decimal_or_hex(s: str) -> int:
    val = int(s, 0)
    if val < 0:
        raise ValueError("Negative values not supported")
    return val


def byte_string_from_hex(s: str) -> bytes:
    return unhexlify(s.replace(":", "").replace(" ", "").replace("0x", ""))


def str_from_manual_code(s: str) -> str:
    """Enforces legal format for manual codes and removes spaces/dashes."""
    s = s.replace("-", "").replace(" ", "")
    regex = r"^([0-9]{11}|[0-9]{21})$"
    match = re.match(regex, s)
    if not match:
        raise ValueError("Invalid manual code format, does not match %s" % regex)

    return s


def int_named_arg(s: str) -> Tuple[str, int]:
    regex = r"^(?P<name>[a-zA-Z_0-9_.-]+):((?P<hex_value>0x[0-9a-fA-F_]+)|(?P<decimal_value>-?\d+))$"
    match = re.match(regex, s)
    if not match:
        raise ValueError("Invalid int argument format, does not match %s" % regex)

    name = match.group("name")
    if match.group("hex_value"):
        value = int(match.group("hex_value"), 0)
    else:
        value = int(match.group("decimal_value"), 10)
    return (name, value)


def str_named_arg(s: str) -> Tuple[str, str]:
    regex = r"^(?P<name>[a-zA-Z_0-9.]+):(?P<value>.*)$"
    match = re.match(regex, s)
    if not match:
        raise ValueError("Invalid string argument format, does not match %s" % regex)

    return (match.group("name"), match.group("value"))


def float_named_arg(s: str) -> Tuple[str, float]:
    regex = r"^(?P<name>[a-zA-Z_0-9.]+):(?P<value>.*)$"
    match = re.match(regex, s)
    if not match:
        raise ValueError("Invalid float argument format, does not match %s" % regex)

    name = match.group("name")
    value = float(match.group("value"))

    return (name, value)


def json_named_arg(s: str) -> Tuple[str, object]:
    regex = r"^(?P<name>[a-zA-Z_0-9.]+):(?P<value>.*)$"
    match = re.match(regex, s)
    if not match:
        raise ValueError("Invalid JSON argument format, does not match %s" % regex)

    name = match.group("name")
    value = json.loads(match.group("value"))

    return (name, value)


def bool_named_arg(s: str) -> Tuple[str, bool]:
    regex = r"^(?P<name>[a-zA-Z_0-9.]+):((?P<truth_value>true|false)|(?P<decimal_value>[01]))$"
    match = re.match(regex, s, re.IGNORECASE)
    if not match:
        raise ValueError("Invalid bool argument format, does not match %s" % regex)

    name = match.group("name")
    if match.group("truth_value"):
        value = match.group("truth_value").lower() == "true"
    else:
        value = int(match.group("decimal_value")) != 0

    return (name, value)


def bytes_as_hex_named_arg(s: str) -> Tuple[str, bytes]:
    regex = r"^(?P<name>[a-zA-Z_0-9.]+):(?P<value>[0-9a-fA-F:]+)$"
    match = re.match(regex, s)
    if not match:
        raise ValueError("Invalid bytes as hex argument format, does not match %s" % regex)

    name = match.group("name")
    value_str = match.group("value")
    value_str = value_str.replace(":", "")
    if len(value_str) % 2 != 0:
        raise ValueError("Byte string argument value needs to be event number of hex chars")
    value = unhexlify(value_str)

    return (name, value)


def root_index(s: str) -> int:
    CHIP_TOOL_COMPATIBILITY = {
        "alpha": 1,
        "beta": 2,
        "gamma": 3
    }

    for name, id in CHIP_TOOL_COMPATIBILITY.items():
        if s.lower() == name:
            return id
    else:
        root_index = int(s)
        if root_index == 0:
            raise ValueError("Only support root index >= 1")
        return root_index

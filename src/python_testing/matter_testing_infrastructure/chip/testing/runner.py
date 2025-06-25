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

import asyncio
import importlib
import logging
import os
import sys
import typing
from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
from pathlib import Path
from typing import Optional
from unittest.mock import MagicMock

import chip.testing.global_stash as global_stash
from chip.clusters import Attribute
from mobly import signals
from mobly.config_parser import ENV_MOBLY_LOGPATH, TestRunConfig
from mobly.test_runner import TestRunner

try:
    from matter_yamltests.hooks import TestRunnerHooks
except ImportError:
    class TestRunnerHooks:
        pass
try:
    from chip.tracing import TracingContext
except ImportError:
    class TracingContext:
        def __enter__(self):
            return self

        def __exit__(self, *args):
            pass

        def StartFromString(self, destination):
            pass

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from chip.testing.matter_testing import MatterTestConfig

_DEFAULT_LOG_PATH = "/tmp/matter_testing/logs"


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
        logging.info(f'Starting test set, running {count} tests')

    def stop(self, duration: int):
        """
        Called when the test runner finishes a test set.

        Args:
            duration: The duration of the test set in milliseconds.
        """
        logging.info(f'Finished test set, ran for {duration}ms')

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
        logging.info(f'Starting test from {filename}: {name} - {count} steps')

    def test_stop(self, exception: Exception, duration: int):
        """
        Called when an individual test completes.

        Args:
            exception: Exception raised during test execution, or None if successful
            duration: Test execution duration in milliseconds
        """
        logging.info(f'Finished test in {duration}ms')

    def step_skipped(self, name: str, expression: str):
        """
        Called when a test step is skipped.

        Args:
            name: Name of the skipped step
            expression: Condition expression that caused the skip
        """
        # TODO: Do we really need the expression as a string? We can evaluate
        # this in code very easily
        logging.info(f'\t\t**** Skipping: {name}')

    def step_start(self, name: str):
        """
        Called when a test step starts.

        Args:
            name: Name of the step including its number
        """
        # The way I'm calling this, the name is already includes the step
        # number, but it seems like it might be good to separate these
        logging.info(f'\t\t***** Test Step {name}')

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
        logging.info('\t\t***** Test Failure : ')
        if received is not None:
            logging.info(f'\t\t      Received: {received}')
        if request is not None:
            logging.info(f'\t\t      Expected: {request}')

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
        logging.info(f"Skipping test from {filename}: {name}")


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
    log_path = _DEFAULT_LOG_PATH if log_path is None else log_path
    if ENV_MOBLY_LOGPATH in os.environ:
        log_path = os.environ[ENV_MOBLY_LOGPATH]

    test_run_config.log_path = log_path
    # TODO: For later, configure controllers
    test_run_config.controller_configs = {}

    test_run_config.user_params = matter_test_config.global_test_params

    return test_run_config


def default_matter_test_main():
    """Execute the test class in a test module.
    This is the default entry point for running a test script file directly.
    In this case, only one test class in a test script is allowed.
    To make your test script executable, add the following to your file:
    .. code-block:: python
      from chip.testing.matter_testing import default_matter_test_main
      ...
      if __name__ == '__main__':
        default_matter_test_main()
    """

    from chip.testing.matter_testing import _find_test_class, parse_matter_test_args

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

    from chip.testing.matter_testing import MatterStackState
    if TYPE_CHECKING:
        from chip.testing.commissioning import CommissionDeviceTest
    else:
        CommissionDeviceTest = None  # Initial placeholder

    # Actual runtime import
    if CommissionDeviceTest is None:
        from chip.testing.commissioning import CommissionDeviceTest

    # NOTE: It's not possible to pass event loop via Mobly TestRunConfig user params, because the
    #       Mobly deep copies the user params before passing them to the test class and the event
    # loop is not serializable. So, we are setting the event loop as a test
    # class member.
    CommissionDeviceTest.event_loop = event_loop
    test_class.event_loop = event_loop

    get_test_info(test_class, matter_test_config)

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
                logging.exception('Exception when executing %s.',
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
        logging.info("Final result: PASS !")
    else:
        logging.error("Final result: FAIL !")
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

    def __init__(self, abs_filename: str, classname: str, test: str, endpoint: int = None,
                 pics: dict[str, bool] = None, paa_trust_store_path=None):

        from chip.testing.matter_testing import MatterStackState, MatterTestConfig

        self.kvs_storage = 'kvs_admin.json'
        self.config = MatterTestConfig(endpoint=endpoint, paa_trust_store_path=paa_trust_store_path,
                                       pics=pics, storage_path=self.kvs_storage)
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

    def set_test_config(self, test_config: 'MatterTestConfig' = None):
        from chip.testing.matter_testing import MatterTestConfig
        if test_config is None:
            test_config = MatterTestConfig()

        self.config = test_config
        self.config.tests = [self.test]
        self.config.storage_path = self.kvs_storage
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

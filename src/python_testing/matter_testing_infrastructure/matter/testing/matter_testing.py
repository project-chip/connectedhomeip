#
#    Copyright (c) 2022-2025 Project CHIP Authors
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
import inspect
import json
import logging
import os
import queue
import random
import shlex
import textwrap
import time
import typing
from dataclasses import dataclass
from datetime import datetime, timedelta, timezone
from enum import IntFlag
from typing import Any, Callable, List, Optional, Type, Union

import matter.testing.conversions as conversions
import matter.testing.decorators as decorators
import matter.testing.matchers as matchers
import matter.testing.runner as runner
import matter.testing.timeoperations as timeoperations

# isort: off

from matter import ChipDeviceCtrl  # Needed before matter.FabricAdmin
import matter.FabricAdmin  # Needed before matter.CertificateAuthority
import matter.CertificateAuthority

# isort: on

from mobly import asserts, base_test, signals

import matter.clusters as Clusters
import matter.logging
import matter.native
import matter.testing.global_stash as global_stash
from matter.clusters import Attribute, ClusterObjects
from matter.interaction_model import InteractionModelError, Status
from matter.setup_payload import SetupPayload
from matter.testing.commissioning import (CommissioningInfo, CustomCommissioningParameters, SetupPayloadInfo, commission_devices,
                                          get_setup_payload_info_config)
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.matter_stack_state import MatterStackState
from matter.testing.matter_test_config import MatterTestConfig
from matter.testing.problem_notices import AttributePathLocation, ClusterMapper, ProblemLocation, ProblemNotice, ProblemSeverity
from matter.testing.runner import TestRunnerHooks, TestStep
from matter.tlv import uint

# TODO: Add utility to commission a device if needed
# TODO: Add utilities to keep track of controllers/fabrics

# Type aliases for common patterns to improve readability
StepNumber = Union[int, str]  # Test step numbers can be integers or strings
OptionalTimeout = Optional[int]  # Optional timeout values

logger = logging.getLogger("matter.python_testing")
logger.setLevel(logging.INFO)

DiscoveryFilterType = ChipDeviceCtrl.DiscoveryFilterType


class TestError(Exception):
    pass


def clear_queue(report_queue: queue.Queue):
    """Flush all contents of a report queue. Useful to get back to empty point."""
    while not report_queue.empty():
        try:
            report_queue.get(block=False)
        except queue.Empty:
            break


@dataclass
class AttributeValue:
    endpoint_id: int
    attribute: ClusterObjects.ClusterAttributeDescriptor
    value: Any
    timestamp_utc: Optional[datetime] = None


class AttributeMatcher:
    """Matcher for a self-described AttributeValue matcher, to be used in `await_all_expected_report_matches` methods.

    This class embodies a predicate for a condition that must be matched by an attribute report.

    A match is considered as having occurred when the `matches` method returns True for an `AttributeValue` report.
    """

    def __init__(self, description: str):
        self._description: str = description

    def matches(self, report: AttributeValue) -> bool:
        """Implementers must override this method to return True when an attribute value matches.

        The condition matched should be clearly expressed by the `description` property.
        """
        return False

    @property
    def description(self):
        return self._description

    @staticmethod
    def from_callable(description: str, matcher: Callable[[AttributeValue], bool]) -> "AttributeMatcher":
        """Take a single callable and wrap it into an AttributeMatcher object. Useful to wrap closures."""
        class AttributeMatcherFromCallable(AttributeMatcher):
            def __init__(self, description, matcher: Callable[[AttributeValue], bool]):
                super().__init__(description)
                self._matcher = matcher

            def matches(self, report: AttributeValue) -> bool:
                return self._matcher(report)

        return AttributeMatcherFromCallable(description, matcher)


class SetupParameters:
    passcode: int
    vendor_id: int = 0xFFF1
    product_id: int = 0x8001
    discriminator: int = 3840
    custom_flow: int = 0
    capabilities: int = 0b0100
    version: int = 0

    @property
    def qr_code(self):
        return SetupPayload().GenerateQrCode(self.passcode, self.vendor_id, self.product_id, self.discriminator,
                                             self.custom_flow, self.capabilities, self.version)

    @property
    def manual_code(self):
        return SetupPayload().GenerateManualPairingCode(self.passcode, self.vendor_id, self.product_id, self.discriminator,
                                                        self.custom_flow, self.capabilities, self.version)


class MatterBaseTest(base_test.BaseTestClass):
    def __init__(self, *args):
        super().__init__(*args)

        # List of accumulated problems across all tests
        self.problems = []
        self.is_commissioning = False
        self.cached_steps: dict[str, list[TestStep]] = {}

    #
    # Mobly Test Controller Methods (Framework Interface)
    #
    # The test framework defines a set of named methods that can be used to set up or tear down tests.
    # setup_class is called once after class initialization, once per class, before any test_ methods are run.
    # setup_test is called once before each test_ function is run
    # teardown_test is called once after each test_
    # teardown_class is called after the last test_ function in the class is run
    #
    # Test authors may overwrite these methods to assist in performing setup and tear down.
    # Test classes that overwrite these functions should ensure the base functions are called as appropriate.
    # setup_ methods  should call the super() method at the start
    # teardown_ methods should call the super() method at the end
    #

    def setup_class(self):
        """Set up the test class before running any tests.

        Initializes cluster mapping, step tracking, and global test state.
        Called once per test class by the Mobly framework.

        Test authors may overwrite this method in the derived class to perform setup that is common for all tests.
        This function is called only once for the class. To perform setup before each test, use setup_test.
        Test authors that implement steps in this function need to be careful of step handling if there is
        more than one test in the class.
        Test authors that implement this method should ensure super().setup_class() is called before any
        custom setup.

        """
        super().setup_class()

        # Mappings of cluster IDs to names and metadata.
        # TODO: Move to using non-generated code and rather use data model description (.matter or .xml)
        self.cluster_mapper = ClusterMapper(self.default_controller._Cluster)
        self.current_step_index = 0
        self.step_start_time = datetime.now(timezone.utc)
        self.step_skipped = False
        # self.stored_global_wildcard stores value of self.global_wildcard after first async call.
        # Because setup_class can be called before commissioning, this variable is lazy-initialized
        # where the read is deferred until the first guard function call that requires global attributes.
        self.stored_global_wildcard = None

    def teardown_class(self):
        """Final teardown after all tests: log all problems.
            Test authors may overwrite this method in the derived class to perform teardown that is common for all tests
             This function is called only once per class. To perform teardown after each test, use teardown_test.
             Test authors that implement steps in this function need to be careful of step handling if there is
             more than one test in the class.
             Test authors that implement this method should ensure super().teardown_class() is called after any
             custom teardown code.

        """
        if len(self.problems) > 0:
            logging.info("###########################################################")
            logging.info("Problems found:")
            logging.info("===============")
            for problem in self.problems:
                logging.info(str(problem))
            logging.info("###########################################################")
        super().teardown_class()

    def setup_test(self):
        """Set up for each individual test execution.

        Resets test state, starts timers, and notifies runner hooks.
        Called before each test method by the Mobly framework.

        Test authors may overwrite this method in the derived class to perform setup that is common for all tests.
        This is called once before each test_ in the class.

        Test authors that implement this method should ensure super().setup_test() is called before any custom setup.
        """
        self.current_step_index = 0
        self.test_start_time = datetime.now(timezone.utc)
        self.step_start_time = datetime.now(timezone.utc)
        self.step_skipped = False
        self.failed = False
        if self.runner_hook and not self.is_commissioning:
            test_name = self.current_test_info.name
            steps = self.get_defined_test_steps(test_name)
            num_steps = 1 if steps is None else len(steps)
            filename = inspect.getfile(self.__class__)
            desc = self.get_test_desc(test_name)
            steps_descriptions = [] if steps is None else [step.description for step in steps]
            self.runner_hook.test_start(filename=filename, name=desc, count=num_steps, steps=steps_descriptions)
            # If we don't have defined steps, we're going to start the one and only step now
            # if there are steps defined by the test, rely on the test calling the step() function
            # to indicates how it is proceeding
            if steps is None:
                self.step(1)

    def on_fail(self, record):
        """Handle test failure callback from Mobly framework.

            This is called by the base framework.
            Tests should not call this directly.
            Tests should not overwrite this method.

        Args:
            record: TestResultRecord containing failure information.
        """
        self.failed = True
        if self.runner_hook and not self.is_commissioning:
            exception = record.termination_signal.exception

            try:
                step_duration = (datetime.now(timezone.utc) - self.step_start_time) / timedelta(microseconds=1)
            except AttributeError:
                # If we failed during setup, these may not be populated
                step_duration = 0
            try:
                test_duration = (datetime.now(timezone.utc) - self.test_start_time) / timedelta(microseconds=1)
            except AttributeError:
                test_duration = 0
            # TODO: I have no idea what logger, logs, request or received are. Hope None works because I have nothing to give
            self.runner_hook.step_failure(logger=None, logs=None, duration=step_duration, request=None, received=None)
            self.runner_hook.test_stop(exception=exception, duration=test_duration)

            def extract_error_text() -> tuple[str, str]:
                """Extract meaningful error information from test failure stack traces.

                This function parses stack trace information to identify the most relevant
                error line and associated file location for test failure reporting.

                Returns:
                    tuple[str, str]: A tuple containing:
                        - probable_error (str): The most likely line containing the actual error.
                            For Mobly framework exceptions (TestError/TestFailure), this finds
                            the last assertion line. For other exceptions, uses the last line
                            of the stack trace.
                        - probable_file (str): The file path where the error occurred,
                            extracted from the stack trace "File" markers.

                Note:
                    - Returns ("Stack Trace Unavailable", "") if no stack trace is available
                    - Returns ("Unknown error, please see stack trace above", "") if no
                        assertion candidates are found for Mobly exceptions
                    - Returns (probable_error, "Unknown file") if no file information
                        can be extracted from the stack trace
                """
                no_stack_trace = ("Stack Trace Unavailable", "")
                if not record.termination_signal.stacktrace:
                    return no_stack_trace
                trace = record.termination_signal.stacktrace.splitlines()
                if not trace:
                    return no_stack_trace

                if isinstance(exception, signals.TestError) or isinstance(exception, signals.TestFailure):
                    # Exception gets raised by the mobly framework, so the proximal error is one line back in the stack trace
                    assert_candidates = [idx for idx, line in enumerate(trace) if "asserts" in line and "asserts.py" not in line]
                    if not assert_candidates:
                        return "Unknown error, please see stack trace above", ""
                    assert_candidate_idx = assert_candidates[-1]
                else:
                    # Normal assert is on the Last line
                    assert_candidate_idx = -1
                probable_error = trace[assert_candidate_idx]

                # Find the file marker immediately above the probable error
                file_candidates = [idx for idx, line in enumerate(trace[:assert_candidate_idx]) if "File" in line]
                if not file_candidates:
                    return probable_error, "Unknown file"
                return probable_error.strip(), trace[file_candidates[-1]].strip()

            probable_error, probable_file = extract_error_text()
            test_steps = self.get_defined_test_steps(self.current_test_info.name)
            test_step = str(test_steps[self.current_step_index-1]
                            ) if test_steps is not None else 'UNKNOWN - no test steps provided in test script'
            logging.error(textwrap.dedent(f"""

                                          ******************************************************************
                                          *
                                          * Test {self.current_test_info.name} failed for the following reason:
                                          * {exception}
                                          *
                                          * {probable_file}
                                          * {probable_error}
                                          *
                                          * Test step:
                                          *     {test_step}
                                          *
                                          * Endpoint: {self.matter_test_config.endpoint}
                                          *
                                          *******************************************************************
                                          """))

    def on_pass(self, record):
        """Handle test success callback from Mobly framework.

            This is called by the base framework.
            Tests should not call this directly.
            Tests should not overwrite this method.

        Args:
            record: TestResultRecord containing test results.
        """
        if self.runner_hook and not self.is_commissioning:
            # What is request? This seems like an implementation detail for the runner
            # TODO: As with failure, I have no idea what logger, logs or request are meant to be
            step_duration = (datetime.now(timezone.utc) - self.step_start_time) / timedelta(microseconds=1)
            test_duration = (datetime.now(timezone.utc) - self.test_start_time) / timedelta(microseconds=1)
            self.runner_hook.step_success(logger=None, logs=None, duration=step_duration, request=None)

        # TODO: this check could easily be annoying when doing dev. flag it somehow? Ditto with the in-order check
        steps = self.get_defined_test_steps(record.test_name)
        if steps is None:
            # if we don't have a list of steps, assume they were all run
            all_steps_run = True
        else:
            all_steps_run = len(steps) == self.current_step_index

        if not all_steps_run:
            # The test is done, but we didn't execute all the steps
            asserts.fail("Test script error: Not all required steps were run")

        if self.runner_hook and not self.is_commissioning:
            self.runner_hook.test_stop(exception=None, duration=test_duration)

    def on_skip(self, record):
        """Handle test skip callback from Mobly framework.

            This is called by the base framework.
            Tests should not call this directly.
            Tests should not overwrite this method.

        Args:
            record: TestResultRecord containing skip information.
        """
        if self.runner_hook and not self.is_commissioning:
            test_duration = (datetime.now(timezone.utc) - self.test_start_time) / timedelta(microseconds=1)
            test_name = self.current_test_info.name
            filename = inspect.getfile(self.__class__)
            self.runner_hook.test_skipped(filename, test_name)
            self.runner_hook.test_stop(exception=None, duration=test_duration)

    #
    # Matter Test API - Core Properties
    #

    # Override this if the test requires a different default timeout.
    # This value will be overridden if a timeout is supplied on the command line.
    @property
    def default_timeout(self) -> int:
        """The default timeout in seconds for async operations in a test."""
        return 90

    @property
    def runner_hook(self) -> TestRunnerHooks:
        """Accesses the Test Runner Hooks for external reporting."""
        return global_stash.unstash_globally(self.user_params.get("hooks"))

    @property
    def matter_test_config(self) -> MatterTestConfig:
        """Accesses the global Matter test configuration object."""
        return global_stash.unstash_globally(self.user_params.get("matter_test_config"))

    @property
    def default_controller(self) -> ChipDeviceCtrl.ChipDeviceController:
        """Accesses the default device controller instance for the test."""
        return global_stash.unstash_globally(self.user_params.get("default_controller"))

    @property
    def matter_stack(self) -> MatterStackState:
        """Accesses the Matter stack state object."""
        return global_stash.unstash_globally(self.user_params.get("matter_stack"))

    @property
    def certificate_authority_manager(self) -> matter.CertificateAuthority.CertificateAuthorityManager:
        """Accesses the Certificate Authority Manager."""
        return global_stash.unstash_globally(self.user_params.get("certificate_authority_manager"))

    @property
    def dut_node_id(self) -> int:
        """Returns the primary DUT (Device Under Test) node ID."""
        return self.matter_test_config.dut_node_ids[0]

    @property
    def is_pics_sdk_ci_only(self) -> bool:
        """Checks if the 'PICS_SDK_CI_ONLY' PICS flag is enabled."""
        return self.check_pics('PICS_SDK_CI_ONLY')

    #
    # Matter Test API - Parameter Getters
    #

    def get_endpoint(self, default: Optional[int] = 0) -> int:
        """Gets the target endpoint ID from config, with a fallback default."""
        endpoint = self.matter_test_config.endpoint
        if endpoint is not None:
            return endpoint
        return 0 if default is None else default

    def get_wifi_ssid(self, default: str = "") -> str:
        ''' Get WiFi SSID

            Get the WiFi networks name provided with flags

        '''
        return self.matter_test_config.wifi_ssid if self.matter_test_config.wifi_ssid is not None else default

    def get_credentials(self, default: str = "") -> str:
        ''' Get WiFi passphrase

            Get the WiFi credentials provided with flags

        '''
        return self.matter_test_config.wifi_passphrase if self.matter_test_config.wifi_passphrase is not None else default

    def get_setup_payload_info(self) -> List[SetupPayloadInfo]:
        """
        Get and builds the payload info provided in the execution.
        Returns:
            List[SetupPayloadInfo]: List of Payload used by the test case
        """
        return get_setup_payload_info_config(self.matter_test_config)

    #
    # Matter Test API - Test Definition Helpers (Steps, PICS, Description)
    #
    #  These helper methods are used by the test harness and should not be called tests.

    def get_test_steps(self, test: str) -> list[TestStep]:
        ''' Retrieves the test step list for the given test

            Test steps are defined in the function called steps_<functionname>.
            ex for test test_TC_TEST_1_1, the steps are in a function called
            steps_TC_TEST_1_1.

            Test that implement a steps_ function should call each step
            in order using self.step(number), where number is the test_plan_number
            from each TestStep.
        '''
        steps = self.get_defined_test_steps(test)
        return [TestStep(1, "Run entire test")] if steps is None else steps

    def get_defined_test_steps(self, test: str) -> Optional[list[TestStep]]:
        """Retrieves test steps from a 'steps_*' function, using a cache."""
        steps_name = f'steps_{test.removeprefix("test_")}'
        if test in self.cached_steps:
            return self.cached_steps[test]

        try:
            fn = getattr(self, steps_name)
            steps = fn()
            self.cached_steps[test] = steps
            return fn()
        except AttributeError:
            return None

    def get_test_pics(self, test: str) -> list[str]:
        ''' Retrieves a list of top-level PICS that should be checked before running this test

            An empty list means the test will always be run.

            PICS are defined in a function called pics_<functionname>.
            ex. for test test_TC_TEST_1_1, the pics are in a function called
            pics_TC_TEST_1_1.
        '''
        pics = self._get_defined_pics(test)
        return [] if pics is None else pics

    def _get_defined_pics(self, test: str) -> Optional[list[str]]:
        """Retrieve PICS list from a 'pics_*' function if it exists.

        Args:
            test: Name of the test to get PICS for.

        Returns:
            List of PICS strings if pics function exists, None otherwise.
        """
        steps_name = f'pics_{test.removeprefix("test_")}'
        try:
            fn = getattr(self, steps_name)
            return fn()
        except AttributeError:
            return None

    def get_test_desc(self, test: str) -> str:
        ''' Returns a description of this test

            Test description is defined in the function called desc_<functionname>.
            ex for test test_TC_TEST_1_1, the steps are in a function called
            desc_TC_TEST_1_1.

            Format:
            <Test plan reference> [<test plan number>] <test plan name>

            ex:
            133.1.1. [TC-ACL-1.1] Global attributes
        '''
        desc_name = f'desc_{test.removeprefix("test_")}'
        try:
            fn = getattr(self, desc_name)
            return fn()
        except AttributeError:
            return test

    #
    # Matter Test API - Step Management & Execution
    #
    # These methods are used to mark test progress for the test harness and logs, to help with test
    # debugging, issue creation and log analysis by the test labs.

    def step(self, step: typing.Union[int, str]):
        """Execute a test step and manage step progression.

        Validates step order, prints step information, and notifies runner hooks.

        Args:
            step: The step number or identifier to execute.

        Raises:
            AssertionError: If steps are called out of order or step doesn't exist.
        """
        test_name = self.current_test_info.name
        steps = self.get_test_steps(test_name)

        # TODO: this might be annoying during dev. Remove? Flag?
        if len(steps) <= self.current_step_index or steps[self.current_step_index].test_plan_number != step:
            asserts.fail(f'Unexpected test step: {step} - steps not called in order, or step does not exist')

        current_step = steps[self.current_step_index]
        self.print_step(step, current_step.description)

        if self.runner_hook:
            # If we've reached the next step with no assertion and the step wasn't skipped, it passed
            if not self.step_skipped and self.current_step_index != 0:
                # TODO: As with failure, I have no idea what loger, logs or request are meant to be
                step_duration = (datetime.now(timezone.utc) - self.step_start_time) / timedelta(microseconds=1)
                self.runner_hook.step_success(logger=None, logs=None, duration=step_duration, request=None)

            # TODO: it seems like the step start should take a number and a name
            name = f'{step} : {current_step.description}'
            self.runner_hook.step_start(name=name)

        self.step_start_time = datetime.now(tz=timezone.utc)
        self.current_step_index = self.current_step_index + 1
        self.step_skipped = False

    def print_step(self, stepnum: typing.Union[int, str], title: str) -> None:
        """Print test step information to logs.

        Args:
            stepnum: The step number or identifier.
            title: The descriptive title of the step.
        """
        logging.info(f'***** Test Step {stepnum} : {title}')

    def skip_step(self, step):
        """Execute and immediately mark a step as skipped.

        Args:
            step: The step number or identifier to skip.
        """
        self.step(step)
        self.mark_current_step_skipped()

    def mark_current_step_skipped(self):
        """Mark the current step as skipped and log the skip."""
        try:
            steps = self.get_test_steps(self.current_test_info.name)
            if self.current_step_index == 0:
                asserts.fail("Script error: mark_current_step_skipped cannot be called before step()")
            num = steps[self.current_step_index - 1].test_plan_number
        except KeyError:
            num = self.current_step_index

        if self.runner_hook:
            # TODO: what does name represent here? The wordy test name? The test plan number? The number and name?
            # TODO: I very much do not want to have people passing in strings here. Do we really need the expression
            #       as a string? Does it get used by the TH?
            self.runner_hook.step_skipped(name=str(num), expression="")
        logging.info(f'**** Skipping: {num}')
        self.step_skipped = True

    def mark_all_remaining_steps_skipped(self, starting_step_number: typing.Union[int, str]) -> None:
        """Mark all remaining test steps starting with provided starting step
            starting_step_number gives the first step to be skipped, as defined in the TestStep.test_plan_number
            starting_step_number must be provided, and is not derived intentionally.
            By providing argument test is more deliberately identifying where test skips are starting from,
            making it easier to validate against the test plan for correctness.
        Args:
            starting_step_number (int,str): Number of name of the step to start skipping the steps.

        Returns nothing on success so the test can go on.
        """
        self.mark_step_range_skipped(starting_step_number, None)

    def mark_step_range_skipped(self, starting_step_number: typing.Union[int, str], ending_step_number: typing.Union[int, str, None]) -> None:
        """Mark a range of remaining test steps starting with provided starting step
            starting_step_number gives the first step to be skipped, as defined in the TestStep.test_plan_number
            starting_step_number must be provided, and is not derived intentionally.

            If ending_step_number is provided, it gives the last step to be skipped, as defined in the TestStep.test_plan_number.
            If ending_step_number is None, all steps until the end of the test will be skipped
            ending_step_number is optional, and if not provided, all steps until the end of the test will be skipped.

            By providing argument test is more deliberately identifying where test skips are starting from,
            making it easier to validate against the test plan for correctness.
        Args:
            starting_step_number (int,str): Number of name of the step to start skipping the steps.
            ending_step_number (int,str,None): Number of name of the step to stop skipping the steps (inclusive).

        Returns nothing on success so the test can go on.
        """
        steps = self.get_test_steps(self.current_test_info.name)
        starting_step_idx = None
        for idx, step in enumerate(steps):
            if step.test_plan_number == starting_step_number:
                starting_step_idx = idx
                break
        asserts.assert_is_not_none(starting_step_idx, "mark_step_ranges_skipped was provided with invalid starting_step_num")
        starting_index: int = typing.cast(int, starting_step_idx)

        ending_step_idx = None
        # If ending_step_number is None, we skip all steps until the end of the test
        if ending_step_number is not None:
            for idx, step in enumerate(steps):
                if step.test_plan_number == ending_step_number:
                    ending_step_idx = idx
                    break

            asserts.assert_is_not_none(ending_step_idx, "mark_step_ranges_skipped was provided with invalid ending_step_num")
            ending_index: int = typing.cast(int, ending_step_idx)
            asserts.assert_greater(ending_index, starting_index,
                                   "mark_step_ranges_skipped was provided with ending_step_num that is before starting_step_num")
            skipping_steps = steps[starting_index:ending_index+1]
        else:
            skipping_steps = steps[starting_index:]

        for step in skipping_steps:
            self.skip_step(step.test_plan_number)

    #
    # Matter Test API - Guard/Condition Helpers (PICS, Attribute, etc.)
    #

    def check_pics(self, pics_key: str) -> bool:
        """Check if a PICS (Protocol Implementation Conformance Statement) key is enabled.

        Args:
            pics_key: The PICS key to check.

        Returns:
            True if the PICS key is enabled, False otherwise.
        """
        return self.matter_test_config.pics.get(pics_key.strip(), False)

    def pics_guard(self, pics_condition: bool):
        """Checks a condition and if False marks the test step as skipped and
           returns False, otherwise returns True.
           For example can be used to check if a test step should be run:

              self.step("4")
              if self.pics_guard(condition_needs_to_be_true_to_execute):
                  # do the test for step 4

              self.step("5")
              if self.pics_guard(condition2_needs_to_be_true_to_execute):
                  # do the test for step 5
           """
        if not pics_condition:
            self.mark_current_step_skipped()
        return pics_condition

    async def _populate_wildcard(self):
        """ Populates self.stored_global_wildcard if not already filled. """
        if self.stored_global_wildcard is None:
            global_wildcard = asyncio.wait_for(self.default_controller.Read(self.dut_node_id, [(Clusters.Descriptor), Attribute.AttributePath(None, None, GlobalAttributeIds.ATTRIBUTE_LIST_ID), Attribute.AttributePath(
                None, None, GlobalAttributeIds.FEATURE_MAP_ID), Attribute.AttributePath(None, None, GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID)]), timeout=60)
            self.stored_global_wildcard = await global_wildcard

    async def attribute_guard(self, endpoint: int, attribute: ClusterObjects.ClusterAttributeDescriptor):
        """Similar to pics_guard above, except checks a condition and if False marks the test step as skipped and
           returns False using attributes against attributes_list, otherwise returns True.
           For example can be used to check if a test step should be run:

              self.step("1")
              if self.attribute_guard(condition1_needs_to_be_true_to_execute):
                  # do the test for step 1

              self.step("2")
              if self.attribute_guard(condition2_needs_to_be_false_to_skip_step):
                  # skip step 2 if condition not met
           """
        await self._populate_wildcard()
        attr_condition = _has_attribute(wildcard=self.stored_global_wildcard, endpoint=endpoint, attribute=attribute)
        if not attr_condition:
            self.mark_current_step_skipped()
        return attr_condition

    async def command_guard(self, endpoint: int, command: ClusterObjects.ClusterCommand):
        """Similar to attribute_guard above, except checks a condition and if False marks the test step as skipped and
           returns False using command id against AcceptedCmdsList, otherwise returns True.
           For example can be used to check if a test step should be run:

              self.step("1")
              if self.command_guard(condition1_needs_to_be_true_to_execute):
                  # do the test for step 1

              self.step("2")
              if self.command_guard(condition2_needs_to_be_false_to_skip_step):
                  # skip step 2 if condition not met
           """
        await self._populate_wildcard()
        cmd_condition = _has_command(wildcard=self.stored_global_wildcard, endpoint=endpoint, command=command)
        if not cmd_condition:
            self.mark_current_step_skipped()
        return cmd_condition

    async def feature_guard(self, endpoint: int, cluster: ClusterObjects.ClusterObjectDescriptor, feature_int: IntFlag):
        """Similar to command_guard and attribute_guard above, except checks a condition and if False marks the test step as skipped and
           returns False using feature id against feature_map, otherwise returns True.
           For example can be used to check if a test step should be run:

              self.step("1")
              if self.feature_guard(condition1_needs_to_be_true_to_execute):
                  # do the test for step 1

              self.step("2")
              if self.feature_guard(condition2_needs_to_be_false_to_skip_step):
                  # skip step 2 if condition not met
           """
        await self._populate_wildcard()
        feat_condition = _has_feature(wildcard=self.stored_global_wildcard, endpoint=endpoint, cluster=cluster, feature=feature_int)
        if not feat_condition:
            self.mark_current_step_skipped()
        return feat_condition

    #
    # Matter Test API - Asynchronous Device Interaction Helpers
    #

    async def commission_devices(self) -> bool:
        """Commission all configured DUT devices.

        Uses the default controller to commission devices based on setup payloads
        and commissioning configuration.

        Returns:
            True if commissioning succeeded, False otherwise.
        """
        dev_ctrl: ChipDeviceCtrl.ChipDeviceController = self.default_controller
        dut_node_ids: List[int] = self.matter_test_config.dut_node_ids
        setup_payloads: List[SetupPayloadInfo] = self.get_setup_payload_info()
        commissioning_info: CommissioningInfo = CommissioningInfo(
            commissionee_ip_address_just_for_testing=self.matter_test_config.commissionee_ip_address_just_for_testing,
            commissioning_method=self.matter_test_config.commissioning_method,
            thread_operational_dataset=self.matter_test_config.thread_operational_dataset,
            wifi_passphrase=self.matter_test_config.wifi_passphrase,
            wifi_ssid=self.matter_test_config.wifi_ssid,
            tc_version_to_simulate=self.matter_test_config.tc_version_to_simulate,
            tc_user_response_to_simulate=self.matter_test_config.tc_user_response_to_simulate,
        )

        return await commission_devices(dev_ctrl, dut_node_ids, setup_payloads, commissioning_info)

    async def open_commissioning_window(self, dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None, node_id: Optional[int] = None, timeout: int = 900) -> CustomCommissioningParameters:
        """Open a commissioning window on the target device.

        Args:
            dev_ctrl: Device controller to use, defaults to default_controller.
            node_id: Node ID of target device, defaults to dut_node_id.
            timeout: Commissioning window timeout in seconds.

        Returns:
            Custom commissioning parameters for the opened window.

        Raises:
            AssertionError: If opening the commissioning window fails.
        """
        rnd_discriminator = random.randint(0, 4095)
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        try:
            commissioning_params = await dev_ctrl.OpenCommissioningWindow(nodeid=node_id, timeout=timeout, iteration=1000,
                                                                          discriminator=rnd_discriminator, option=dev_ctrl.CommissioningWindowPasscode.kTokenWithRandomPin)
            params = CustomCommissioningParameters(commissioning_params, rnd_discriminator)
            return params

        except InteractionModelError as e:
            asserts.fail(e.status, 'Failed to open commissioning window')
            raise  # Help mypy understand this never returns

    async def read_single_attribute(
            self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController, node_id: int, endpoint: int, attribute: Type[ClusterObjects.ClusterAttributeDescriptor], fabricFiltered: bool = True) -> object:
        """Read a single attribute value from a device.

        Args:
            dev_ctrl: Device controller to use for the read operation.
            node_id: Node ID of the target device.
            endpoint: Endpoint ID where the attribute resides.
            attribute: The attribute to read.
            fabricFiltered: Whether to apply fabric filtering.

        Returns:
            The attribute value.
        """
        result = await dev_ctrl.ReadAttribute(node_id, [(endpoint, attribute)], fabricFiltered=fabricFiltered)
        data = result[endpoint]
        return list(data.values())[0][attribute]

    async def read_single_attribute_all_endpoints(
            self, cluster: ClusterObjects.Cluster, attribute: Type[ClusterObjects.ClusterAttributeDescriptor],
            dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None, node_id: Optional[int] = None):
        """Reads a single attribute of a specified cluster across all endpoints.

        Returns:
            dict: endpoint to attribute value

        """
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        # mypy expects tuple-shaped items here. Some tests crash when attribute requests are wrapped in a single-element tuple here.
        # We pass the plain attribute to avoid the runtime issue; so we ignore that type.
        read_response = await dev_ctrl.ReadAttribute(node_id, [attribute])  # type: ignore[list-item]
        attrs = {}
        for endpoint in read_response:
            attr_ret = read_response[endpoint][cluster][attribute]
            attrs[endpoint] = attr_ret
        return attrs

    async def read_single_attribute_check_success(
            self, cluster: ClusterObjects.Cluster, attribute: Type[ClusterObjects.ClusterAttributeDescriptor],
            dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None, node_id: Optional[int] = None, endpoint: Optional[int] = None, fabric_filtered: bool = True, assert_on_error: bool = True, test_name: str = "") -> object:
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        if endpoint is None:
            endpoint = self.get_endpoint()
        result = await dev_ctrl.ReadAttribute(node_id, [(endpoint, attribute)], fabricFiltered=fabric_filtered)
        attr_ret = result[endpoint][cluster][attribute]
        read_err_msg = f"Error reading {str(cluster)}:{str(attribute)} = {attr_ret}"
        desired_type = attribute.attribute_type.Type
        type_err_msg = f'Returned attribute {attribute} is wrong type expected {desired_type}, got {type(attr_ret)}'
        read_ok = attr_ret is not None and not isinstance(attr_ret, Clusters.Attribute.ValueDecodeFailure)
        type_ok = type_matches(attr_ret, desired_type)
        if assert_on_error:
            asserts.assert_true(read_ok, read_err_msg)
            asserts.assert_true(type_ok, type_err_msg)
        else:
            location = AttributePathLocation(endpoint_id=endpoint, cluster_id=cluster.id,
                                             attribute_id=attribute.attribute_id)
            if not read_ok:
                self.record_error(test_name=test_name, location=location, problem=read_err_msg)
                return None
            elif not type_ok:
                self.record_error(test_name=test_name, location=location, problem=type_err_msg)
                return None
        return attr_ret

    async def read_single_attribute_expect_error(
            self, cluster: ClusterObjects.Cluster, attribute: Type[ClusterObjects.ClusterAttributeDescriptor],
            error: Status, dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None, node_id: Optional[int] = None, endpoint: Optional[int] = None,
            fabric_filtered: bool = True, assert_on_error: bool = True, test_name: str = "") -> object:
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        if endpoint is None:
            endpoint = self.get_endpoint()
        result = await dev_ctrl.ReadAttribute(node_id, [(endpoint, attribute)], fabricFiltered=fabric_filtered)
        attr_ret = result[endpoint][cluster][attribute]
        err_msg = "Did not see expected error when reading {}:{}".format(str(cluster), str(attribute))
        error_type_ok = attr_ret is not None and isinstance(
            attr_ret, Clusters.Attribute.ValueDecodeFailure) and isinstance(attr_ret.Reason, InteractionModelError)
        if assert_on_error:
            asserts.assert_true(error_type_ok, err_msg)
            asserts.assert_equal(attr_ret.Reason.status, error, err_msg)
        elif not error_type_ok or attr_ret.Reason.status != error:
            location = AttributePathLocation(endpoint_id=endpoint, cluster_id=cluster.id,
                                             attribute_id=attribute.attribute_id)
            self.record_error(test_name=test_name, location=location, problem=err_msg)
            return None

        return attr_ret

    async def write_single_attribute(self, attribute_value: ClusterObjects.ClusterAttributeDescriptor, endpoint_id: Optional[int] = None, expect_success: bool = True) -> Status:
        """Write a single `attribute_value` on a given `endpoint_id` and assert on failure.

        If `endpoint_id` is None, the default DUT endpoint for the test is selected.

        If `expect_success` is True, a test assertion fails on error status codes

        Status code is returned.
        """
        dev_ctrl = self.default_controller
        node_id = self.dut_node_id
        if endpoint_id is None:
            endpoint_id = 0 if self.matter_test_config.endpoint is None else self.matter_test_config.endpoint

        write_result = await dev_ctrl.WriteAttribute(node_id, [(endpoint_id, attribute_value)])
        if expect_success:
            asserts.assert_equal(write_result[0].Status, Status.Success,
                                 f"Expected write success for write to attribute {attribute_value} on endpoint {endpoint_id}")
        return write_result[0].Status

    def write_to_app_pipe(self, command_dict: dict, app_pipe: Optional[str] = None):
        """
        Send an out-of-band command to a Matter app.
        Args:
            command_dict (dict): dictionary with the command and data.
            app_pipe (Optional[str], optional): Name of the cluster pipe file  (i.e. /tmp/chip_all_clusters_fifo_55441 or /tmp/chip_rvc_fifo_11111). Raises
            FileNotFoundError if pipe file is not found. If None takes the value from the CI argument --app-pipe,  arg --app-pipe has his own file exists check.

        This method uses the following environment variables:

         - LINUX_DUT_IP
            * if not provided, the Matter app is assumed to run on the same machine as the test,
              such as during CI, and the commands are sent to it using a local named pipe
            * if provided, the commands for writing to the named pipe are forwarded to the DUT
        - LINUX_DUT_USER
            * if LINUX_DUT_IP is provided, use this for the DUT user name
            * If a remote password is needed, set up ssh keys to ensure that this script can log in to the DUT without a password:
                 + Step 1: If you do not have a key, create one using ssh-keygen
                 + Step 2: Authorize this key on the remote host: run ssh-copy-id user@ip once, using your password
                 + Step 3: From now on ssh user@ip will no longer ask for your password
        """
        # If is not empty from the args, verify if the fifo file exists.
        if app_pipe is not None and not os.path.exists(app_pipe):
            logging.error("Named pipe %r does NOT exist" % app_pipe)
            raise FileNotFoundError("CANNOT FIND %r" % app_pipe)

        if app_pipe is None:
            app_pipe = self.matter_test_config.pipe_name

        if not isinstance(app_pipe, str):
            raise TypeError("The named pipe must be provided as a string value")

        if not isinstance(command_dict, dict):
            raise TypeError("The command must be passed as a dictionary value")

        command = json.dumps(command_dict)
        dut_ip = os.getenv('LINUX_DUT_IP')

        # Checks for concatenate app_pipe and app_pid
        if dut_ip is None:
            with open(app_pipe, "w") as app_pipe_fp:
                logger.info(f"Sending out-of-band command: {command} to file: {app_pipe}")
                app_pipe_fp.write(json.dumps(command_dict) + "\n")
            # TODO(#31239): remove the need for sleep
            # This was tested with matter.js as being reliable enough
            time.sleep(0.05)
        else:
            logging.info(f"Using DUT IP address: {dut_ip}")

            dut_uname = os.getenv('LINUX_DUT_USER')
            asserts.assert_true(dut_uname is not None, "The LINUX_DUT_USER environment variable must be set")
            logging.info(f"Using DUT user name: {dut_uname}")
            command_fixed = shlex.quote(json.dumps(command_dict))
            cmd = "echo \"%s\" | ssh %s@%s \'cat > %s\'" % (command_fixed, dut_uname, dut_ip, app_pipe)
            os.system(cmd)

    async def send_single_cmd(
            self, cmd: Clusters.ClusterObjects.ClusterCommand,
            dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None, node_id: Optional[int] = None, endpoint: Optional[int] = None,
            timedRequestTimeoutMs: OptionalTimeout = None,
            payloadCapability: int = ChipDeviceCtrl.TransportPayloadCapability.MRP_PAYLOAD) -> object:
        """Send a single command to a Matter device.

        Args:
            cmd: The cluster command to send.
            dev_ctrl: Device controller, defaults to default_controller.
            node_id: Target node ID, defaults to dut_node_id.
            endpoint: Target endpoint, defaults to configured endpoint.
            timedRequestTimeoutMs: Timeout for timed requests in milliseconds.
            payloadCapability: Transport payload capability setting.

        Returns:
            Command response object.
        """
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        if endpoint is None:
            endpoint = self.get_endpoint()

        result = await dev_ctrl.SendCommand(nodeid=node_id, endpoint=endpoint, payload=cmd, timedRequestTimeoutMs=timedRequestTimeoutMs,
                                            payloadCapability=payloadCapability)
        return result

    async def send_test_event_triggers(self, eventTrigger: int, enableKey: Optional[bytes] = None):
        """This helper function sends a test event trigger to the General Diagnostics cluster on endpoint 0

           The enableKey can be passed into the function, or omitted which will then
           use the one provided to the script via --hex-arg enableKey:<HEX VALUE>
           if not it defaults to 0x000102030405060708090a0b0c0d0e0f
        """
        # get the test event enable key or assume the default
        # This can be passed in on command line using
        #    --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
        if enableKey is None:
            if 'enableKey' not in self.matter_test_config.global_test_params:
                enableKey = bytes([b for b in range(16)])
            else:
                enableKey = self.matter_test_config.global_test_params['enableKey']

        eventTrigger = self._update_legacy_test_event_triggers(eventTrigger)

        try:
            # GeneralDiagnostics cluster is meant to be on Endpoint 0 (Root)
            await self.send_single_cmd(endpoint=0, cmd=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(enableKey, uint(eventTrigger)))

        except InteractionModelError as e:
            asserts.fail(
                f"Sending TestEventTrigger resulted in Unexpected error. Are they enabled in DUT? Command returned - {e.status}")

    async def check_test_event_triggers_enabled(self):
        """This cluster checks that the General Diagnostics cluster TestEventTriggersEnabled attribute is True.
           It will assert and fail the test if not True."""
        full_attr = Clusters.GeneralDiagnostics.Attributes.TestEventTriggersEnabled
        cluster = Clusters.Objects.GeneralDiagnostics
        # GeneralDiagnostics cluster is meant to be on Endpoint 0 (Root)
        test_event_enabled = await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=full_attr)
        asserts.assert_equal(test_event_enabled, True, "TestEventTriggersEnabled is False")

    def _update_legacy_test_event_triggers(self, eventTrigger: int) -> int:
        """Update event trigger if legacy flag is set.

        Args:
            eventTrigger: The base event trigger value.

        Returns:
            Updated event trigger with endpoint information.

        Raises:
            ValueError: If target endpoint is out of valid range.
        """
        target_endpoint = 0

        if self.matter_test_config.legacy:
            logger.info("Legacy test event trigger activated")
        else:
            logger.info("Legacy test event trigger deactivated")
            target_endpoint = self.get_endpoint()

        if not (0 <= target_endpoint <= 0xFFFF):
            raise ValueError("Target endpoint should be between 0 and 0xFFFF")

        # Clean endpoint target
        eventTrigger = eventTrigger & ~ (0xFFFF << 32)

        # Sets endpoint in eventTrigger
        eventTrigger |= (target_endpoint & 0xFFFF) << 32

        return eventTrigger

    #
    # Matter Test API - Utility Helpers (Problem Recording, User Input)
    #

    def record_error(self, test_name: str, location: ProblemLocation, problem: str, spec_location: str = ""):
        """Record an error-level problem during test execution.

        Args:
            test_name: Name of the test where the problem occurred.
            location: Location information for the problem.
            problem: Description of the problem.
            spec_location: Specification reference (optional).
        """
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.ERROR, problem, spec_location))

    def record_warning(self, test_name: str, location: ProblemLocation, problem: str, spec_location: str = ""):
        """Record a warning-level problem during test execution.

        Args:
            test_name: Name of the test where the problem occurred.
            location: Location information for the problem.
            problem: Description of the problem.
            spec_location: Specification reference (optional).
        """
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.WARNING, problem, spec_location))

    def record_note(self, test_name: str, location: ProblemLocation, problem: str, spec_location: str = ""):
        """Record a note-level problem during test execution.

        Args:
            test_name: Name of the test where the problem occurred.
            location: Location information for the problem.
            problem: Description of the problem.
            spec_location: Specification reference (optional).
        """
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.NOTE, problem, spec_location))

    def wait_for_user_input(self,
                            prompt_msg: str,
                            prompt_msg_placeholder: str = "Submit anything to continue",
                            default_value: str = "y") -> Optional[str]:
        """Ask for user input and wait for it.

        Args:
            prompt_msg (str): Message for TH UI prompt and input function. Indicates what is expected from the user.
            prompt_msg_placeholder (str, optional): TH UI prompt input placeholder (where the user types). Defaults to "Submit anything to continue".
            default_value (str, optional): TH UI prompt default value. Defaults to "y".

        Returns:
            str: User input or none if input is closed.
        """

        # TODO(#31928): Remove any assumptions of test params for endpoint ID.

        # Get the endpoint user param instead of `--endpoint-id` result, if available, temporarily.
        endpoint_id = self.user_params.get("endpoint", None)
        if endpoint_id is None or not isinstance(endpoint_id, int):
            endpoint_id = self.matter_test_config.endpoint

        if self.runner_hook:
            # TODO(#31928): Add endpoint support to hooks.
            self.runner_hook.show_prompt(msg=prompt_msg,
                                         placeholder=prompt_msg_placeholder,
                                         default_value=default_value)

        logging.info(f"========= USER PROMPT for Endpoint {endpoint_id} =========")
        logging.info(f">>> {prompt_msg.rstrip()} (press enter to confirm)")
        try:
            return input()
        except EOFError:
            logging.info("========= EOF on STDIN =========")
            return None

    def user_verify_snap_shot(self,
                              prompt_msg: str,
                              image: bytes) -> None:
        """Show Image Verification Prompt and wait for user validation.
           This method will be executed only when TC is running in TH.

        Args:
            prompt_msg (str): Message for TH UI prompt and input function.
            Indicates what is expected from the user.
            image (bytes): Image data as bytes.

        Returns:
            Returns nothing indicating success so the test can go on.

        Raises:
            TestError: Indicating image validation step failed.
        """
        # Only run when TC is being executed in TH
        if self.runner_hook and hasattr(self.runner_hook, 'show_image_prompt'):
            # Convert bytes to comma separated hex string
            hex_string = ', '.join(f'{byte:02x}' for byte in image)
            self.runner_hook.show_image_prompt(
                msg=prompt_msg,
                img_hex_str=hex_string
            )

            logging.info("========= USER PROMPT for Image Validation =========")

            try:
                result = input()
                if result != '1':  # User did not select 'PASS'
                    raise TestError("Image validation failed")
            except EOFError:
                logging.info("========= EOF on STDIN =========")
                return None

    def user_verify_video_stream(self,
                                 prompt_msg: str) -> None:
        """Show Video Verification Prompt and wait for user validation.
           This method will be executed only when TC is running in TH.

        Args:
            prompt_msg (str): Message for TH UI prompt and input function.
            Indicates what is expected from the user.

        Returns:
            Returns nothing indicating success so the test can go on.

        Raises:
            TestError: Indicating video validation step failed.
        """
        # Only run when TC is being executed in TH
        if self.runner_hook and hasattr(self.runner_hook, 'show_video_prompt'):
            self.runner_hook.show_video_prompt(msg=prompt_msg)

            logging.info("========= USER PROMPT for Video Stream Validation =========")

            try:
                result = input()
                if result != '1':  # User did not select 'PASS'
                    raise TestError("Video stream validation failed")
            except EOFError:
                logging.info("========= EOF on STDIN =========")
                return None


def _async_runner(body, self: MatterBaseTest, *args, **kwargs):
    """Runs an async function within the test's event loop with a timeout.

    This helper function takes an awaitable (async function) and executes it
    using the test's event loop (`self.event_loop.run_until_complete`).
    It applies a timeout based on the test configuration (`self.matter_test_config.timeout`)
    or the default timeout (`self.default_timeout`) if not specified.

    Args:
        body: The async function (coroutine) to execute. It will be called
              with `self` as the first argument, followed by `*args` and `**kwargs`.
        self: The instance of the MatterBaseTest class.
        *args: Positional arguments to pass to the `body` function.
        **kwargs: Keyword arguments to pass to the `body` function.

    Returns:
        The result returned by the awaited `body` function.
    """
    timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout
    return self.event_loop.run_until_complete(asyncio.wait_for(body(self, *args, **kwargs), timeout=timeout))


EndpointCheckFunction = typing.Callable[[Clusters.Attribute.AsyncReadTransaction.ReadResponse, int], bool]


def get_cluster_from_attribute(attribute: ClusterObjects.ClusterAttributeDescriptor) -> ClusterObjects.Cluster:
    """Returns the cluster object for a given attribute descriptor."""
    return ClusterObjects.ALL_CLUSTERS[attribute.cluster_id]


def get_cluster_from_command(command: ClusterObjects.ClusterCommand) -> ClusterObjects.Cluster:
    """Returns the cluster object for a given command object."""
    return ClusterObjects.ALL_CLUSTERS[command.cluster_id]


async def _get_all_matching_endpoints(self: MatterBaseTest, accept_function: EndpointCheckFunction) -> list[uint]:
    """ Returns a list of endpoints matching the accept condition. """
    wildcard = await self.default_controller.Read(self.dut_node_id, [
        (Clusters.Descriptor,),  # single-element tuple needs trailing comma
        Attribute.AttributePath(None, None, GlobalAttributeIds.ATTRIBUTE_LIST_ID),
        Attribute.AttributePath(None, None, GlobalAttributeIds.FEATURE_MAP_ID),
        Attribute.AttributePath(None, None, GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID)
    ])
    matching = [e for e in wildcard.attributes.keys()
                if accept_function(wildcard, e)]
    return matching


# TODO(#37537): Remove these temporary aliases after transition period
type_matches = matchers.is_type
utc_time_in_matter_epoch = timeoperations.utc_time_in_matter_epoch
utc_datetime_from_matter_epoch_us = timeoperations.utc_datetime_from_matter_epoch_us
utc_datetime_from_posix_time_ms = timeoperations.utc_datetime_from_posix_time_ms
compare_time = timeoperations.compare_time
get_wait_seconds_from_set_time = timeoperations.get_wait_seconds_from_set_time
bytes_from_hex = conversions.bytes_from_hex
hex_from_bytes = conversions.hex_from_bytes
id_str = conversions.format_decimal_and_hex
cluster_id_str = conversions.cluster_id_with_name

async_test_body = decorators.async_test_body
run_if_endpoint_matches = decorators.run_if_endpoint_matches
run_on_singleton_matching_endpoint = decorators.run_on_singleton_matching_endpoint
has_cluster = decorators.has_cluster
has_attribute = decorators.has_attribute
has_command = decorators.has_command
has_feature = decorators.has_feature
should_run_test_on_endpoint = decorators.should_run_test_on_endpoint
# autopep8: off
_get_all_matching_endpoints = decorators._get_all_matching_endpoints  # type: ignore[assignment]
# autopep8: on
_has_feature = decorators._has_feature
_has_command = decorators._has_command
_has_attribute = decorators._has_attribute

default_matter_test_main = runner.default_matter_test_main
get_test_info = runner.get_test_info
run_tests = runner.run_tests
run_tests_no_exit = runner.run_tests_no_exit
get_default_paa_trust_store = runner.get_default_paa_trust_store

# Backward compatibility aliases for relocated functions
parse_matter_test_args = runner.parse_matter_test_args

# isort: off

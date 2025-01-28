"""Core test infrastructure for Matter testing."""

import asyncio
import inspect
import logging
import random
import textwrap
import typing
from datetime import datetime, timedelta, timezone
from enum import IntFlag
from time import sleep
from typing import List, Optional

import chip
import chip.clusters as Clusters
from chip import discovery
from chip.clusters import Attribute, ClusterObjects
from chip.clusters.Attribute import Status
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError
from chip.setup_payload import SetupPayload
from mobly import asserts, base_test, signals

from controller.python.chip import ChipDeviceCtrl
from python_testing.matter_testing_infrastructure.chip.testing.global_attribute_ids import GlobalAttributeIds

from .decorators import _has_attribute, _has_command, _has_feature
from .models import (AttributePathLocation, ClusterMapper, CustomCommissioningParameters, DiscoveryFilterType, ProblemLocation,
                     ProblemNotice, ProblemSeverity, SetupPayloadInfo, TestStep)
from .runner import MatterStackState, MatterTestConfig, TestRunnerHooks, unstash_globally
from .utilities import type_matches

logger = logging.getLogger("matter.python_testing")
logger.setLevel(logging.INFO)


class MatterBaseTest(base_test.BaseTestClass):
    def __init__(self, *args):
        super().__init__(*args)

        # List of accumulated problems across all tests
        self.problems = []
        self.is_commissioning = False
        # The named pipe name must be set in the derived classes
        self.app_pipe = None

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
        steps_name = f'steps_{test.removeprefix("test_")}'
        try:
            fn = getattr(self, steps_name)
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

    def get_default_app_pipe_name(self) -> str:
        return self.app_pipe

    def write_to_app_pipe(self, command_dict: dict, app_pipe_name: Optional[str] = None):
        """
        Sends an out-of-band command to a Matter app.

        Use the following environment variables:

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

        if app_pipe_name is None:
            app_pipe_name = self.get_default_app_pipe_name()

        if not isinstance(app_pipe_name, str):
            raise TypeError("the named pipe must be provided as a string value")

        if not isinstance(command_dict, dict):
            raise TypeError("the command must be passed as a dictionary value")

        import json
        command = json.dumps(command_dict)

        import os
        dut_ip = os.getenv('LINUX_DUT_IP')

        if dut_ip is None:
            with open(app_pipe_name, "w") as app_pipe:
                app_pipe.write(command + "\n")
            # TODO(#31239): remove the need for sleep
            sleep(0.001)
        else:
            logging.info(f"Using DUT IP address: {dut_ip}")

            dut_uname = os.getenv('LINUX_DUT_USER')
            asserts.assert_true(dut_uname is not None, "The LINUX_DUT_USER environment variable must be set")

            logging.info(f"Using DUT user name: {dut_uname}")

            command_fixed = command.replace('\"', '\\"')
            cmd = "echo \"%s\" | ssh %s@%s \'cat > %s\'" % (command_fixed, dut_uname, dut_ip, app_pipe_name)
            os.system(cmd)

    # Override this if the test requires a different default timeout.
    # This value will be overridden if a timeout is supplied on the command line.
    @property
    def default_timeout(self) -> int:
        return 90

    @property
    def runner_hook(self) -> TestRunnerHooks:
        return unstash_globally(self.user_params.get("hooks"))

    @property
    def matter_test_config(self) -> MatterTestConfig:
        return unstash_globally(self.user_params.get("matter_test_config"))

    @property
    def default_controller(self) -> ChipDeviceCtrl.ChipDeviceController:
        return unstash_globally(self.user_params.get("default_controller"))

    @property
    def matter_stack(self) -> MatterStackState:
        return unstash_globally(self.user_params.get("matter_stack"))

    @property
    def certificate_authority_manager(self) -> chip.CertificateAuthority.CertificateAuthorityManager:
        return unstash_globally(self.user_params.get("certificate_authority_manager"))

    @property
    def dut_node_id(self) -> int:
        return self.matter_test_config.dut_node_ids[0]

    def get_endpoint(self, default: Optional[int] = 0) -> int:
        return self.matter_test_config.endpoint if self.matter_test_config.endpoint is not None else default

    def setup_class(self):
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

    def setup_test(self):
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

    def teardown_class(self):
        """Final teardown after all tests: log all problems."""
        if len(self.problems) > 0:
            logging.info("###########################################################")
            logging.info("Problems found:")
            logging.info("===============")
            for problem in self.problems:
                logging.info(str(problem))
            logging.info("###########################################################")
        super().teardown_class()

    def check_pics(self, pics_key: str) -> bool:
        return self.matter_test_config.pics.get(pics_key.strip(), False)

    @property
    def is_pics_sdk_ci_only(self) -> bool:
        return self.check_pics('PICS_SDK_CI_ONLY')

    async def open_commissioning_window(self, dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None, node_id: Optional[int] = None, timeout: int = 900) -> CustomCommissioningParameters:
        rnd_discriminator = random.randint(0, 4095)
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        try:
            commissioning_params = await dev_ctrl.OpenCommissioningWindow(nodeid=node_id, timeout=timeout, iteration=1000,
                                                                          discriminator=rnd_discriminator, option=1)
            params = CustomCommissioningParameters(commissioning_params, rnd_discriminator)
            return params

        except InteractionModelError as e:
            asserts.fail(e.status, 'Failed to open commissioning window')

    async def read_single_attribute(
            self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController, node_id: int, endpoint: int, attribute: object, fabricFiltered: bool = True) -> object:
        result = await dev_ctrl.ReadAttribute(node_id, [(endpoint, attribute)], fabricFiltered=fabricFiltered)
        data = result[endpoint]
        return list(data.values())[0][attribute]

    async def read_single_attribute_check_success(
            self, cluster: Clusters.ClusterObjects.ClusterCommand, attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor,
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
            self, cluster: object, attribute: object,
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

    async def write_single_attribute(self, attribute_value: object, endpoint_id: Optional[int] = None, expect_success: bool = True) -> Status:
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

    async def send_single_cmd(
            self, cmd: Clusters.ClusterObjects.ClusterCommand,
            dev_ctrl: Optional[ChipDeviceCtrl.ChipDeviceController] = None, node_id: Optional[int] = None, endpoint: Optional[int] = None,
            timedRequestTimeoutMs: typing.Union[None, int] = None,
            payloadCapability: int = ChipDeviceCtrl.TransportPayloadCapability.MRP_PAYLOAD) -> object:
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        if endpoint is None:
            endpoint = self.get_endpoint()

        result = await dev_ctrl.SendCommand(nodeid=node_id, endpoint=endpoint, payload=cmd, timedRequestTimeoutMs=timedRequestTimeoutMs,
                                            payloadCapability=payloadCapability)
        return result

    async def send_test_event_triggers(self, eventTrigger: int, enableKey: bytes = None):
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

        try:
            # GeneralDiagnostics cluster is meant to be on Endpoint 0 (Root)
            await self.send_single_cmd(endpoint=0,
                                       cmd=Clusters.GeneralDiagnostics.Commands.TestEventTrigger(
                                           enableKey,
                                           eventTrigger)
                                       )

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

    def print_step(self, stepnum: typing.Union[int, str], title: str) -> None:
        logging.info(f'***** Test Step {stepnum} : {title}')

    def record_error(self, test_name: str, location: ProblemLocation, problem: str, spec_location: str = ""):
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.ERROR, problem, spec_location))

    def record_warning(self, test_name: str, location: ProblemLocation, problem: str, spec_location: str = ""):
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.WARNING, problem, spec_location))

    def record_note(self, test_name: str, location: ProblemLocation, problem: str, spec_location: str = ""):
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.NOTE, problem, spec_location))

    def on_fail(self, record):
        ''' Called by Mobly on test failure

            record is of type TestResultRecord
        '''
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
        ''' Called by Mobly on test pass

            record is of type TestResultRecord
        '''
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
        ''' Called by Mobly on test skip

            record is of type TestResultRecord
        '''
        if self.runner_hook and not self.is_commissioning:
            test_duration = (datetime.now(timezone.utc) - self.test_start_time) / timedelta(microseconds=1)
            test_name = self.current_test_info.name
            filename = inspect.getfile(self.__class__)
            self.runner_hook.test_skipped(filename, test_name)
            self.runner_hook.test_stop(exception=None, duration=test_duration)

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

    def mark_current_step_skipped(self):
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

    def skip_step(self, step):
        self.step(step)
        self.mark_current_step_skipped()

    def skip_all_remaining_steps(self, starting_step_number):
        ''' Skips all remaining test steps starting with provided starting step

            starting_step_number gives the first step to be skipped, as defined in the TestStep.test_plan_number
            starting_step_number must be provided, and is not derived intentionally. By providing argument
                test is more deliberately identifying where test skips are starting from, making
                it easier to validate against the test plan for correctness.
        '''
        steps = self.get_test_steps(self.current_test_info.name)
        for idx, step in enumerate(steps):
            if step.test_plan_number == starting_step_number:
                starting_step_idx = idx
                break
        else:
            asserts.fail("skip_all_remaining_steps was provided with invalid starting_step_num")
        remaining = steps[starting_step_idx:]
        for step in remaining:
            self.skip_step(step.test_plan_number)

    def step(self, step: typing.Union[int, str]):
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

    def get_setup_payload_info(self) -> List[SetupPayloadInfo]:
        setup_payloads = []
        for qr_code in self.matter_test_config.qr_code_content:
            try:
                setup_payloads.append(SetupPayload().ParseQrCode(qr_code))
            except ChipStackError:
                asserts.fail(f"QR code '{qr_code} failed to parse properly as a Matter setup code.")

        for manual_code in self.matter_test_config.manual_code:
            try:
                setup_payloads.append(SetupPayload().ParseManualPairingCode(manual_code))
            except ChipStackError:
                asserts.fail(
                    f"Manual code code '{manual_code}' failed to parse properly as a Matter setup code. Check that all digits are correct and length is 11 or 21 characters.")

        infos = []
        for setup_payload in setup_payloads:
            info = SetupPayloadInfo()
            info.passcode = setup_payload.setup_passcode
            if setup_payload.short_discriminator is not None:
                info.filter_type = discovery.FilterType.SHORT_DISCRIMINATOR
                info.filter_value = setup_payload.short_discriminator
            else:
                info.filter_type = discovery.FilterType.LONG_DISCRIMINATOR
                info.filter_value = setup_payload.long_discriminator
            infos.append(info)

        num_passcodes = 0 if self.matter_test_config.setup_passcodes is None else len(self.matter_test_config.setup_passcodes)
        num_discriminators = 0 if self.matter_test_config.discriminators is None else len(self.matter_test_config.discriminators)
        asserts.assert_equal(num_passcodes, num_discriminators, "Must have same number of discriminators as passcodes")
        if self.matter_test_config.discriminators:
            for idx, discriminator in enumerate(self.matter_test_config.discriminators):
                info = SetupPayloadInfo()
                info.passcode = self.matter_test_config.setup_passcodes[idx]
                info.filter_type = DiscoveryFilterType.LONG_DISCRIMINATOR
                info.filter_value = discriminator
                infos.append(info)

        return infos

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

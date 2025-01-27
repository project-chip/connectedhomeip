"""Core test infrastructure for Matter testing."""

import inspect
import logging
import textwrap
from datetime import datetime, timedelta, timezone
from typing import Optional, Union

import chip.clusters as Clusters
from chip.ChipDeviceCtrl import ChipDeviceController
from chip.clusters.Attribute import Status
from mobly import asserts, base_test, signals

from .models import AttributePathLocation, ClusterMapper, ProblemLocation, ProblemNotice, ProblemSeverity
from .utilities import type_matches

logger = logging.getLogger("matter.python_testing")
logger.setLevel(logging.INFO)


class MatterBaseTest(base_test.BaseTestClass):
    """Base class for Matter test cases."""

    def __init__(self, *args):
        super().__init__(*args)
        self.problems = []
        self.is_commissioning = False
        self.app_pipe = None
        self.stored_global_wildcard = None

    def setup_class(self):
        """Set up the test class."""
        super().setup_class()
        self.cluster_mapper = ClusterMapper(self.default_controller._Cluster)
        self.current_step_index = 0
        self.step_start_time = datetime.now(timezone.utc)
        self.step_skipped = False

    def setup_test(self):
        """Set up an individual test."""
        self.current_step_index = 0
        self.test_start_time = datetime.now(timezone.utc)
        self.step_start_time = datetime.now(tz=timezone.utc)
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
            if steps is None:
                self.step(1)

    def teardown_class(self):
        """Clean up after all tests are complete."""
        if len(self.problems) > 0:
            logging.info("###########################################################")
            logging.info("Problems found:")
            logging.info("===============")
            for problem in self.problems:
                logging.info(str(problem))
            logging.info("###########################################################")
        super().teardown_class()

    async def read_single_attribute(
            self, dev_ctrl: ChipDeviceController, node_id: int, endpoint: int, attribute: object, fabricFiltered: bool = True) -> object:
        """Read a single attribute from a device."""
        result = await dev_ctrl.ReadAttribute(node_id, [(endpoint, attribute)], fabricFiltered=fabricFiltered)
        data = result[endpoint]
        return list(data.values())[0][attribute]

    async def read_single_attribute_check_success(
            self, cluster: Clusters.ClusterObjects.ClusterCommand, attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor,
            dev_ctrl: Optional[ChipDeviceController] = None, node_id: Optional[int] = None, endpoint: Optional[int] = None,
            fabric_filtered: bool = True, assert_on_error: bool = True, test_name: str = "") -> object:
        """Read a single attribute and verify success."""
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

    async def write_single_attribute(self, attribute_value: object, endpoint_id: Optional[int] = None,
                                     expect_success: bool = True) -> Status:
        """Write a single attribute value."""
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
            dev_ctrl: Optional[ChipDeviceController] = None, node_id: Optional[int] = None, endpoint: Optional[int] = None,
            timedRequestTimeoutMs: Union[None, int] = None,
            payloadCapability: int = ChipDeviceController.TransportPayloadCapability.MRP_PAYLOAD) -> object:
        """Send a single command to a device."""
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        if endpoint is None:
            endpoint = self.get_endpoint()

        result = await dev_ctrl.SendCommand(nodeid=node_id, endpoint=endpoint, payload=cmd,
                                            timedRequestTimeoutMs=timedRequestTimeoutMs,
                                            payloadCapability=payloadCapability)
        return result

    def record_error(self, test_name: str, location: ProblemLocation, problem: str, spec_location: str = ""):
        """Record an error that occurred during testing."""
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.ERROR, problem, spec_location))

    def record_warning(self, test_name: str, location: ProblemLocation, problem: str, spec_location: str = ""):
        """Record a warning that occurred during testing."""
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.WARNING, problem, spec_location))

    def record_note(self, test_name: str, location: ProblemLocation, problem: str, spec_location: str = ""):
        """Record a note about the test execution."""
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.NOTE, problem, spec_location))

    def on_fail(self, record):
        """Handle test failure."""
        self.failed = True
        if self.runner_hook and not self.is_commissioning:
            exception = record.termination_signal.exception

            try:
                step_duration = (datetime.now(timezone.utc) - self.step_start_time) / timedelta(microseconds=1)
            except AttributeError:
                step_duration = 0
            try:
                test_duration = (datetime.now(timezone.utc) - self.test_start_time) / timedelta(microseconds=1)
            except AttributeError:
                test_duration = 0

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
                    assert_candidates = [idx for idx, line in enumerate(trace) if "asserts" in line and "asserts.py" not in line]
                    if not assert_candidates:
                        return "Unknown error, please see stack trace above", ""
                    assert_candidate_idx = assert_candidates[-1]
                else:
                    assert_candidate_idx = -1
                probable_error = trace[assert_candidate_idx]

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
        """Handle test success."""
        if self.runner_hook and not self.is_commissioning:
            step_duration = (datetime.now(timezone.utc) - self.step_start_time) / timedelta(microseconds=1)
            test_duration = (datetime.now(timezone.utc) - self.test_start_time) / timedelta(microseconds=1)
            self.runner_hook.step_success(logger=None, logs=None, duration=step_duration, request=None)

        steps = self.get_defined_test_steps(record.test_name)
        if steps is None:
            all_steps_run = True
        else:
            all_steps_run = len(steps) == self.current_step_index

        if not all_steps_run:
            asserts.fail("Test script error: Not all required steps were run")

        if self.runner_hook and not self.is_commissioning:
            self.runner_hook.test_stop(exception=None, duration=test_duration)

    def on_skip(self, record):
        """Handle test skip."""
        if self.runner_hook and not self.is_commissioning:
            test_duration = (datetime.now(timezone.utc) - self.test_start_time) / timedelta(microseconds=1)
            test_name = self.current_test_info.name
            filename = inspect.getfile(self.__class__)
            self.runner_hook.test_skipped(filename, test_name)
            self.runner_hook.test_stop(exception=None, duration=test_duration)

    def step(self, step: Union[int, str]):
        """Execute a test step."""
        test_name = self.current_test_info.name
        steps = self.get_test_steps(test_name)

        if len(steps) <= self.current_step_index or steps[self.current_step_index].test_plan_number != step:
            asserts.fail(f'Unexpected test step: {step} - steps not called in order, or step does not exist')

        current_step = steps[self.current_step_index]
        self.print_step(step, current_step.description)

        if self.runner_hook:
            if not self.step_skipped and self.current_step_index != 0:
                step_duration = (datetime.now(timezone.utc) - self.step_start_time) / timedelta(microseconds=1)
                self.runner_hook.step_success(logger=None, logs=None, duration=step_duration, request=None)

            name = f'{step} : {current_step.description}'
            self.runner_hook.step_start(name=name)

        self.step_start_time = datetime.now(tz=timezone.utc)
        self.current_step_index = self.current_step_index + 1
        self.step_skipped = False

    def print_step(self, stepnum: Union[int, str], title: str) -> None:
        """Print information about the current test step."""
        logging.info(f'***** Test Step {stepnum} : {title}')

    def mark_current_step_skipped(self):
        """Mark the current step as skipped."""
        try:
            steps = self.get_test_steps(self.current_test_info.name)
            if self.current_step_index == 0:
                asserts.fail("Script error: mark_current_step_skipped cannot be called before step()")
            num = steps[self.current_step_index - 1].test_plan_number
        except KeyError:
            num = self.current_step_index

        if self.runner_hook:
            self.runner_hook.step_skipped(name=str(num), expression="")
        logging.info(f'**** Skipping: {num}')
        self.step_skipped = True

    def skip_step(self, step):
        """Skip a specific test step."""
        self.step(step)
        self.mark_current_step_skipped()

    def skip_all_remaining_steps(self, starting_step_number):
        """Skip all remaining test steps starting from a specific step."""
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

    def wait_for_user_input(self,
                            prompt_msg: str,
                            prompt_msg_placeholder: str = "Submit anything to continue",
                            default_value: str = "y") -> Optional[str]:
        """Ask for user input and wait for it."""
        endpoint_id = self.user_params.get("endpoint", None)
        if endpoint_id is None or not isinstance(endpoint_id, int):
            endpoint_id = self.matter_test_config.endpoint

        if self.runner_hook:
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

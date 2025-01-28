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

import argparse
import asyncio
import builtins
import importlib
import logging
import os
import pathlib
import sys
import uuid
from dataclasses import asdict as dataclass_asdict
from datetime import datetime, timedelta, timezone
from itertools import chain
from pathlib import Path
from typing import Any, List, Optional
from unittest.mock import MagicMock

import chip
import chip.utils as utils
from chip.clusters import Attribute
from chip.testing.pics import read_pics_from_file
from chip.tracing import TracingContext
from mobly import signals
from mobly.config_parser import ENV_MOBLY_LOGPATH, TestRunConfig
from mobly.test_runner import TestRunner

from controller.python.chip.ChipStack import ChipStack
from controller.python.chip.storage import PersistentStorage

from .commissioning import CommissionDeviceTest
from .constants import _DEFAULT_ADMIN_VENDOR_ID, _DEFAULT_CONTROLLER_NODE_ID, _DEFAULT_TRUST_ROOT_INDEX
from .matter_base_test import MatterBaseTest
from .models import _DEFAULT_DUT_NODE_ID, _DEFAULT_LOG_PATH, _DEFAULT_STORAGE_PATH, MatterTestConfig, TestInfo
from .utilities import (bool_named_arg, byte_string_from_hex, bytes_as_hex_named_arg, float_named_arg, int_decimal_or_hex,
                        int_named_arg, json_named_arg, root_index, str_from_manual_code, str_named_arg)

try:
    from matter_yamltests.hooks import TestRunnerHooks
except ImportError:
    class TestRunnerHooks:
        pass

logger = logging.getLogger("matter.python_testing")
logger.setLevel(logging.INFO)

# Mobly cannot deal with user config passing of ctypes objects,
# so we use this dict of uuid -> object to recover items stashed
# by reference.
_GLOBAL_DATA = {}


def stash_globally(o: object) -> str:
    id = str(uuid.uuid1())
    _GLOBAL_DATA[id] = o
    return id


def unstash_globally(id: str) -> Any:
    return _GLOBAL_DATA.get(id)


def default_paa_rootstore_from_root(root_path: pathlib.Path) -> Optional[pathlib.Path]:
    """Find PAA trust store following SDK convention at root_path."""
    start_path = root_path.resolve()
    cred_path = start_path.joinpath("credentials")
    dev_path = cred_path.joinpath("development")
    paa_path = dev_path.joinpath("paa-root-certs")

    return paa_path.resolve() if all([path.exists() for path in [cred_path, dev_path, paa_path]]) else None


def get_default_paa_trust_store(root_path: pathlib.Path) -> pathlib.Path:
    """Find PAA trust store starting at root_path."""
    cur_dir = pathlib.Path.cwd()
    max_levels = 10

    for level in range(max_levels):
        paa_trust_store_path = default_paa_rootstore_from_root(cur_dir)
        if paa_trust_store_path is not None:
            return paa_trust_store_path
        cur_dir = cur_dir.joinpath("..")
    else:
        return pathlib.Path.cwd()


def populate_commissioning_args(args: argparse.Namespace, config: MatterTestConfig) -> bool:
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

    if args.qr_code != [] and args.manual_code != []:
        print("error: Cannot have both --qr-code and --manual-code present!")
        return False

    if len(config.discriminators) != len(config.setup_passcodes):
        print("error: supplied number of discriminators does not match number of passcodes")
        return False

    device_descriptors = config.qr_code_content + config.manual_code + config.discriminators

    if not config.dut_node_ids:
        config.dut_node_ids = [_DEFAULT_DUT_NODE_ID]

    if args.commissioning_method is None:
        return True

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

    if config.commissioning_method == "ble-wifi":
        if args.wifi_ssid is None:
            print("error: missing --wifi-ssid <SSID> for --commissioning-method ble-wifi!")
            return False

        if args.wifi_passphrase is None:
            print("error: missing --wifi-passphrase <passphrasse> for --commissioning-method ble-wifi!")
            return False

        config.wifi_ssid = args.wifi_ssid
        config.wifi_passphrase = args.wifi_passphrase
    elif config.commissioning_method == "ble-thread":
        if args.thread_dataset_hex is None:
            print("error: missing --thread-dataset-hex <DATASET_HEX> for --commissioning-method ble-thread!")
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


def convert_args_to_matter_config(args: argparse.Namespace) -> MatterTestConfig:
    config = MatterTestConfig()

    # Populate commissioning config if present, exiting on error
    if not populate_commissioning_args(args, config):
        sys.exit(1)

    config.storage_path = pathlib.Path(_DEFAULT_STORAGE_PATH) if args.storage_path is None else args.storage_path
    config.logs_path = pathlib.Path(_DEFAULT_LOG_PATH) if args.logs_path is None else args.logs_path
    config.paa_trust_store_path = args.paa_trust_store_path
    config.ble_interface_id = args.ble_interface_id
    config.pics = {} if args.PICS is None else read_pics_from_file(args.PICS)
    config.tests = list(chain.from_iterable(args.tests or []))
    config.timeout = args.timeout  # This can be none, we pull the default from the test if it's unspecified
    config.endpoint = args.endpoint  # This can be None, the get_endpoint function allows the tests to supply a default
    config.app_pid = 0 if args.app_pid is None else args.app_pid
    config.fail_on_skipped_tests = args.fail_on_skipped

    config.controller_node_id = args.controller_node_id
    config.trace_to = args.trace_to

    config.tc_version_to_simulate = args.tc_version_to_simulate
    config.tc_user_response_to_simulate = args.tc_user_response_to_simulate

    # Accumulate all command-line-passed named args
    all_global_args = []
    argsets = [item for item in (args.int_arg, args.float_arg, args.string_arg, args.json_arg,
                                 args.hex_arg, args.bool_arg) if item is not None]
    for argset in chain.from_iterable(argsets):
        all_global_args.extend(argset)

    config.global_test_params = {}
    for name, value in all_global_args:
        config.global_test_params[name] = value

    # Embed the rest of the config in the global test params dict which will be passed to Mobly tests
    config.global_test_params["meta_config"] = {k: v for k, v in dataclass_asdict(config).items() if k != "global_test_params"}

    return config


def parse_matter_test_args(argv: Optional[List[str]] = None) -> MatterTestConfig:
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
    basic_group.add_argument('--ble-interface-id', action="store", type=int,
                             metavar="INTERFACE_ID", help="ID of BLE adapter (from hciconfig)")
    basic_group.add_argument('-N', '--controller-node-id', type=int_decimal_or_hex,
                             metavar='NODE_ID',
                             default=_DEFAULT_CONTROLLER_NODE_ID,
                             help='NodeID to use for initial/default controller (default: %d)' % _DEFAULT_CONTROLLER_NODE_ID)
    basic_group.add_argument('-n', '--dut-node-id', '--nodeId', type=int_decimal_or_hex,
                             metavar='NODE_ID', dest='dut_node_ids', default=[],
                             help='Node ID for primary DUT communication, '
                             'and NodeID to assign if commissioning (default: %d)' % _DEFAULT_DUT_NODE_ID, nargs="+")
    basic_group.add_argument('--endpoint', type=int, default=None, help="Endpoint under test")
    basic_group.add_argument('--app-pid', type=int, default=0, help="The PID of the app against which the test is going to run")
    basic_group.add_argument('--timeout', type=int, help="Test timeout in seconds")
    basic_group.add_argument("--PICS", help="PICS file path", type=str)

    commission_group = parser.add_argument_group(title="Commissioning", description="Arguments to commission a node")

    commission_group.add_argument('-m', '--commissioning-method', type=str,
                                  metavar='METHOD_NAME',
                                  choices=["on-network", "ble-wifi", "ble-thread", "on-network-ip"],
                                  help='Name of commissioning method to use')
    commission_group.add_argument('--in-test-commissioning-method', type=str,
                                  metavar='METHOD_NAME',
                                  choices=["on-network", "ble-wifi", "ble-thread", "on-network-ip"],
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
    commission_group.add_argument('-i', '--ip-addr', type=str,
                                  metavar='RAW_IP_ADDRESS',
                                  help='IP address to use (only for method "on-network-ip". ONLY FOR LOCAL TESTING!', nargs="+")

    commission_group.add_argument('--wifi-ssid', type=str,
                                  metavar='SSID',
                                  help='Wi-Fi SSID for ble-wifi commissioning')
    commission_group.add_argument('--wifi-passphrase', type=str,
                                  metavar='PASSPHRASE',
                                  help='Wi-Fi passphrase for ble-wifi commissioning')

    commission_group.add_argument('--thread-dataset-hex', type=byte_string_from_hex,
                                  metavar='OPERATIONAL_DATASET_HEX',
                                  help='Thread operational dataset as a hex string for ble-thread commissioning')

    commission_group.add_argument('--admin-vendor-id', action="store", type=int_decimal_or_hex, default=_DEFAULT_ADMIN_VENDOR_ID,
                                  metavar="VENDOR_ID",
                                  help="VendorID to use during commissioning (default 0x%04X)" % _DEFAULT_ADMIN_VENDOR_ID)
    commission_group.add_argument('--case-admin-subject', action="store", type=int_decimal_or_hex,
                                  metavar="CASE_ADMIN_SUBJECT",
                                  help="Set the CASE admin subject to an explicit value (default to commissioner Node ID)")

    commission_group.add_argument('--commission-only', action="store_true", default=False,
                                  help="If true, test exits after commissioning without running subsequent tests")

    commission_group.add_argument('--tc-version-to-simulate', type=int, help="Terms and conditions version")

    commission_group.add_argument('--tc-user-response-to-simulate', type=int, help="Terms and conditions acknowledgements")

    code_group = parser.add_mutually_exclusive_group(required=False)

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
                              metavar='ROOT_INDEX_OR_NAME', default=_DEFAULT_TRUST_ROOT_INDEX,
                              help='Root of trust under which to operate/commission for single-fabric basic usage. '
                              'alpha/beta/gamma are aliases for 1/2/3. Default (%d)' % _DEFAULT_TRUST_ROOT_INDEX)

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

    return convert_args_to_matter_config(parser.parse_known_args(argv)[0])


def generate_mobly_test_config(matter_test_config: MatterTestConfig):
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


def _find_test_class():
    """Finds the test class in a test script.
    Walk through module members and find the subclass of MatterBaseTest. Only
    one subclass is allowed in a test script.
    Returns:
      The test class in the test module.
    Raises:
      SystemExit: Raised if the number of test classes is not exactly one.
    """
    subclasses = utils.find_subclasses_in_module([MatterBaseTest], sys.modules['__main__'])
    subclasses = [c for c in subclasses if c.__name__ != "MatterBaseTest"]
    if len(subclasses) != 1:
        print(
            'Exactly one subclass of `MatterBaseTest` should be in the main file. Found %s.' %
            str([subclass.__name__ for subclass in subclasses]))
        sys.exit(1)

    return subclasses[0]


def get_test_info(test_class: MatterBaseTest, matter_test_config: MatterTestConfig) -> list[TestInfo]:
    test_config = generate_mobly_test_config(matter_test_config)
    base = test_class(test_config)

    if len(matter_test_config.tests) > 0:
        tests = matter_test_config.tests
    else:
        tests = base.get_existing_test_names()

    info = []
    for t in tests:
        info.append(TestInfo(t, steps=base.get_test_steps(t), desc=base.get_test_desc(t), pics=base.get_test_pics(t)))

    return info


def run_tests_no_exit(test_class: MatterBaseTest, matter_test_config: MatterTestConfig,
                      event_loop: asyncio.AbstractEventLoop, hooks: TestRunnerHooks,
                      default_controller=None, external_stack=None) -> bool:

    # NOTE: It's not possible to pass event loop via Mobly TestRunConfig user params, because the
    #       Mobly deep copies the user params before passing them to the test class and the event
    #       loop is not serializable. So, we are setting the event loop as a test class member.
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

        test_config.user_params["matter_stack"] = stash_globally(stack)

        # TODO: Steer to right FabricAdmin!
        # TODO: If CASE Admin Subject is a CAT tag range, then make sure to issue NOC with that CAT tag
        if not default_controller:
            default_controller = stack.certificate_authorities[0].adminList[0].NewController(
                nodeId=matter_test_config.controller_node_id,
                paaTrustStorePath=str(matter_test_config.paa_trust_store_path),
                catTags=matter_test_config.controller_cat_tags
            )
        test_config.user_params["default_controller"] = stash_globally(default_controller)

        test_config.user_params["matter_test_config"] = stash_globally(matter_test_config)
        test_config.user_params["hooks"] = stash_globally(hooks)

        # Execute the test class with the config
        ok = True

        test_config.user_params["certificate_authority_manager"] = stash_globally(stack.certificate_authority_manager)

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
                # Mobly gives the test run time in seconds, lets be a bit more precise
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
                logging.exception('Exception when executing %s.', test_config.testbed_name)
                ok = False

    if hooks:
        duration = (datetime.now(timezone.utc) - runner_start_time) / timedelta(microseconds=1)
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


def run_tests(test_class: MatterBaseTest, matter_test_config: MatterTestConfig,
              hooks: TestRunnerHooks, default_controller=None, external_stack=None) -> None:
    with asyncio.Runner() as runner:
        if not run_tests_no_exit(test_class, matter_test_config, runner.get_loop(),
                                 hooks, default_controller, external_stack):
            sys.exit(1)


def default_matter_test_main():
    """Execute the test class in a test module.
    This is the default entry point for running a test script file directly.
    In this case, only one test class in a test script is allowed.
    To make your test script executable, add the following to your file:
    .. code-block:: python
      from chip.testing.runner import default_matter_test_main
      ...
      if __name__ == '__main__':
        default_matter_test_main()
    """

    matter_test_config = parse_matter_test_args()

    # Find the test class in the test script.
    test_class = _find_test_class()

    hooks = InternalTestRunnerHooks()

    run_tests(test_class, matter_test_config, hooks)


class InternalTestRunnerHooks(TestRunnerHooks):

    def start(self, count: int):
        logging.info(f'Starting test set, running {count} tests')

    def stop(self, duration: int):
        logging.info(f'Finished test set, ran for {duration}ms')

    def test_start(self, filename: str, name: str, count: int, steps: list[str] = []):
        logging.info(f'Starting test from {filename}: {name} - {count} steps')

    def test_stop(self, exception: Exception, duration: int):
        logging.info(f'Finished test in {duration}ms')

    def step_skipped(self, name: str, expression: str):
        # TODO: Do we really need the expression as a string? We can evaluate this in code very easily
        logging.info(f'\t\t**** Skipping: {name}')

    def step_start(self, name: str):
        # The way I'm calling this, the name is already includes the step number, but it seems like it might be good to separate these
        logging.info(f'\t\t***** Test Step {name}')

    def step_success(self, logger, logs, duration: int, request):
        pass

    def step_failure(self, logger, logs, duration: int, request, received):
        # TODO: there's supposed to be some kind of error message here, but I have no idea where it's meant to come from in this API
        logging.info('\t\t***** Test Failure : ')

    def step_unknown(self):
        """
        This method is called when the result of running a step is unknown. For example during a dry-run.
        """
        pass

    def show_prompt(self,
                    msg: str,
                    placeholder: Optional[str] = None,
                    default_value: Optional[str] = None) -> None:
        pass

    def test_skipped(self, filename: str, name: str):
        logging.info(f"Skipping test from {filename}: {name}")


class MatterStackState:
    def __init__(self, config: MatterTestConfig):
        self._logger = logger
        self._config = config

        if not hasattr(builtins, "chipStack"):
            chip.native.Init(bluetoothAdapter=config.ble_interface_id)
            if config.storage_path is None:
                raise ValueError("Must have configured a MatterTestConfig.storage_path")
            self._init_stack(already_initialized=False, persistentStoragePath=config.storage_path)
            self._we_initialized_the_stack = True
        else:
            self._init_stack(already_initialized=True)
            self._we_initialized_the_stack = False

    def _init_stack(self, already_initialized: bool, **kwargs):
        if already_initialized:
            self._chip_stack = builtins.chipStack
            self._logger.warn(
                "Re-using existing ChipStack object found in current interpreter: "
                "storage path %s will be ignored!" % (self._config.storage_path)
            )
            # TODO: Warn that storage will not follow what we set in config
        else:
            self._chip_stack = ChipStack(**kwargs)
            builtins.chipStack = self._chip_stack

        chip.logging.RedirectToPythonLogging()

        self._storage = self._chip_stack.GetStorageManager()
        self._certificate_authority_manager = chip.CertificateAuthority.CertificateAuthorityManager(chipStack=self._chip_stack)
        self._certificate_authority_manager.LoadAuthoritiesFromStorage()

        if (len(self._certificate_authority_manager.activeCaList) == 0):
            self._logger.warn(
                "Didn't find any CertificateAuthorities in storage -- creating a new CertificateAuthority + FabricAdmin...")
            ca = self._certificate_authority_manager.NewCertificateAuthority(caIndex=self._config.root_of_trust_index)
            ca.maximizeCertChains = self._config.maximize_cert_chains
            ca.certificateValidityPeriodSec = self._config.certificate_validity_period
            ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=self._config.fabric_id)
        elif (len(self._certificate_authority_manager.activeCaList[0].adminList) == 0):
            self._logger.warn("Didn't find any FabricAdmins in storage -- creating a new one...")
            self._certificate_authority_manager.activeCaList[0].NewFabricAdmin(vendorId=0xFFF1, fabricId=self._config.fabric_id)

    # TODO: support getting access to chip-tool credentials issuer's data

    def Shutdown(self):
        if self._we_initialized_the_stack:
            # Unfortunately, all the below are singleton and possibly
            # managed elsewhere so we have to be careful not to touch unless
            # we initialized ourselves.
            self._certificate_authority_manager.Shutdown()
            global_chip_stack = builtins.chipStack
            global_chip_stack.Shutdown()

    @property
    def certificate_authorities(self):
        return self._certificate_authority_manager.activeCaList

    @property
    def certificate_authority_manager(self):
        return self._certificate_authority_manager

    @property
    def storage(self) -> PersistentStorage:
        return self._storage

    @property
    def stack(self) -> ChipStack:
        return builtins.chipStack


class AsyncMock(MagicMock):
    async def __call__(self, *args, **kwargs):
        return super(AsyncMock, self).__call__(*args, **kwargs)


class MockTestRunner():

    def __init__(self, abs_filename: str, classname: str, test: str, endpoint: int = None,
                 pics: dict[str, bool] = None, paa_trust_store_path=None):
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

    def set_test_config(self, test_config: MatterTestConfig = MatterTestConfig()):
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
        with asyncio.Runner() as runner:
            return run_tests_no_exit(self.test_class, self.config, runner.get_loop(),
                                     hooks, self.default_controller, self.stack)

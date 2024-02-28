#
#    Copyright (c) 2022 Project CHIP Authors
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
import glob
import inspect
import json
import logging
import os
import pathlib
import queue
import random
import re
import sys
import typing
import uuid
import xml.etree.ElementTree as ET
from binascii import hexlify, unhexlify
from dataclasses import asdict as dataclass_asdict
from dataclasses import dataclass, field
from datetime import datetime, timedelta, timezone
from enum import Enum
from typing import List, Optional, Tuple, Union

from chip.tlv import float32, uint

# isort: off

from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
import chip.FabricAdmin  # Needed before chip.CertificateAuthority
import chip.CertificateAuthority
from chip.ChipDeviceCtrl import CommissioningParameters

# isort: on
import chip.clusters as Clusters
import chip.logging
import chip.native
from chip import discovery
from chip.ChipStack import ChipStack
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from chip.setup_payload import SetupPayload
from chip.storage import PersistentStorage
from chip.tracing import TracingContext
from global_attribute_ids import GlobalAttributeIds
from mobly import asserts, base_test, signals, utils
from mobly.config_parser import ENV_MOBLY_LOGPATH, TestRunConfig
from mobly.test_runner import TestRunner

try:
    from matter_yamltests.hooks import TestRunnerHooks
except ImportError:
    class TestRunnerHooks:
        pass


# TODO: Add utility to commission a device if needed
# TODO: Add utilities to keep track of controllers/fabrics

logger = logging.getLogger("matter.python_testing")
logger.setLevel(logging.INFO)

DiscoveryFilterType = ChipDeviceCtrl.DiscoveryFilterType

_DEFAULT_ADMIN_VENDOR_ID = 0xFFF1
_DEFAULT_STORAGE_PATH = "admin_storage.json"
_DEFAULT_LOG_PATH = "/tmp/matter_testing/logs"
_DEFAULT_CONTROLLER_NODE_ID = 112233
_DEFAULT_DUT_NODE_ID = 0x12344321
_DEFAULT_TRUST_ROOT_INDEX = 1

# Mobly cannot deal with user config passing of ctypes objects,
# so we use this dict of uuid -> object to recover items stashed
# by reference.
_GLOBAL_DATA = {}


def stash_globally(o: object) -> str:
    id = str(uuid.uuid1())
    _GLOBAL_DATA[id] = o
    return id


def unstash_globally(id: str) -> object:
    return _GLOBAL_DATA.get(id)


def default_paa_rootstore_from_root(root_path: pathlib.Path) -> Optional[pathlib.Path]:
    """Attempt to find a PAA trust store following SDK convention at `root_path`

    This attempts to find {root_path}/credentials/development/paa-root-certs.

    Returns the fully resolved path on success or None if not found.
    """
    start_path = root_path.resolve()
    cred_path = start_path.joinpath("credentials")
    dev_path = cred_path.joinpath("development")
    paa_path = dev_path.joinpath("paa-root-certs")

    return paa_path.resolve() if all([path.exists() for path in [cred_path, dev_path, paa_path]]) else None


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


def parse_pics(lines: typing.List[str]) -> dict[str, bool]:
    pics = {}
    for raw in lines:
        line, _, _ = raw.partition("#")
        line = line.strip()

        if not line:
            continue

        key, _, val = line.partition("=")
        val = val.strip()
        if val not in ["1", "0"]:
            raise ValueError('PICS {} must have a value of 0 or 1'.format(key))

        pics[key.strip()] = (val == "1")
    return pics


def parse_pics_xml(contents: str) -> dict[str, bool]:
    pics = {}
    mytree = ET.fromstring(contents)
    for pi in mytree.iter('picsItem'):
        name = pi.find('itemNumber').text
        support = pi.find('support').text
        pics[name] = int(json.loads(support.lower())) == 1
    return pics


def read_pics_from_file(path: str) -> dict[str, bool]:
    """ Reads a dictionary of PICS from a file (ci format) or directory (xml format). """
    if os.path.isdir(os.path.abspath(path)):
        pics_dict = {}
        for filename in glob.glob(f'{path}/*.xml'):
            with open(filename, 'r') as f:
                contents = f.read()
                pics_dict.update(parse_pics_xml(contents))
        return pics_dict

    else:
        with open(path, 'r') as f:
            lines = f.readlines()
            return parse_pics(lines)


def type_matches(received_value, desired_type):
    """ Checks if the value received matches the expected type.

        Handles unpacking Nullable and Optional types and
        compares list value types for non-empty lists.
    """
    if typing.get_origin(desired_type) == typing.Union:
        return any(type_matches(received_value, t) for t in typing.get_args(desired_type))
    elif typing.get_origin(desired_type) == list:
        if isinstance(received_value, list):
            # Assume an empty list is of the correct type
            return True if received_value == [] else any(type_matches(received_value[0], t) for t in typing.get_args(desired_type))
        else:
            return False
    elif desired_type == uint:
        return isinstance(received_value, int) and received_value >= 0
    elif desired_type == float32:
        return isinstance(received_value, float)
    else:
        return isinstance(received_value, desired_type)

# TODO(#31177): Need to add unit tests for all time conversion methods.


def utc_time_in_matter_epoch(desired_datetime: Optional[datetime] = None):
    """ Returns the time in matter epoch in us.

        If desired_datetime is None, it will return the current time.
    """
    if desired_datetime is None:
        utc_native = datetime.now(tz=timezone.utc)
    else:
        utc_native = desired_datetime
    # Matter epoch is 0 hours, 0 minutes, 0 seconds on Jan 1, 2000 UTC
    utc_th_delta = utc_native - datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)
    utc_th_us = int(utc_th_delta.total_seconds() * 1000000)
    return utc_th_us


matter_epoch_us_from_utc_datetime = utc_time_in_matter_epoch


def utc_datetime_from_matter_epoch_us(matter_epoch_us: int) -> datetime:
    """Returns the given Matter epoch time as a usable Python datetime in UTC."""
    delta_from_epoch = timedelta(microseconds=matter_epoch_us)
    matter_epoch = datetime(2000, 1, 1, 0, 0, 0, 0, timezone.utc)

    return matter_epoch + delta_from_epoch


def utc_datetime_from_posix_time_ms(posix_time_ms: int) -> datetime:
    millis = posix_time_ms % 1000
    seconds = posix_time_ms // 1000
    return datetime.fromtimestamp(seconds, timezone.utc) + timedelta(milliseconds=millis)


def compare_time(received: int, offset: timedelta = timedelta(), utc: int = None, tolerance: timedelta = timedelta(seconds=5)) -> None:
    if utc is None:
        utc = utc_time_in_matter_epoch()

    # total seconds includes fractional for microseconds
    expected = utc + offset.total_seconds() * 1000000
    delta_us = abs(expected - received)
    delta = timedelta(microseconds=delta_us)
    asserts.assert_less_equal(delta, tolerance, "Received time is out of tolerance")


def get_wait_seconds_from_set_time(set_time_matter_us: int, wait_seconds: int):
    seconds_passed = (utc_time_in_matter_epoch() - set_time_matter_us) // 1000000
    return wait_seconds - seconds_passed


class SimpleEventCallback:
    def __init__(self, name: str, expected_cluster_id: int, expected_event_id: int, output_queue: queue.SimpleQueue):
        self._name = name
        self._expected_cluster_id = expected_cluster_id
        self._expected_event_id = expected_event_id
        self._output_queue = output_queue

    def __call__(self, event_result: EventReadResult, transaction: SubscriptionTransaction):
        if (self._expected_cluster_id == event_result.Header.ClusterId and
                self._expected_event_id == event_result.Header.EventId):
            self._output_queue.put(event_result)

    @property
    def name(self) -> str:
        return self._name


class EventChangeCallback:
    def __init__(self, expected_cluster: ClusterObjects):
        """This class creates a queue to store received event callbacks, that can be checked by the test script
           expected_cluster: is the cluster from which the events are expected
        """
        self._q = queue.Queue()
        self._expected_cluster = expected_cluster

    async def start(self, dev_ctrl, node_id: int, endpoint: int):
        """This starts a subscription for events on the specified node_id and endpoint. The cluster is specified when the class instance is created."""
        self._subscription = await dev_ctrl.ReadEvent(node_id,
                                                      events=[(endpoint, self._expected_cluster, True)], reportInterval=(1, 5),
                                                      fabricFiltered=False, keepSubscriptions=True, autoResubscribe=False)
        self._subscription.SetEventUpdateCallback(self.__call__)

    def __call__(self, res: EventReadResult, transaction: SubscriptionTransaction):
        """This is the subscription callback when an event is received.
           It checks the event is from the expected_cluster and then posts it into the queue for later processing."""
        if res.Status == Status.Success and res.Header.ClusterId == self._expected_cluster.id:
            logging.info(
                f'Got subscription report for event on cluster {self._expected_cluster}: {res.Data}')
            self._q.put(res)

    def wait_for_event_report(self, expected_event: ClusterObjects.ClusterEvent, timeout: int = 10):
        """This function allows a test script to block waiting for the specific event to arrive with a timeout.
           It returns the event data so that the values can be checked."""
        try:
            res = self._q.get(block=True, timeout=timeout)
        except queue.Empty:
            asserts.fail("Failed to receive a report for the event {}".format(expected_event))

        asserts.assert_equal(res.Header.ClusterId, expected_event.cluster_id, "Expected cluster ID not found in event report")
        asserts.assert_equal(res.Header.EventId, expected_event.event_id, "Expected event ID not found in event report")
        return res.Data


class InternalTestRunnerHooks(TestRunnerHooks):

    def start(self, count: int):
        logging.info(f'Starting test set, running {count} tests')

    def stop(self, duration: int):
        logging.info(f'Finished test set, ran for {duration}ms')

    def test_start(self, filename: str, name: str, count: int):
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


@dataclass
class MatterTestConfig:
    storage_path: pathlib.Path = pathlib.Path(".")
    logs_path: pathlib.Path = pathlib.Path(".")
    paa_trust_store_path: Optional[pathlib.Path] = None
    ble_interface_id: Optional[int] = None
    commission_only: bool = False

    admin_vendor_id: int = _DEFAULT_ADMIN_VENDOR_ID
    case_admin_subject: Optional[int] = None
    global_test_params: dict = field(default_factory=dict)
    # List of explicit tests to run by name. If empty, all tests will run
    tests: List[str] = field(default_factory=list)
    timeout: typing.Union[int, None] = None
    endpoint: int = 0
    app_pid: int = 0

    commissioning_method: Optional[str] = None
    discriminators: Optional[List[int]] = None
    setup_passcodes: Optional[List[int]] = None
    commissionee_ip_address_just_for_testing: Optional[str] = None
    maximize_cert_chains: bool = False

    qr_code_content: Optional[str] = None
    manual_code: Optional[str] = None

    wifi_ssid: Optional[str] = None
    wifi_passphrase: Optional[str] = None
    thread_operational_dataset: Optional[str] = None

    pics: dict[bool, str] = field(default_factory=dict)

    # Node ID for basic DUT
    dut_node_ids: Optional[List[int]] = None
    # Node ID to use for controller/commissioner
    controller_node_id: int = _DEFAULT_CONTROLLER_NODE_ID
    # CAT Tags for default controller/commissioner
    controller_cat_tags: List[int] = field(default_factory=list)

    # Fabric ID which to use
    fabric_id: int = 1

    # "Alpha" by default
    root_of_trust_index: int = _DEFAULT_TRUST_ROOT_INDEX

    # If this is set, we will reuse root of trust keys at that location
    chip_tool_credentials_path: Optional[pathlib.Path] = None

    trace_to: List[str] = field(default_factory=list)


class ClusterMapper:
    """Describe clusters/attributes using schema names."""

    def __init__(self, legacy_cluster_mapping) -> None:
        self._mapping = legacy_cluster_mapping

    def get_cluster_string(self, cluster_id: int) -> str:
        mapping = self._mapping._CLUSTER_ID_DICT.get(cluster_id, None)
        if not mapping:
            return f"Cluster Unknown ({cluster_id}, 0x{cluster_id:08X})"
        else:
            name = mapping["clusterName"]
            return f"Cluster {name} ({cluster_id}, 0x{cluster_id:04X})"

    def get_attribute_string(self, cluster_id: int, attribute_id) -> str:
        global_attrs = [item.value for item in GlobalAttributeIds]
        if attribute_id in global_attrs:
            return f"Attribute {GlobalAttributeIds(attribute_id).to_name()} {attribute_id}, 0x{attribute_id:04X}"
        mapping = self._mapping._CLUSTER_ID_DICT.get(cluster_id, None)
        if not mapping:
            return f"Attribute Unknown ({attribute_id}, 0x{attribute_id:08X})"
        else:
            attribute_mapping = mapping["attributes"].get(attribute_id, None)

            if not attribute_mapping:
                return f"Attribute Unknown ({attribute_id}, 0x{attribute_id:08X})"
            else:
                attribute_name = attribute_mapping["attributeName"]
                return f"Attribute {attribute_name} ({attribute_id}, 0x{attribute_id:04X})"


def id_str(id):
    return f'{id} (0x{id:02x})'


def cluster_id_str(id):
    if id in Clusters.ClusterObjects.ALL_CLUSTERS.keys():
        s = Clusters.ClusterObjects.ALL_CLUSTERS[id].__name__
    else:
        s = "Unknown cluster"
    try:
        return f'{id_str(id)} {s}'
    except TypeError:
        return 'HERE IS THE PROBLEM'


@dataclass
class CustomCommissioningParameters:
    commissioningParameters: CommissioningParameters
    randomDiscriminator: int


@dataclass
class AttributePathLocation:
    endpoint_id: int
    cluster_id: Optional[int] = None
    attribute_id: Optional[int] = None

    def as_cluster_string(self, mapper: ClusterMapper):
        desc = f"Endpoint {self.endpoint_id}"
        if self.cluster_id is not None:
            desc += f", {mapper.get_cluster_string(self.cluster_id)}"
        return desc

    def as_string(self, mapper: ClusterMapper):
        desc = self.as_cluster_string(mapper)
        if self.cluster_id is not None and self.attribute_id is not None:
            desc += f", {mapper.get_attribute_string(self.cluster_id, self.attribute_id)}"

        return desc

    def __str__(self):
        return (f'\n        Endpoint: {self.endpoint_id},'
                f'\n        Cluster:  {cluster_id_str(self.cluster_id)},'
                f'\n        Attribute:{id_str(self.attribute_id)}')


@dataclass
class EventPathLocation:
    endpoint_id: int
    cluster_id: int
    event_id: int

    def __str__(self):
        return (f'\n        Endpoint: {self.endpoint_id},'
                f'\n        Cluster:  {cluster_id_str(self.cluster_id)},'
                f'\n        Event:    {id_str(self.event_id)}')


@dataclass
class CommandPathLocation:
    endpoint_id: int
    cluster_id: int
    command_id: int

    def __str__(self):
        return (f'\n        Endpoint: {self.endpoint_id},'
                f'\n        Cluster:  {cluster_id_str(self.cluster_id)},'
                f'\n        Command:  {id_str(self.command_id)}')


@dataclass
class ClusterPathLocation:
    endpoint_id: int
    cluster_id: int

    def __str__(self):
        return (f'\n       Endpoint: {self.endpoint_id},'
                f'\n       Cluster:  {cluster_id_str(self.cluster_id)}')


@dataclass
class FeaturePathLocation:
    endpoint_id: int
    cluster_id: int
    feature_code: str

    def __str__(self):
        return (f'\n        Endpoint: {self.endpoint_id},'
                f'\n        Cluster:  {cluster_id_str(self.cluster_id)},'
                f'\n        Feature:  {self.feature_code}')

# ProblemSeverity is not using StrEnum, but rather Enum, since StrEnum only
# appeared in 3.11. To make it JSON serializable easily, multiple inheritance
# from `str` is used. See https://stackoverflow.com/a/51976841.


class ProblemSeverity(str, Enum):
    NOTE = "NOTE"
    WARNING = "WARNING"
    ERROR = "ERROR"


@dataclass
class ProblemNotice:
    test_name: str
    location: Union[AttributePathLocation, EventPathLocation, CommandPathLocation, ClusterPathLocation, FeaturePathLocation]
    severity: ProblemSeverity
    problem: str
    spec_location: str = ""

    def __str__(self):
        return (f'\nProblem: {str(self.severity)}'
                f'\n    test_name: {self.test_name}'
                f'\n    location: {str(self.location)}'
                f'\n    problem: {self.problem}'
                f'\n    spec_location: {self.spec_location}\n')


@dataclass
class SetupPayloadInfo:
    filter_type: discovery.FilterType = discovery.FilterType.LONG_DISCRIMINATOR
    filter_value: int = 0
    passcode: int = 0


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


def bytes_from_hex(hex: str) -> bytes:
    """Converts any `hex` string representation including `01:ab:cd` to bytes

    Handles any whitespace including newlines, which are all stripped.
    """
    return unhexlify("".join(hex.replace(":", "").replace(" ", "").split()))


def hex_from_bytes(b: bytes) -> str:
    """Converts a bytes object `b` into a hex string (reverse of bytes_from_hex)"""
    return hexlify(b).decode("utf-8")


@dataclass
class TestStep:
    test_plan_number: typing.Union[int, str]
    description: str
    is_commissioning: bool = False


@dataclass
class TestInfo:
    function: str
    desc: str
    steps: list[TestStep]
    pics: list[str]


class MatterBaseTest(base_test.BaseTestClass):
    def __init__(self, *args):
        super().__init__(*args)

        # List of accumulated problems across all tests
        self.problems = []
        self.is_commissioning = False

    def get_test_steps(self, test: str) -> list[TestStep]:
        ''' Retrieves the test step list for the given test

            Test steps are defined in the function called steps_<functionname>.
            ex for test test_TC_TEST_1_1, the steps are in a function called
            steps_TC_TEST_1_1.

            Test that implement a steps_ function should call each step
            in order using self.step(number), where number is the test_plan_number
            from each TestStep.
        '''
        steps = self._get_defined_test_steps(test)
        return [TestStep(1, "Run entire test")] if steps is None else steps

    def _get_defined_test_steps(self, test: str) -> list[TestStep]:
        steps_name = 'steps_' + test[5:]
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

    def _get_defined_pics(self, test: str) -> list[TestStep]:
        steps_name = 'pics_' + test[5:]
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
        desc_name = 'desc_' + test[5:]
        try:
            fn = getattr(self, desc_name)
            return fn()
        except AttributeError:
            return test

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
    def default_controller(self) -> ChipDeviceCtrl:
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

    def setup_class(self):
        super().setup_class()

        # Mappings of cluster IDs to names and metadata.
        # TODO: Move to using non-generated code and rather use data model description (.matter or .xml)
        self.cluster_mapper = ClusterMapper(self.default_controller._Cluster)
        self.current_step_index = 0
        self.step_start_time = datetime.now(timezone.utc)
        self.step_skipped = False

    def setup_test(self):
        self.current_step_index = 0
        self.step_start_time = datetime.now(timezone.utc)
        self.step_skipped = False
        if self.runner_hook and not self.is_commissioning:
            test_name = self.current_test_info.name
            steps = self._get_defined_test_steps(test_name)
            num_steps = 1 if steps is None else len(steps)
            filename = inspect.getfile(self.__class__)
            desc = self.get_test_desc(test_name)
            self.runner_hook.test_start(filename=filename, name=desc, count=num_steps)
            # If we don't have defined steps, we're going to start the one and only step now
            # if there are steps defined by the test, rely on the test calling the step() function
            # to indicates how it is proceeding
            if steps is None:
                self.step(1)

    def teardown_class(self):
        """Final teardown after all tests: log all problems"""
        if len(self.problems) == 0:
            return

        logging.info("###########################################################")
        logging.info("Problems found:")
        logging.info("===============")
        for problem in self.problems:
            logging.info(str(problem))
        logging.info("###########################################################")

        super().teardown_class()

    def check_pics(self, pics_key: str) -> bool:
        picsd = self.matter_test_config.pics
        pics_key = pics_key.strip()
        return pics_key in picsd and picsd[pics_key]

    def openCommissioningWindow(self, dev_ctrl: ChipDeviceCtrl, node_id: int) -> CustomCommissioningParameters:
        rnd_discriminator = random.randint(0, 4095)
        try:
            commissioning_params = dev_ctrl.OpenCommissioningWindow(nodeid=node_id, timeout=900, iteration=1000,
                                                                    discriminator=rnd_discriminator, option=1)
            params = CustomCommissioningParameters(commissioning_params, rnd_discriminator)
            return params

        except InteractionModelError as e:
            asserts.fail(e.status, 'Failed to open commissioning window')

    async def read_single_attribute(
            self, dev_ctrl: ChipDeviceCtrl, node_id: int, endpoint: int, attribute: object, fabricFiltered: bool = True) -> object:
        result = await dev_ctrl.ReadAttribute(node_id, [(endpoint, attribute)], fabricFiltered=fabricFiltered)
        data = result[endpoint]
        return list(data.values())[0][attribute]

    async def read_single_attribute_check_success(
            self, cluster: Clusters.ClusterObjects.ClusterCommand, attribute: Clusters.ClusterObjects.ClusterAttributeDescriptor,
            dev_ctrl: ChipDeviceCtrl = None, node_id: int = None, endpoint: int = None, fabric_filtered: bool = True, assert_on_error: bool = True, test_name: str = "") -> object:
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        if endpoint is None:
            endpoint = self.matter_test_config.endpoint

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
            error: Status, dev_ctrl: ChipDeviceCtrl = None, node_id: int = None, endpoint: int = None,
            fabric_filtered: bool = True, assert_on_error: bool = True, test_name: str = "") -> object:
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        if endpoint is None:
            endpoint = self.matter_test_config.endpoint

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

    async def send_single_cmd(
            self, cmd: Clusters.ClusterObjects.ClusterCommand,
            dev_ctrl: ChipDeviceCtrl = None, node_id: int = None, endpoint: int = None,
            timedRequestTimeoutMs: typing.Union[None, int] = None) -> object:
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        if endpoint is None:
            endpoint = self.matter_test_config.endpoint

        result = await dev_ctrl.SendCommand(nodeid=node_id, endpoint=endpoint, payload=cmd, timedRequestTimeoutMs=timedRequestTimeoutMs)
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

    def record_error(self, test_name: str, location: Union[AttributePathLocation, EventPathLocation, CommandPathLocation, ClusterPathLocation, FeaturePathLocation], problem: str, spec_location: str = ""):
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.ERROR, problem, spec_location))

    def record_warning(self, test_name: str, location: Union[AttributePathLocation, EventPathLocation, CommandPathLocation, ClusterPathLocation, FeaturePathLocation], problem: str, spec_location: str = ""):
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.WARNING, problem, spec_location))

    def record_note(self, test_name: str, location: Union[AttributePathLocation, EventPathLocation, CommandPathLocation, ClusterPathLocation, FeaturePathLocation], problem: str, spec_location: str = ""):
        self.problems.append(ProblemNotice(test_name, location, ProblemSeverity.NOTE, problem, spec_location))

    def on_fail(self, record):
        ''' Called by Mobly on test failure

            record is of type TestResultRecord
        '''
        if self.runner_hook and not self.is_commissioning:
            exception = record.termination_signal.exception
            step_duration = (datetime.now(timezone.utc) - self.step_start_time) / timedelta(microseconds=1)
            # This isn't QUITE the test duration because the commissioning is handled separately, but it's clsoe enough for now
            # This is already given in milliseconds
            test_duration = record.end_time - record.begin_time
            # TODO: I have no idea what logger, logs, request or received are. Hope None works because I have nothing to give
            self.runner_hook.step_failure(logger=None, logs=None, duration=step_duration, request=None, received=None)
            self.runner_hook.test_stop(exception=exception, duration=test_duration)

    def on_pass(self, record):
        ''' Called by Mobly on test pass

            record is of type TestResultRecord
        '''
        if self.runner_hook and not self.is_commissioning:
            # What is request? This seems like an implementation detail for the runner
            # TODO: As with failure, I have no idea what logger, logs or request are meant to be
            step_duration = (datetime.now(timezone.utc) - self.step_start_time) / timedelta(microseconds=1)
            test_duration = record.end_time - record.begin_time
            self.runner_hook.step_success(logger=None, logs=None, duration=step_duration, request=None)

        # TODO: this check could easily be annoying when doing dev. flag it somehow? Ditto with the in-order check
        steps = self._get_defined_test_steps(record.test_name)
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

    def mark_current_step_skipped(self):
        try:
            steps = self.get_test_steps(self.current_test_info.name)
            if self.current_step_index == 0:
                asserts.fail("Script error: mark_current_step_skipped cannot be called before step()")
            num = steps[self.current_step_index-1].test_plan_number
        except KeyError:
            num = self.current_step_index

        if self.runner_hook:
            # TODO: what does name represent here? The wordy test name? The test plan number? The number and name?
            # TODO: I very much do not want to have people passing in strings here. Do we really need the expression
            #       as a string? Does it get used by the TH?
            self.runner_hook.step_skipped(name=str(num), expression="")
        else:
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

        if self.runner_hook:
            # If we've reached the next step with no assertion and the step wasn't skipped, it passed
            if not self.step_skipped and self.current_step_index != 0:
                # TODO: As with failure, I have no idea what loger, logs or request are meant to be
                step_duration = (datetime.now(timezone.utc) - self.step_start_time) / timedelta(microseconds=1)
                self.runner_hook.step_success(logger=None, logs=None, duration=step_duration, request=None)

            # TODO: it seems like the step start should take a number and a name
            name = f'{step} : {steps[self.current_step_index].description}'
            self.runner_hook.step_start(name=name)
        else:
            self.print_step(step, steps[self.current_step_index].description)

        self.step_start_time = datetime.now(tz=timezone.utc)
        self.current_step_index = self.current_step_index + 1
        self.step_skipped = False

    def get_setup_payload_info(self) -> SetupPayloadInfo:
        if self.matter_test_config.qr_code_content is not None:
            qr_code = self.matter_test_config.qr_code_content
            try:
                setup_payload = SetupPayload().ParseQrCode(qr_code)
            except ChipStackError:
                asserts.fail(f"QR code '{qr_code} failed to parse properly as a Matter setup code.")

        elif self.matter_test_config.manual_code is not None:
            manual_code = self.matter_test_config.manual_code
            try:
                setup_payload = SetupPayload().ParseManualPairingCode(manual_code)
            except ChipStackError:
                asserts.fail(
                    f"Manual code code '{manual_code}' failed to parse properly as a Matter setup code. Check that all digits are correct and length is 11 or 21 characters.")
        else:
            asserts.fail("Require either --qr-code or --manual-code.")

        info = SetupPayloadInfo()
        info.passcode = setup_payload.setup_passcode
        if setup_payload.short_discriminator is not None:
            info.filter_type = discovery.FilterType.SHORT_DISCRIMINATOR
            info.filter_value = setup_payload.short_discriminator
        else:
            info.filter_type = discovery.FilterType.LONG_DISCRIMINATOR
            info.filter_value = setup_payload.long_discriminator

        return info

    def wait_for_user_input(self,
                            prompt_msg: str,
                            input_msg: str = "Press Enter when done.\n",
                            prompt_msg_placeholder: str = "Submit anything to continue",
                            default_value: str = "y") -> str:
        """Ask for user input and wait for it.

        Args:
            prompt_msg (str): Message for TH UI prompt. Indicates what is expected from the user.
            input_msg (str, optional): Prompt for input function, used when running tests manually. Defaults to "Press Enter when done.\n".
            prompt_msg_placeholder (str, optional): TH UI prompt input placeholder. Defaults to "Submit anything to continue".
            default_value (str, optional): TH UI prompt default value. Defaults to "y".

        Returns:
            str: User input
        """
        if self.runner_hook:
            self.runner_hook.show_prompt(msg=prompt_msg,
                                         placeholder=prompt_msg_placeholder,
                                         default_value=default_value)
        return input(input_msg)


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
    regex = r"^(?P<name>[a-zA-Z_0-9.]+):((?P<hex_value>0x[0-9a-fA-F_]+)|(?P<decimal_value>-?\d+))$"
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
    match = re.match(regex, s.lower())
    if not match:
        raise ValueError("Invalid bool argument format, does not match %s" % regex)

    name = match.group("name")
    if match.group("truth_value"):
        value = True if match.group("truth_value") == "true" else False
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
    config.commission_only = args.commission_only

    # TODO: this should also allow multiple once QR and manual codes are supported.
    config.qr_code_content = args.qr_code
    if args.manual_code:
        config.manual_code = args.manual_code
    else:
        config.manual_code = None

    if args.commissioning_method is None:
        return True

    if args.discriminators is None and (args.qr_code is None and args.manual_code is None):
        print("error: Missing --discriminator when no --qr-code/--manual-code present!")
        return False
    config.discriminators = args.discriminators

    if args.passcodes is None and (args.qr_code is None and args.manual_code is None):
        print("error: Missing --passcode when no --qr-code/--manual-code present!")
        return False
    config.setup_passcodes = args.passcodes

    if args.qr_code is not None and args.manual_code is not None:
        print("error: Cannot have both --qr-code and --manual-code present!")
        return False

    if len(config.discriminators) != len(config.setup_passcodes):
        print("error: supplied number of discriminators does not match number of passcodes")
        return False

    device_descriptors = [config.qr_code_content] if config.qr_code_content is not None else [
        config.manual_code] if config.manual_code is not None else config.discriminators

    if len(config.dut_node_ids) > len(device_descriptors):
        print("error: More node IDs provided than discriminators")
        return False

    if len(config.dut_node_ids) < len(device_descriptors):
        missing = len(device_descriptors) - len(config.dut_node_ids)
        # We generate new node IDs sequentially from the last one seen for all
        # missing NodeIDs when commissioning many nodes at once.
        for i in range(missing):
            config.dut_node_ids.append(config.dut_node_ids[-1] + 1)

    if len(config.dut_node_ids) != len(set(config.dut_node_ids)):
        print("error: Duplicate values in node id list")
        return False

    if len(config.discriminators) != len(set(config.discriminators)):
        print("error: Duplicate value in discriminator list")
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
    config.tests = [] if args.tests is None else args.tests
    config.timeout = args.timeout  # This can be none, we pull the default from the test if it's unspecified
    config.endpoint = 0 if args.endpoint is None else args.endpoint
    config.app_pid = 0 if args.app_pid is None else args.app_pid

    config.controller_node_id = args.controller_node_id
    config.trace_to = args.trace_to

    # Accumulate all command-line-passed named args
    all_global_args = []
    argsets = [item for item in (args.int_arg, args.float_arg, args.string_arg, args.json_arg,
                                 args.hex_arg, args.bool_arg) if item is not None]
    for argset in argsets:
        all_global_args.extend(argset)

    config.global_test_params = {}
    for name, value in all_global_args:
        config.global_test_params[name] = value

    # Embed the rest of the config in the global test params dict which will be passed to Mobly tests
    config.global_test_params["meta_config"] = {k: v for k, v in dataclass_asdict(config).items() if k != "global_test_params"}

    return config


def parse_matter_test_args(argv: List[str]) -> MatterTestConfig:
    parser = argparse.ArgumentParser(description='Matter standalone Python test')

    basic_group = parser.add_argument_group(title="Basic arguments", description="Overall test execution arguments")

    basic_group.add_argument('--tests',
                             '--test_case',
                             action="store",
                             nargs='+',
                             type=str,
                             metavar='test_a test_b...',
                             help='A list of tests in the test class to execute.')
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
                             metavar='NODE_ID', dest='dut_node_ids', default=[_DEFAULT_DUT_NODE_ID],
                             help='Node ID for primary DUT communication, '
                             'and NodeID to assign if commissioning (default: %d)' % _DEFAULT_DUT_NODE_ID, nargs="+")
    basic_group.add_argument('--endpoint', type=int, default=0, help="Endpoint under test")
    basic_group.add_argument('--app-pid', type=int, default=0, help="The PID of the app against which the test is going to run")
    basic_group.add_argument('--timeout', type=int, help="Test timeout in seconds")
    basic_group.add_argument("--PICS", help="PICS file path", type=str)

    commission_group = parser.add_argument_group(title="Commissioning", description="Arguments to commission a node")

    commission_group.add_argument('-m', '--commissioning-method', type=str,
                                  metavar='METHOD_NAME',
                                  choices=["on-network", "ble-wifi", "ble-thread", "on-network-ip"],
                                  help='Name of commissioning method to use')
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

    code_group = parser.add_mutually_exclusive_group(required=False)

    code_group.add_argument('-q', '--qr-code', type=str,
                            metavar="QR_CODE", help="QR setup code content (overrides passcode and discriminator)")
    code_group.add_argument('--manual-code', type=str_from_manual_code,
                            metavar="MANUAL_CODE", help="Manual setup code content (overrides passcode and discriminator)")

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
    args_group.add_argument('--int-arg', nargs='*', type=int_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for an integer as hex or decimal (e.g. -2 or 0xFFFF_1234)")
    args_group.add_argument('--bool-arg', nargs='*', type=bool_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for an boolean value (e.g. true/false or 0/1)")
    args_group.add_argument('--float-arg', nargs='*', type=float_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for a floating point value (e.g. -2.1 or 6.022e23)")
    args_group.add_argument('--string-arg', nargs='*', type=str_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for a string value")
    args_group.add_argument('--json-arg', nargs='*', type=json_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for JSON stored as a list or dict")
    args_group.add_argument('--hex-arg', nargs='*', type=bytes_as_hex_named_arg, metavar="NAME:VALUE",
                            help="Add a named test argument for an octet string in hex (e.g. 0011cafe or 00:11:CA:FE)")

    if not argv:
        argv = sys.argv[1:]

    return convert_args_to_matter_config(parser.parse_known_args(argv)[0])


def async_test_body(body):
    """Decorator required to be applied whenever a `test_*` method is `async def`.

    Since Mobly doesn't support asyncio directly, and the test methods are called
    synchronously, we need a mechanism to allow an `async def` to be converted to
    a asyncio-run synchronous method. This decorator does the wrapping.
    """

    def async_runner(self: MatterBaseTest, *args, **kwargs):
        timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout
        runner_with_timeout = asyncio.wait_for(body(self, *args, **kwargs), timeout=timeout)
        return asyncio.run(runner_with_timeout)

    return async_runner


class CommissionDeviceTest(MatterBaseTest):
    """Test class auto-injected at the start of test list to commission a device when requested"""

    def __init__(self, *args):
        super().__init__(*args)
        self.is_commissioning = True

    def test_run_commissioning(self):
        conf = self.matter_test_config
        for commission_idx, node_id in enumerate(conf.dut_node_ids):
            logging.info("Starting commissioning for root index %d, fabric ID 0x%016X, node ID 0x%016X" %
                         (conf.root_of_trust_index, conf.fabric_id, node_id))
            logging.info("Commissioning method: %s" % conf.commissioning_method)

            if not self._commission_device(commission_idx):
                raise signals.TestAbortAll("Failed to commission node")

    def _commission_device(self, i) -> bool:
        dev_ctrl = self.default_controller
        conf = self.matter_test_config

        # TODO: qr code and manual code aren't lists

        if conf.qr_code_content or conf.manual_code:
            info = self.get_setup_payload_info()
        else:
            info = SetupPayloadInfo()
            info.passcode = conf.setup_passcodes[i]
            info.filter_type = DiscoveryFilterType.LONG_DISCRIMINATOR
            info.filter_value = conf.discriminators[i]

        if conf.commissioning_method == "on-network":
            return dev_ctrl.CommissionOnNetwork(
                nodeId=conf.dut_node_ids[i],
                setupPinCode=info.passcode,
                filterType=info.filter_type,
                filter=info.filter_value
            )
        elif conf.commissioning_method == "ble-wifi":
            return dev_ctrl.CommissionWiFi(
                info.filter_value,
                info.passcode,
                conf.dut_node_ids[i],
                conf.wifi_ssid,
                conf.wifi_passphrase
            )
        elif conf.commissioning_method == "ble-thread":
            return dev_ctrl.CommissionThread(
                info.filter_value,
                info.passcode,
                conf.dut_node_ids[i],
                conf.thread_operational_dataset
            )
        elif conf.commissioning_method == "on-network-ip":
            logging.warning("==== USING A DIRECT IP COMMISSIONING METHOD NOT SUPPORTED IN THE LONG TERM ====")
            return dev_ctrl.CommissionIP(
                ipaddr=conf.commissionee_ip_address_just_for_testing,
                setupPinCode=info.passcode, nodeid=conf.dut_node_ids[i]
            )
        else:
            raise ValueError("Invalid commissioning method %s!" % conf.commissioning_method)


def default_matter_test_main(argv=None, **kwargs):
    """Execute the test class in a test module.
    This is the default entry point for running a test script file directly.
    In this case, only one test class in a test script is allowed.
    To make your test script executable, add the following to your file:
    .. code-block:: python
      from matter_testing_support.py import default_matter_test_main
      ...
      if __name__ == '__main__':
        default_matter_test_main.main()
    Args:
      argv: A list that is then parsed as command line args. If None, defaults to sys.argv
    """

    matter_test_config = parse_matter_test_args(argv)

    # Allow override of command line from optional arguments
    if not matter_test_config.controller_cat_tags and "controller_cat_tags" in kwargs:
        matter_test_config.controller_cat_tags = kwargs["controller_cat_tags"]

    # Find the test class in the test script.
    test_class = _find_test_class()

    # This is required in case we need any testing with maximized certificate chains.
    # We need *all* issuers from the start, even for default controller, to use
    # maximized chains, before MatterStackState init, others some stale certs
    # may not chain properly.
    if "maximize_cert_chains" in kwargs:
        matter_test_config.maximize_cert_chains = kwargs["maximize_cert_chains"]

    hooks = InternalTestRunnerHooks()

    run_tests(test_class, matter_test_config, hooks)


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


def run_tests(test_class: MatterBaseTest, matter_test_config: MatterTestConfig, hooks: TestRunnerHooks) -> None:

    get_test_info(test_class, matter_test_config)

    # Load test config file.
    test_config = generate_mobly_test_config(matter_test_config)

    # Parse test specifiers if exist.
    tests = None
    if len(matter_test_config.tests) > 0:
        tests = matter_test_config.tests

    stack = MatterStackState(matter_test_config)

    with TracingContext() as tracing_ctx:
        for destination in matter_test_config.trace_to:
            tracing_ctx.StartFromString(destination)

        test_config.user_params["matter_stack"] = stash_globally(stack)

        # TODO: Steer to right FabricAdmin!
        # TODO: If CASE Admin Subject is a CAT tag range, then make sure to issue NOC with that CAT tag

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
                # but it's relatively easy to exapand that to make the test process faster
                # TODO: support a list of tests
                hooks.start(count=1)
                # Mobly gives the test run time in seconds, lets be a bit more precise
                runner_start_time = datetime.now(timezone.utc)

            try:
                runner.run()
                ok = runner.results.is_all_pass and ok
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

    # Shutdown the stack when all done
    stack.Shutdown()

    if ok:
        logging.info("Final result: PASS !")
    else:
        logging.error("Final result: FAIL !")
        sys.exit(1)

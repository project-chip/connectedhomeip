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

import argparse
import asyncio
import builtins
import inspect
import json
import logging
import os
import pathlib
import queue
import random
import re
import sys
import textwrap
import threading
import time
import typing
import uuid
from binascii import unhexlify
from dataclasses import asdict as dataclass_asdict
from dataclasses import dataclass, field
from datetime import datetime, timedelta, timezone
from enum import Enum, IntFlag
from functools import partial
from itertools import chain
from typing import Any, Iterable, List, Optional, Tuple

import chip.testing.conversions as conversions
import chip.testing.matchers as matchers
import chip.testing.timeoperations as timeoperations
from chip.tlv import uint

# isort: off

from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
import chip.FabricAdmin  # Needed before chip.CertificateAuthority
import chip.CertificateAuthority

# isort: on
from time import sleep

import chip.clusters as Clusters
import chip.logging
import chip.native
from chip import discovery
from chip.ChipStack import ChipStack
from chip.clusters import Attribute
from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction, TypedAttributePath
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError, Status
from chip.setup_payload import SetupPayload
from chip.storage import PersistentStorage
from chip.testing.commissioning import CommissioningInfo, CustomCommissioningParameters, SetupPayloadInfo, commission_devices
from chip.testing.global_attribute_ids import GlobalAttributeIds
from chip.testing.pics import read_pics_from_file
from chip.tracing import TracingContext
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


def unstash_globally(id: str) -> Any:
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
    def __init__(self, expected_cluster: ClusterObjects.Cluster):
        """This class creates a queue to store received event callbacks, that can be checked by the test script
           expected_cluster: is the cluster from which the events are expected
        """
        self._q: queue.Queue = queue.Queue()
        self._expected_cluster = expected_cluster

    async def start(self, dev_ctrl, node_id: int, endpoint: int, fabric_filtered: bool = False, min_interval_sec: int = 0, max_interval_sec: int = 30) -> Any:
        """This starts a subscription for events on the specified node_id and endpoint. The cluster is specified when the class instance is created."""
        urgent = True
        self._subscription = await dev_ctrl.ReadEvent(node_id,
                                                      events=[(endpoint, self._expected_cluster, urgent)], reportInterval=(
                                                          min_interval_sec, max_interval_sec),
                                                      fabricFiltered=fabric_filtered, keepSubscriptions=True, autoResubscribe=False)
        self._subscription.SetEventUpdateCallback(self.__call__)
        return self._subscription

    def __call__(self, res: EventReadResult, transaction: SubscriptionTransaction):
        """This is the subscription callback when an event is received.
           It checks the event is from the expected_cluster and then posts it into the queue for later processing."""
        if res.Status == Status.Success and res.Header.ClusterId == self._expected_cluster.id:
            logging.info(
                f'Got subscription report for event on cluster {self._expected_cluster}: {res.Data}')
            self._q.put(res)

    def wait_for_event_report(self, expected_event: ClusterObjects.ClusterEvent, timeout_sec: float = 10.0) -> Any:
        """This function allows a test script to block waiting for the specific event to be the next event
           to arrive within a timeout (specified in seconds). It returns the event data so that the values can be checked."""
        logging.info(f"Waiting for {expected_event} for {timeout_sec:.1f} seconds")
        try:
            res = self._q.get(block=True, timeout=timeout_sec)
        except queue.Empty:
            asserts.fail("Failed to receive a report for the event {}".format(expected_event))

        asserts.assert_equal(res.Header.ClusterId, expected_event.cluster_id, "Expected cluster ID not found in event report")
        asserts.assert_equal(res.Header.EventId, expected_event.event_id, "Expected event ID not found in event report")
        logging.info(f"Successfully waited for {expected_event}")
        return res.Data

    def wait_for_event_expect_no_report(self, timeout_sec: float = 10.0):
        """This function returns if an event does not arrive within the timeout specified in seconds.
           If any event does arrive, an assert failure occurs."""
        try:
            res = self._q.get(block=True, timeout=timeout_sec)
        except queue.Empty:
            return

        asserts.fail(f"Event reported when not expected {res}")

    def get_last_event(self) -> Optional[Any]:
        """Flush entire queue, returning last (newest) event only."""
        last_event: Optional[Any] = None
        while True:
            try:
                last_event = self._q.get(block=False)
            except queue.Empty:
                return last_event

    def flush_events(self) -> None:
        """Flush entire queue, returning nothing."""
        _ = self.get_last_event()
        return

    def reset(self) -> None:
        """Resets state as if no events had ever been received."""
        self.flush_events()

    @property
    def event_queue(self) -> queue.Queue:
        return self._q


class AttributeChangeCallback:
    def __init__(self, expected_attribute: ClusterObjects.ClusterAttributeDescriptor):
        self._output = queue.Queue()
        self._expected_attribute = expected_attribute

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction):
        """This is the subscription callback when an attribute is updated.
           It checks the passed in attribute is the same as the subscribed to attribute and
           then posts it into the queue for later processing."""

        asserts.assert_equal(path.AttributeType, self._expected_attribute,
                             f"[AttributeChangeCallback] Attribute mismatch. Expected: {self._expected_attribute}, received: {path.AttributeType}")
        logging.debug(f"[AttributeChangeCallback] Attribute update callback for {path.AttributeType}")
        q = (path, transaction)
        self._output.put(q)

    def wait_for_report(self):
        try:
            path, transaction = self._output.get(block=True, timeout=10)
        except queue.Empty:
            asserts.fail(
                f"[AttributeChangeCallback] Failed to receive a report for the {self._expected_attribute} attribute change")

        asserts.assert_equal(path.AttributeType, self._expected_attribute,
                             f"[AttributeChangeCallback] Received incorrect report. Expected: {self._expected_attribute}, received: {path.AttributeType}")
        try:
            attribute_value = transaction.GetAttribute(path)
            logging.info(
                f"[AttributeChangeCallback] Got attribute subscription report. Attribute {path.AttributeType}. Updated value: {attribute_value}. SubscriptionId: {transaction.subscriptionId}")
        except KeyError:
            asserts.fail(f"[AttributeChangeCallback] Attribute {self._expected_attribute} not found in returned report")


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


def await_sequence_of_reports(report_queue: queue.Queue, endpoint_id: int, attribute: TypedAttributePath, sequence: list[Any], timeout_sec: float) -> None:
    """Given a queue.Queue hooked-up to an attribute change accumulator, await a given expected sequence of attribute reports.

    Args:
      - report_queue: the queue that receives all the reports.
      - endpoint_id: endpoint ID to match for reports to check.
      - attribute: attribute to match for reports to check.
      - sequence: list of attribute values in order that are expected.
      - timeout_sec: number of seconds to wait for.

    *** WARNING: The queue contains every report since the sub was established. Use
        clear_queue to make it empty. ***

    This will fail current Mobly test with assertion failure if the data is not as expected in order.

    Returns nothing on success so the test can go on.
    """
    start_time = time.time()
    elapsed = 0.0
    time_remaining = timeout_sec

    sequence_idx = 0
    actual_values = []

    while time_remaining > 0:
        expected_value = sequence[sequence_idx]
        logging.info(f"Expecting value {expected_value} for attribute {attribute} on endpoint {endpoint_id}")
        logging.info(f"Waiting for {timeout_sec:.1f} seconds for all reports.")
        try:
            item: AttributeValue = report_queue.get(block=True, timeout=time_remaining)

            # Track arrival of all values for the given attribute.
            if item.endpoint_id == endpoint_id and item.attribute == attribute:
                actual_values.append(item.value)

                if item.value == expected_value:
                    logging.info(f"Got expected attribute change {sequence_idx+1}/{len(sequence)} for attribute {attribute}")
                    sequence_idx += 1
                else:
                    asserts.assert_equal(item.value, expected_value,
                                         msg=f"Did not get expected attribute value in correct sequence. Sequence so far: {actual_values}")

                # We are done waiting when we have accumulated all results.
                if sequence_idx == len(sequence):
                    logging.info("Got all attribute changes, done waiting.")
                    return
        except queue.Empty:
            # No error, we update timeouts and keep going
            pass

        elapsed = time.time() - start_time
        time_remaining = timeout_sec - elapsed

    asserts.fail(f"Did not get full sequence {sequence} in {timeout_sec:.1f} seconds. Got {actual_values} before time-out.")


class ClusterAttributeChangeAccumulator:
    def __init__(self, expected_cluster: ClusterObjects.Cluster):
        self._expected_cluster = expected_cluster
        self._subscription = None
        self._lock = threading.Lock()
        self._q = queue.Queue()
        self._endpoint_id = 0
        self.reset()

    def reset(self):
        with self._lock:
            self._attribute_report_counts = {}
            attrs = [cls for name, cls in inspect.getmembers(self._expected_cluster.Attributes) if inspect.isclass(
                cls) and issubclass(cls, ClusterObjects.ClusterAttributeDescriptor)]
            self._attribute_reports = {}
            for a in attrs:
                self._attribute_report_counts[a] = 0
                self._attribute_reports[a] = []

        self.flush_reports()

    async def start(self, dev_ctrl, node_id: int, endpoint: int, fabric_filtered: bool = False, min_interval_sec: int = 0, max_interval_sec: int = 5, keepSubscriptions: bool = True) -> Any:
        """This starts a subscription for attributes on the specified node_id and endpoint. The cluster is specified when the class instance is created."""
        self._subscription = await dev_ctrl.ReadAttribute(
            nodeid=node_id,
            attributes=[(endpoint, self._expected_cluster)],
            reportInterval=(int(min_interval_sec), int(max_interval_sec)),
            fabricFiltered=fabric_filtered,
            keepSubscriptions=keepSubscriptions
        )
        self._endpoint_id = endpoint
        self._subscription.SetAttributeUpdateCallback(self.__call__)
        return self._subscription

    async def cancel(self):
        """This cancels a subscription."""
        # Wait for the asyncio.CancelledError to be called before returning
        try:
            self._subscription.Shutdown()
            await asyncio.sleep(5)
        except asyncio.CancelledError:
            pass

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction):
        """This is the subscription callback when an attribute report is received.
           It checks the report is from the expected_cluster and then posts it into the queue for later processing."""
        if path.ClusterType == self._expected_cluster:
            data = transaction.GetAttribute(path)
            value = AttributeValue(endpoint_id=path.Path.EndpointId, attribute=path.AttributeType,
                                   value=data, timestamp_utc=datetime.now(timezone.utc))
            logging.info(f"Got subscription report for {path.AttributeType}: {data}")
            self._q.put(value)
            with self._lock:
                self._attribute_report_counts[path.AttributeType] += 1
                self._attribute_reports[path.AttributeType].append(value)

    def await_all_final_values_reported(self, expected_final_values: Iterable[AttributeValue], timeout_sec: float = 1.0):
        """Expect that every `expected_final_value` report is the last value reported for the given attribute, ignoring timestamps.

        Waits for at least `timeout_sec` seconds.

        This is a form of barrier for a set of attribute changes that should all happen together for an action.
        """
        start_time = time.time()
        elapsed = 0.0
        time_remaining = timeout_sec

        last_report_matches: dict[int, bool] = {idx: False for idx, _ in enumerate(expected_final_values)}

        for element in expected_final_values:
            logging.info(
                f"--> Expecting report for value {element.value} for attribute {element.attribute} on endpoint {element.endpoint_id}")
        logging.info(f"Waiting for {timeout_sec:.1f} seconds for all reports.")

        while time_remaining > 0:
            # Snapshot copy at the beginning of the loop. This is thread-safe based on the design.
            all_reports = self._attribute_reports

            # Recompute all last-value matches
            for expected_idx, expected_element in enumerate(expected_final_values):
                last_value = None
                for report in all_reports.get(expected_element.attribute, []):
                    if report.endpoint_id == expected_element.endpoint_id:
                        last_value = report.value

                last_report_matches[expected_idx] = (last_value is not None and last_value == expected_element.value)

            # Determine if all were met
            if all(last_report_matches.values()):
                logging.info("Found all expected reports were true.")
                return

            elapsed = time.time() - start_time
            time_remaining = timeout_sec - elapsed
            time.sleep(0.1)

        # If we reach here, there was no early return and we failed to find all the values.
        logging.error("Reached time-out without finding all expected report values.")
        logging.info("Values found:")
        for expected_idx, expected_element in enumerate(expected_final_values):
            logging.info(f"  -> {expected_element} found: {last_report_matches.get(expected_idx)}")
        asserts.fail("Did not find all expected last report values before time-out")

    def await_sequence_of_reports(self, attribute: TypedAttributePath, sequence: list[Any], timeout_sec: float) -> None:
        """Await a given expected sequence of attribute reports in the accumulator for the endpoint associated.

        Args:
          - attribute: attribute to match for reports to check.
          - sequence: list of attribute values in order that are expected.
          - timeout_sec: number of seconds to wait for.

        *** WARNING: The queue contains every report since the sub was established. Use
            self.reset() to make it empty. ***

        This will fail current Mobly test with assertion failure if the data is not as expected in order.

        Returns nothing on success so the test can go on.
        """
        await_sequence_of_reports(report_queue=self.attribute_queue, endpoint_id=self._endpoint_id,
                                  attribute=attribute, sequence=sequence, timeout_sec=timeout_sec)

    @property
    def attribute_queue(self) -> queue.Queue:
        return self._q

    @property
    def attribute_report_counts(self) -> dict[ClusterObjects.ClusterAttributeDescriptor, int]:
        with self._lock:
            return self._attribute_report_counts

    @property
    def attribute_reports(self) -> dict[ClusterObjects.ClusterAttributeDescriptor, AttributeValue]:
        with self._lock:
            return self._attribute_reports.copy()

    def get_last_report(self) -> Optional[Any]:
        """Flush entire queue, returning last (newest) report only."""
        last_report: Optional[Any] = None
        while True:
            try:
                last_report = self._q.get(block=False)
            except queue.Empty:
                return last_report

    def flush_reports(self) -> None:
        """Flush entire queue, returning nothing."""
        _ = self.get_last_report()
        return


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
    endpoint: typing.Union[int, None] = 0
    app_pid: int = 0
    fail_on_skipped_tests: bool = False

    commissioning_method: Optional[str] = None
    in_test_commissioning_method: Optional[str] = None
    discriminators: List[int] = field(default_factory=list)
    setup_passcodes: List[int] = field(default_factory=list)
    commissionee_ip_address_just_for_testing: Optional[str] = None
    # By default, we start with maximized cert chains, as required for RR-1.1.
    # This allows cert tests to be run without re-commissioning for RR-1.1.
    maximize_cert_chains: bool = True

    # By default, let's set validity to 10 years
    certificate_validity_period = int(timedelta(days=10*365).total_seconds())

    qr_code_content: List[str] = field(default_factory=list)
    manual_code: List[str] = field(default_factory=list)

    wifi_ssid: Optional[str] = None
    wifi_passphrase: Optional[str] = None
    thread_operational_dataset: Optional[str] = None

    pics: dict[bool, str] = field(default_factory=dict)

    # Node ID for basic DUT
    dut_node_ids: List[int] = field(default_factory=list)
    # Node ID to use for controller/commissioner
    controller_node_id: int = _DEFAULT_CONTROLLER_NODE_ID
    # CAT Tags for default controller/commissioner
    # By default, we commission with CAT tags specified for RR-1.1
    # so the cert tests can be run without re-commissioning the device
    # for this one test. This can be overwritten from the command line
    controller_cat_tags: List[int] = field(default_factory=lambda: [0x0001_0001])

    # Fabric ID which to use
    fabric_id: int = 1

    # "Alpha" by default
    root_of_trust_index: int = _DEFAULT_TRUST_ROOT_INDEX

    # If this is set, we will reuse root of trust keys at that location
    chip_tool_credentials_path: Optional[pathlib.Path] = None

    trace_to: List[str] = field(default_factory=list)

    # Accepted Terms and Conditions if used
    tc_version_to_simulate: int = None
    tc_user_response_to_simulate: int = None
    # path to device attestation revocation set json file
    dac_revocation_set_path: Optional[pathlib.Path] = None


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


@dataclass
class ClusterPathLocation:
    endpoint_id: int
    cluster_id: int

    def __str__(self):
        return (f'\n       Endpoint: {self.endpoint_id},'
                f'\n       Cluster:  {cluster_id_str(self.cluster_id)}')


@dataclass
class AttributePathLocation(ClusterPathLocation):
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
        return (f'{super().__str__()}'
                f'\n      Attribute:{id_str(self.attribute_id)}')


@dataclass
class EventPathLocation(ClusterPathLocation):
    event_id: int

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n       Event:    {id_str(self.event_id)}')


@dataclass
class CommandPathLocation(ClusterPathLocation):
    command_id: int

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n       Command:  {id_str(self.command_id)}')


@dataclass
class FeaturePathLocation(ClusterPathLocation):
    feature_code: str

    def __str__(self):
        return (f'{super().__str__()}'
                f'\n       Feature:  {self.feature_code}')


@dataclass
class DeviceTypePathLocation:
    device_type_id: int
    cluster_id: Optional[int] = None

    def __str__(self):
        msg = f'\n       DeviceType: {self.device_type_id}'
        if self.cluster_id:
            msg += f'\n       ClusterID: {self.cluster_id}'
        return msg


class UnknownProblemLocation:
    def __str__(self):
        return '\n      Unknown Locations - see message for more details'


ProblemLocation = typing.Union[ClusterPathLocation, DeviceTypePathLocation, UnknownProblemLocation]

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
    location: ProblemLocation
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
            if not os.path.exists(app_pipe_name):
                # Named pipes are unique, so we MUST have consistent PID/paths
                # set up for them to work.
                logging.error("Named pipe %r does NOT exist" % app_pipe_name)
                raise FileNotFoundError("CANNOT FIND %r" % app_pipe_name)
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

    async def commission_devices(self) -> bool:
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
    config.dac_revocation_set_path = args.dac_revocation_set_path

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
    basic_group.add_argument('--dac-revocation-set-path', action="store", type=pathlib.Path, metavar="PATH",
                             help="Path to JSON file containing the device attestation revocation set.")
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


def _async_runner(body, self: MatterBaseTest, *args, **kwargs):
    timeout = self.matter_test_config.timeout if self.matter_test_config.timeout is not None else self.default_timeout
    return self.event_loop.run_until_complete(asyncio.wait_for(body(self, *args, **kwargs), timeout=timeout))


def async_test_body(body):
    """Decorator required to be applied whenever a `test_*` method is `async def`.

    Since Mobly doesn't support asyncio directly, and the test methods are called
    synchronously, we need a mechanism to allow an `async def` to be converted to
    a asyncio-run synchronous method. This decorator does the wrapping.
    """

    def async_runner(self: MatterBaseTest, *args, **kwargs):
        return _async_runner(body, self, *args, **kwargs)

    return async_runner


EndpointCheckFunction = typing.Callable[[Clusters.Attribute.AsyncReadTransaction.ReadResponse, int], bool]


def get_cluster_from_attribute(attribute: ClusterObjects.ClusterAttributeDescriptor) -> ClusterObjects.Cluster:
    return ClusterObjects.ALL_CLUSTERS[attribute.cluster_id]


def get_cluster_from_command(command: ClusterObjects.ClusterCommand) -> ClusterObjects.Cluster:
    return ClusterObjects.ALL_CLUSTERS[command.cluster_id]


def _has_cluster(wildcard, endpoint, cluster: ClusterObjects.Cluster) -> bool:
    try:
        return cluster in wildcard.attributes[endpoint]
    except KeyError:
        return False


def has_cluster(cluster: ClusterObjects.ClusterObjectDescriptor) -> EndpointCheckFunction:
    """ EndpointCheckFunction that can be passed as a parameter to the run_if_endpoint_matches decorator.

        Use this function with the run_if_endpoint_matches decorator to run this test on all endpoints with
        the specified cluster. For example, given a device with the following conformance

        EP0: cluster A, B, C
        EP1: cluster D, E
        EP2, cluster D
        EP3, cluster E

        And the following test specification:
        @run_if_endpoint_matches(has_cluster(Clusters.D))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the run_if_endpoint_matches decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.
    """
    return partial(_has_cluster, cluster=cluster)


def _has_attribute(wildcard, endpoint, attribute: ClusterObjects.ClusterAttributeDescriptor) -> bool:
    cluster = get_cluster_from_attribute(attribute)
    try:
        attr_list = wildcard.attributes[endpoint][cluster][cluster.Attributes.AttributeList]
        if not isinstance(attr_list, list):
            asserts.fail(
                f"Failed to read mandatory AttributeList attribute value for cluster {cluster} on endpoint {endpoint}: {attr_list}.")
        return attribute.attribute_id in attr_list
    except KeyError:
        return False


def has_attribute(attribute: ClusterObjects.ClusterAttributeDescriptor) -> EndpointCheckFunction:
    """ EndpointCheckFunction that can be passed as a parameter to the run_if_endpoint_matches decorator.

        Use this function with the run_if_endpoint_matches decorator to run this test on all endpoints with
        the specified attribute. For example, given a device with the following conformance

        EP0: cluster A, B, C
        EP1: cluster D with attribute d, E
        EP2, cluster D with attribute d
        EP3, cluster D without attribute d

        And the following test specification:
        @run_if_endpoint_matches(has_attribute(Clusters.D.Attributes.d))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the run_if_endpoint_matches decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.
    """
    return partial(_has_attribute, attribute=attribute)


def _has_command(wildcard, endpoint, command: ClusterObjects.ClusterCommand) -> bool:
    cluster = get_cluster_from_command(command)
    try:
        cmd_list = wildcard.attributes[endpoint][cluster][cluster.Attributes.AcceptedCommandList]
        if not isinstance(cmd_list, list):
            asserts.fail(
                f"Failed to read mandatory AcceptedCommandList command value for cluster {cluster} on endpoint {endpoint}: {cmd_list}.")
        return command.command_id in cmd_list
    except KeyError:
        return False


def has_command(command: ClusterObjects.ClusterCommand) -> EndpointCheckFunction:
    """ EndpointCheckFunction that can be passed as a parameter to the run_if_endpoint_matches decorator.

        Use this function with the run_if_endpoint_matches decorator to run this test on all endpoints with
        the specified attribute. For example, given a device with the following conformance

        EP0: cluster A, B, C
        EP1: cluster D with command d, E
        EP2, cluster D with command d
        EP3, cluster D without command d

        And the following test specification:
        @run_if_endpoint_matches(has_command(Clusters.D.Commands.d))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the run_if_endpoint_matches decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.
    """
    return partial(_has_command, command=command)


def _has_feature(wildcard, endpoint: int, cluster: ClusterObjects.ClusterObjectDescriptor, feature: IntFlag) -> bool:
    try:
        feature_map = wildcard.attributes[endpoint][cluster][cluster.Attributes.FeatureMap]
        if not isinstance(feature_map, int):
            asserts.fail(
                f"Failed to read mandatory FeatureMap attribute value for cluster {cluster} on endpoint {endpoint}: {feature_map}.")

        return (feature & feature_map) != 0
    except KeyError:
        return False


def has_feature(cluster: ClusterObjects.ClusterObjectDescriptor, feature: IntFlag) -> EndpointCheckFunction:
    """ EndpointCheckFunction that can be passed as a parameter to the run_if_endpoint_matches decorator.

        Use this function with the run_if_endpoint_matches decorator to run this test on all endpoints with
        the specified feature. For example, given a device with the following conformance

        EP0: cluster A, B, C
        EP1: cluster D with feature F0
        EP2: cluster D with feature F0
        EP3: cluster D without feature F0

        And the following test specification:
        @run_if_endpoint_matches(has_feature(Clusters.D.Bitmaps.Feature.F0))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the run_if_endpoint_matches decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.
    """
    return partial(_has_feature, cluster=cluster, feature=feature)


async def _get_all_matching_endpoints(self: MatterBaseTest, accept_function: EndpointCheckFunction) -> list[uint]:
    """ Returns a list of endpoints matching the accept condition. """
    wildcard = await self.default_controller.Read(self.dut_node_id, [(Clusters.Descriptor), Attribute.AttributePath(None, None, GlobalAttributeIds.ATTRIBUTE_LIST_ID), Attribute.AttributePath(None, None, GlobalAttributeIds.FEATURE_MAP_ID), Attribute.AttributePath(None, None, GlobalAttributeIds.ACCEPTED_COMMAND_LIST_ID)])
    matching = [e for e in wildcard.attributes.keys() if accept_function(wildcard, e)]
    return matching


async def should_run_test_on_endpoint(self: MatterBaseTest, accept_function: EndpointCheckFunction) -> bool:
    """ Helper function for the run_if_endpoint_matches decorator.

        Returns True if self.matter_test_config.endpoint matches the accept function.
    """
    if self.matter_test_config.endpoint is None:
        msg = """
              The --endpoint flag is required for this test.
              """
        asserts.fail(msg)
    matching = await (_get_all_matching_endpoints(self, accept_function))
    return self.matter_test_config.endpoint in matching


def run_on_singleton_matching_endpoint(accept_function: EndpointCheckFunction):
    """ Test decorator for a test that needs to be run on the endpoint that matches the given accept function.

        This decorator should be used for tests where the endpoint is not known a-priori (dynamic endpoints).
        Note that currently this test is limited to devices with a SINGLE matching endpoint.
    """
    def run_on_singleton_matching_endpoint_internal(body):
        def matching_runner(self: MatterBaseTest, *args, **kwargs):
            runner_with_timeout = asyncio.wait_for(_get_all_matching_endpoints(self, accept_function), timeout=30)
            matching = self.event_loop.run_until_complete(runner_with_timeout)
            asserts.assert_less_equal(len(matching), 1, "More than one matching endpoint found for singleton test.")
            if not matching:
                logging.info("Test is not applicable to any endpoint - skipping test")
                asserts.skip('No endpoint matches test requirements')
                return
            # Exceptions should flow through, hence no except block
            try:
                old_endpoint = self.matter_test_config.endpoint
                self.matter_test_config.endpoint = matching[0]
                logging.info(f'Running test on endpoint {self.matter_test_config.endpoint}')
                _async_runner(body, self, *args, **kwargs)
            finally:
                self.matter_test_config.endpoint = old_endpoint
        return matching_runner
    return run_on_singleton_matching_endpoint_internal


def run_if_endpoint_matches(accept_function: EndpointCheckFunction):
    """ Test decorator for a test that needs to be run only if the endpoint meets the accept_function criteria.

        Place this decorator above the test_ method to have the test framework run this test only if the endpoint matches.
        This decorator takes an EndpointCheckFunction to assess whether a test needs to be run on a particular
        endpoint.

        For example, given the following device conformance:

        EP0: cluster A, B, C
        EP1: cluster D, E
        EP2, cluster D
        EP3, cluster E

        And the following test specification:
        @run_if_endpoint_matches(has_cluster(Clusters.D))
        test_mytest(self):
            ...

        If you run this test with --endpoint 1 or --endpoint 2, the test will be run. If you run this test
        with any other --endpoint the decorator will call the on_skip function to
        notify the test harness that the test is not applicable to this node and the test will not be run.

        Tests that use this decorator cannot use a pics_ method for test selection and should not reference any
        PICS values internally.
    """
    def run_if_endpoint_matches_internal(body):
        def per_endpoint_runner(self: MatterBaseTest, *args, **kwargs):
            runner_with_timeout = asyncio.wait_for(should_run_test_on_endpoint(self, accept_function), timeout=60)
            should_run_test = self.event_loop.run_until_complete(runner_with_timeout)
            if not should_run_test:
                logging.info("Test is not applicable to this endpoint - skipping test")
                asserts.skip('Endpoint does not match test requirements')
                return
            logging.info(f'Running test on endpoint {self.matter_test_config.endpoint}')
            _async_runner(body, self, *args, **kwargs)
        return per_endpoint_runner
    return run_if_endpoint_matches_internal


class CommissionDeviceTest(MatterBaseTest):
    """Test class auto-injected at the start of test list to commission a device when requested"""

    def __init__(self, *args):
        super().__init__(*args)
        self.is_commissioning = True

    def test_run_commissioning(self):
        if not self.event_loop.run_until_complete(self.commission_devices()):
            raise signals.TestAbortAll("Failed to commission node(s)")


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

    matter_test_config = parse_matter_test_args()

    # Find the test class in the test script.
    test_class = _find_test_class()

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
                catTags=matter_test_config.controller_cat_tags,
                dacRevocationSetPath=str(matter_test_config.dac_revocation_set_path),
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

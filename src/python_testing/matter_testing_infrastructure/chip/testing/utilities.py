"""Utility functions for Matter testing infrastructure."""

import asyncio
import inspect
import json
import logging
import pathlib
import queue
import re
import threading
import time
import typing
from binascii import hexlify, unhexlify
from datetime import datetime, timedelta, timezone
from typing import Any, Iterable, Optional, Tuple

from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction, TypedAttributePath
from chip.interaction_model import Status
from chip.tlv import float32, uint
from mobly import asserts

from python_testing.matter_testing_infrastructure.chip.testing.models import AttributeValue


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


def bytes_from_hex(hex: str) -> bytes:
    """Converts any `hex` string representation including `01:ab:cd` to bytes

    Handles any whitespace including newlines, which are all stripped.
    """
    return unhexlify("".join(hex.replace(":", "").replace(" ", "").split()))


def hex_from_bytes(b: bytes) -> str:
    """Converts a bytes object `b` into a hex string (reverse of bytes_from_hex)"""
    return hexlify(b).decode("utf-8")


def int_decimal_or_hex(s: str) -> int:
    val = int(s, 0)
    if val < 0:
        raise ValueError("Negative values not supported")
    return val


def byte_string_from_hex(s: str) -> bytes:
    return unhexlify(s.replace(":", "").replace(" ", "").replace("0x", ""))


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


def compare_time(received: int, offset: timedelta = timedelta(), utc: Optional[int] = None, tolerance: timedelta = timedelta(seconds=5)) -> None:
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

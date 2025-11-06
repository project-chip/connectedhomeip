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

"""
This module provides classes to manage and validate subscription-based event and attribute reporting.

Classes:
    EventSubscriptionHandler: Handles subscription to events.
    AttributeSubscriptionHandler: Manages subscriptions to specific attributes.

Both classes allow tests to start and manage subscriptions, queue received updates asynchronously and
block until epected reports are received or fail on timeouts
"""

import asyncio
import inspect
import logging
import queue
import threading
import time
from datetime import datetime, timezone
from typing import Any, Iterable, Optional

from mobly import asserts

from matter.clusters import ClusterObjects as ClusterObjects
from matter.clusters.Attribute import EventReadResult, SubscriptionTransaction, TypedAttributePath
from matter.interaction_model import Status
from matter.testing.matter_testing import AttributeMatcher, AttributeValue

LOGGER = logging.getLogger(__name__)


class EventSubscriptionHandler:
    """
    Handles subscription-based event reporting. It sets up and manages event subscriptions for a specific cluster or event ID,
    captures incoming event reports through a callback and stores them for validation and processing.

    It supports two usage modes:
        1. Cluster mode: Pass a 'ClusterObjects.Cluster' to subscribe to all events in a cluster.
        2. Event ID mode: Pass 'expected_cluster_id' and 'expected_event_id' to subscribe to a specific event.

    Attributes:
        _expected_cluster: The cluster object to match.
        _expected_cluster_id: The cluster ID to match against incoming event headers.
        _expected_event_id: The specific event ID to match.
        _q: Internal queue that stores matching EventReadResult objects.
    """

    def __init__(self, *, expected_cluster: Optional[ClusterObjects.Cluster] = None, expected_cluster_id: Optional[int] = None, expected_event_id: Optional[int] = None):
        is_cluster_mode = expected_cluster is not None
        is_id_mode = all(x is not None for x in (expected_cluster_id, expected_event_id))

        if not (is_cluster_mode ^ is_id_mode):
            raise ValueError("Failed argument inputs in EventSubscriptionHandler. You should use Cluster or ClusterId and EventId")

        self._expected_cluster = expected_cluster
        self._expected_cluster_id = expected_cluster_id if expected_cluster_id is not None else expected_cluster.id
        self._expected_event_id = expected_event_id
        self._subscription = None
        self._q: queue.Queue = queue.Queue()

    def __call__(self, event_result: EventReadResult, transaction: SubscriptionTransaction):
        """
        Callback invoked when an event report is received via subscription. This enqueued the event for later processing.

        Parameters:
            event_result (EventReadResult): The result of the received event (includinng header and payload).
            transaction (SubscriptionTransaction): Associated subscription transaction.

        Notes:
            If an expected_event_id is set, only events with that ID will be accepted.
        """

        if event_result.Status != Status.Success:
            return

        header = event_result.Header
        if header.ClusterId != self._expected_cluster_id:
            return

        if self._expected_event_id is not None and header.EventId != self._expected_event_id:
            return

        LOGGER.info(f"[EventSubscriptionHandler] Received event: {header}")
        self._q.put(event_result)

    async def start(self, dev_ctrl, node_id: int, endpoint: int, fabric_filtered: bool = False, min_interval_sec: int = 0, max_interval_sec: int = 30) -> Any:
        """This starts a subscription for events on the specified node_id and endpoint. The cluster is specified when the class instance is created."""
        urgent = True
        self._subscription = await dev_ctrl.ReadEvent(node_id,
                                                      events=[(endpoint, self._expected_cluster, urgent)], reportInterval=(
                                                          min_interval_sec, max_interval_sec),
                                                      fabricFiltered=fabric_filtered, keepSubscriptions=True, autoResubscribe=False)
        self._subscription.SetEventUpdateCallback(self.__call__)
        return self._subscription

    def wait_for_event_report(self, expected_event: ClusterObjects.ClusterEvent, timeout_sec: float = 10.0) -> Any:
        """This function allows a test script to block waiting for the specific event to be the next event
           to arrive within a timeout (specified in seconds). It returns the event data so that the values can be checked."""
        LOGGER.info(f"Waiting for {expected_event} for {timeout_sec:.1f} seconds")
        try:
            res = self._q.get(block=True, timeout=timeout_sec)
        except queue.Empty:
            asserts.fail("Failed to receive a report for the event {}".format(expected_event))

        asserts.assert_equal(res.Header.ClusterId, expected_event.cluster_id, "Expected cluster ID not found in event report")
        asserts.assert_equal(res.Header.EventId, expected_event.event_id, "Expected event ID not found in event report")
        LOGGER.info(f"Successfully waited for {expected_event}")
        return res.Data

    def wait_for_event_expect_no_report(self, timeout_sec: float = 10.0):
        """This function returns if an event does not arrive within the timeout specified in seconds.
           If any event does arrive, an assert failure occurs."""
        try:
            res = self._q.get(block=True, timeout=timeout_sec)
        except queue.Empty:
            return

        asserts.fail(f"Event reported when not expected {res}")

    def wait_for_event_type_report(self, event_type: ClusterObjects.ClusterEvent, timeout_sec: float) -> Optional[Any]:
        """
        Waits for a specific event type from the event subscription handler within the timeout period.

        Parameters:
            event_type (ClusterObjects.ClusterEvent): The expected event type to wait for.
            timeout_sec (float): The maximum time to wait for the event, in seconds.

        Returns:
            The event data (from EventReadResult.Data) when the expected event is received, or fails on timeout.
        """
        event_queue = self.event_queue
        start_time = time.time()
        while True:
            remaining = timeout_sec - (time.time() - start_time)
            if remaining <= 0:
                asserts.fail(f"Timeout waiting for event {event_type}.")
            try:
                event = event_queue.get(block=True, timeout=remaining)
            except queue.Empty:
                asserts.fail(f"Timeout waiting for event {event_type}.")
            if event.Header.EventId == event_type.event_id:
                LOGGER.info(f"Event {event_type.__name__} received: {event}")
                return event.Data
            else:
                LOGGER.info(f"Received other event: {event.Header.EventId}, ignoring and waiting for {event_type.__name__}.")

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

    def get_size(self) -> int:
        return self._q.qsize()

    def get_event_from_queue(self, block: bool, timeout: int):
        return self._q.get(block, timeout)

    async def cancel(self):
        """This cancels a subscription."""
        # Wait for the asyncio.CancelledError to be called before returning
        try:
            self._subscription.Shutdown()
            await asyncio.sleep(5)
        except asyncio.CancelledError:
            pass


class AttributeSubscriptionHandler:
    """
    Callback class to handle attribute subscription reports. This class manages the reception. filtering and queuing of attribute update reports.

    It provides methods to wait for specific updates, validate expected final values and track changes for verification.

    Attributes;
        _expected_cluster: The cluster type to subscribe to.
        _expected_attribute: The attribute within the cluster expected to receive updates.
        _q: Queue storing AttributeValue instances for received updates.
        _attribute_reports: Dictionary holding history of all received reports by attribute.
        _attribute_report_counts: Dictionary counting the number of reports received per attribute.
    """

    def __init__(self, expected_cluster: ClusterObjects.Cluster = None, expected_attribute: ClusterObjects.ClusterAttributeDescriptor = None):

        if expected_cluster is None:
            raise ValueError("Missing argument. Expected Cluster attribute is missing in AttributeSubscriptionHandler constructor")

        self._expected_cluster = expected_cluster
        self._expected_attribute = expected_attribute
        self._subscription = None
        self._q = queue.Queue()
        self._endpoint_id = 0
        self._attribute_report_counts = None
        self._attribute_reports = None
        self._lock = threading.Lock()
        self.reset()

    def reset(self):
        with self._lock:
            self._attribute_report_counts = {}
            attrs = [cls for name, cls in inspect.getmembers(self._expected_cluster.Attributes) if inspect.isclass(
                cls) and issubclass(cls, ClusterObjects.ClusterAttributeDescriptor)]
            self._attribute_reports: dict[Any, AttributeValue] = {}
            if self._expected_attribute is not None:
                attrs = [self._expected_attribute]
            for a in attrs:
                self._attribute_report_counts[a] = 0
                self._attribute_reports[a] = []

        self.flush_reports()

    async def start(self, dev_ctrl, node_id: int, endpoint: int, fabric_filtered: bool = False, min_interval_sec: int = 0, max_interval_sec: int = 5, keepSubscriptions: bool = True) -> Any:
        """This starts a subscription for attributes on the specified node_id and endpoint. The cluster is specified when the class instance is created."""
        attributes = [(endpoint, self._expected_cluster)]
        if self._expected_attribute is not None:
            attributes = [(endpoint, self._expected_attribute)]
        self._subscription = await dev_ctrl.ReadAttribute(
            nodeId=node_id,
            attributes=attributes,
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
        """
        Callback invoked when an attribute  repoort is received via subscription.

        It extracts tha value using the transaction object, wraps into an AttributeValue, enqueues it for later processing,
        and stores it in internal history for verification.

        Parameters:
            path (TypedAttributePath): Contains cluster and attribute metadata for the report.
            transaction (SubscriptionTransaction): Provides access to the actual reported value.
        """

        valid_report = False
        if self._expected_attribute:
            if path.AttributeType == self._expected_attribute:
                valid_report = True
        elif self._expected_cluster:
            if path.ClusterType == self._expected_cluster:
                valid_report = True

        if valid_report:
            data = transaction.GetAttribute(path)
            value = AttributeValue(endpoint_id=path.Path.EndpointId, attribute=path.AttributeType,
                                   value=data, timestamp_utc=datetime.now(timezone.utc))
            LOGGER.info(f"[AttributeSubscriptionHandler] Received attribute report: {path.AttributeType} = {data}")
            self._q.put(value)
            if self._lock:
                with self._lock:
                    self._attribute_report_counts[path.AttributeType] += 1
                    self._attribute_reports[path.AttributeType].append(value)

    def wait_for_attribute_report(self):
        """
        Blocks and waits for a single attribute report to arrive in the queue.

        This method dequeues one report, validates it and return its value.
        """

        try:
            item = self._q.get(block=True, timeout=10)
            attribute_value = item.value
            LOGGER.info(
                f"[AttributeSubscriptionHandler] Got attribute subscription report. Attribute {item.attribute}. Updated value: {attribute_value}. SubscriptionId: {item.value}")
        except queue.Empty:
            asserts.fail(
                f"[AttributeSubscriptionHandler] Failed to receive a report for the {self._expected_attribute} attribute change")

        asserts.assert_equal(item.attribute, self._expected_attribute,
                             f"[AttributeSubscriptionHandler] Received incorrect report. Expected: {self._expected_attribute}, received: {item.attribute}")

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
            LOGGER.info(
                f"--> Expecting report for value {element.value} for attribute {element.attribute} on endpoint {element.endpoint_id}")
        LOGGER.info(f"Waiting for {timeout_sec:.1f} seconds for all reports.")

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
                LOGGER.info("Found all expected reports were true.")
                return

            elapsed = time.time() - start_time
            time_remaining = timeout_sec - elapsed
            time.sleep(0.1)

        # If we reach here, there was no early return and we failed to find all the values.
        LOGGER.error("Reached time-out without finding all expected report values.")
        LOGGER.info("Values found:")
        for expected_idx, expected_element in enumerate(expected_final_values):
            LOGGER.info(f"  -> {expected_element} found: {last_report_matches.get(expected_idx)}")
        asserts.fail("Did not find all expected last report values before time-out")

    def await_all_expected_report_matches(self, expected_matchers: Iterable[AttributeMatcher], timeout_sec: float = 1.0):
        """Expect that every predicate in `expected_matchers`, when run against all the incoming reports, reaches true by the end, ignoring timestamps.

        Waits for at least `timeout_sec` seconds.

        This is a form of barrier for a set of attribute changes that should all happen together for an action.

        Note that this does not check against the "last" value of every attribute, only that each expected
        report was seen at least once.
        """
        start_time = time.time()
        elapsed = 0.0
        time_remaining = timeout_sec

        report_matches: dict[int, bool] = {idx: False for idx, _ in enumerate(expected_matchers)}

        for matcher in expected_matchers:
            LOGGER.info(
                f"--> Matcher waiting: {matcher.description}")
        LOGGER.info(f"Waiting for {timeout_sec:.1f} seconds for all reports.")

        while time_remaining > 0:
            # Snapshot copy at the beginning of the loop. This is thread-safe based on the design.
            all_reports = self._attribute_reports

            # Recompute all last-value matches
            for expected_idx, matcher in enumerate(expected_matchers):
                for attribute, reports in all_reports.items():
                    for report in reports:
                        if matcher.matches(report) and not report_matches[expected_idx]:
                            LOGGER.info(f"  --> Found a match for: {matcher.description}")
                            report_matches[expected_idx] = True

            # Determine if all were met
            if all(report_matches.values()):
                LOGGER.info("Found all expected matchers did match.")
                return

            elapsed = time.time() - start_time
            time_remaining = timeout_sec - elapsed
            time.sleep(0.1)

        # If we reach here, there was no early return and we failed to find all the values.
        LOGGER.error("Reached time-out without finding all expected report values.")
        for expected_idx, expected_matcher in enumerate(expected_matchers):
            LOGGER.info(f"  -> {expected_matcher.description}: {report_matches.get(expected_idx)}")
        asserts.fail("Did not find all expected reports before time-out")

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

        start_time = time.time()
        elapsed = 0.0
        time_remaining = timeout_sec

        sequence_idx = 0
        actual_values = []

        while time_remaining > 0:
            expected_value = sequence[sequence_idx]
            LOGGER.info(f"Expecting value {expected_value} for attribute {attribute} on endpoint {self._endpoint_id}")
            LOGGER.info(f"Waiting for {timeout_sec:.1f} seconds for all reports.")
            try:
                item: AttributeValue = self._q.get(block=True, timeout=time_remaining)

                # Track arrival of all values for the given attribute.
                if item.endpoint_id == self._endpoint_id and item.attribute == attribute:
                    actual_values.append(item.value)

                    if item.value == expected_value:
                        LOGGER.info(f"Got expected attribute change {sequence_idx+1}/{len(sequence)} for attribute {attribute}")
                        sequence_idx += 1
                    else:
                        asserts.assert_equal(item.value, expected_value,
                                             msg=f"Did not get expected attribute value in correct sequence. Sequence so far: {actual_values}")

                    # We are done waiting when we have accumulated all results.
                    if sequence_idx == len(sequence):
                        LOGGER.info("Got all attribute changes, done waiting.")
                        return
            except queue.Empty:
                # No error, we update timeouts and keep going
                pass

            elapsed = time.time() - start_time
            time_remaining = timeout_sec - elapsed

        asserts.fail(f"Did not get full sequence {sequence} in {timeout_sec:.1f} seconds. Got {actual_values} before time-out.")

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

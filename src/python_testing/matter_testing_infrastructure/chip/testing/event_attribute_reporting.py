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
This module contains classes and functions related to device event and attribute repporting
"""

import asyncio
import inspect
import logging
import queue
import threading
import time
from datetime import datetime, timezone
from typing import Any, Iterable, Optional

from chip.clusters import ClusterObjects as ClusterObjects
from chip.clusters.Attribute import EventReadResult, SubscriptionTransaction, TypedAttributePath
from chip.interaction_model import Status
from chip.testing.matter_testing import AttributeMatcher, AttributeValue
from mobly import asserts


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


class ClusterAttributeChangeAccumulator:
    """
    Subscribes to a cluster or single attribute and accumulates attribute reports in a queue.

    If `expected_attribute` is provided, it subscribes only to that specific attribute.
    Otherwise, it subscribes to all attributes from the cluster.

    Args:
        expected_cluster (ClusterObjects.Cluster): The cluster to subscribe to.
        expected_attribute (ClusterObjects.ClusterAttributeDescriptor, optional):
            If provided, subscribes to a single attribute. Defaults to None.
    """

    def __init__(self, expected_cluster: ClusterObjects.Cluster, expected_attribute: ClusterObjects.ClusterAttributeDescriptor = None):
        self._expected_cluster = expected_cluster
        self._expected_attribute = expected_attribute
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
            nodeid=node_id,
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
        """This is the subscription callback when an attribute report is received.
           It checks the report is from the expected_cluster and then posts it into the queue for later processing."""
        valid_report = False
        if path.ClusterType == self._expected_cluster:
            if self._expected_attribute is not None:
                valid_report = path.ClusterId == self._expected_attribute.cluster_id
            else:
                valid_report = True

        if valid_report:
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
            logging.info(
                f"--> Matcher waiting: {matcher.description}")
        logging.info(f"Waiting for {timeout_sec:.1f} seconds for all reports.")

        while time_remaining > 0:
            # Snapshot copy at the beginning of the loop. This is thread-safe based on the design.
            all_reports = self._attribute_reports

            # Recompute all last-value matches
            for expected_idx, matcher in enumerate(expected_matchers):
                for attribute, reports in all_reports.items():
                    for report in reports:
                        if matcher.matches(report) and not report_matches[expected_idx]:
                            logging.info(f"  --> Found a match for: {matcher.description}")
                            report_matches[expected_idx] = True

            # Determine if all were met
            if all(report_matches.values()):
                logging.info("Found all expected matchers did match.")
                return

            elapsed = time.time() - start_time
            time_remaining = timeout_sec - elapsed
            time.sleep(0.1)

        # If we reach here, there was no early return and we failed to find all the values.
        logging.error("Reached time-out without finding all expected report values.")
        for expected_idx, expected_matcher in enumerate(expected_matchers):
            logging.info(f"  -> {expected_matcher.description}: {report_matches.get(expected_idx)}")
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

#
#    Copyright (c) 2022-2026 Project CHIP Authors
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

from __future__ import annotations

import asyncio
import inspect
import json
import logging
import os
import queue
import random
import select
import shlex
import socket
import subprocess
import textwrap
import threading
import time
import typing
from collections.abc import Callable
from dataclasses import asdict, dataclass, fields
from datetime import UTC, datetime, timedelta
from enum import IntFlag
from typing import Any, TypeAlias

import matter.testing.matchers as matchers

# isort: off

from matter import ChipDeviceCtrl, discovery  # Needed before matter.FabricAdmin
import matter.FabricAdmin  # Needed before matter.CertificateAuthority
import matter.CertificateAuthority

# isort: on

from mobly import asserts, base_test, signals

import matter.clusters as Clusters
import matter.logging
import matter.native
import matter.testing.global_stash as global_stash
from matter.clusters import Attribute, ClusterObjects
from matter.clusters.Attribute import SubscriptionTransaction, TypedAttributePath
from matter.clusters.Types import NullValue
from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status
from matter.setup_payload import SetupPayload
from matter.testing.commissioning import (CommissioningInfo, CustomCommissioningParameters, SetupPayloadInfo, commission_device,
                                          commission_devices, get_setup_payload_info_config)
from matter.testing.decorators import _has_attribute, _has_cluster, _has_command, _has_feature
from matter.testing.global_attribute_ids import GlobalAttributeIds
from matter.testing.harness_params import (format_declared_parameters_for_failure, format_missing_test_parameters,
                                           resolve_harness_value)
from matter.testing.matter_stack_state import MatterStackState
from matter.testing.matter_test_config import MatterTestConfig
from matter.testing.pixit import _PIXIT_NO_DEFAULT, get_pixit_definitions
from matter.testing.problem_notices import AttributePathLocation, ClusterMapper, ProblemLocation, ProblemNotice, ProblemSeverity
from matter.testing.runner import TestRunnerHooks, TestStep
from matter.testing.spec_parsing import PrebuiltDataModelDirectory, SpecParsingException, build_xml_clusters
from matter.tlv import uint

# TODO: Add utility to commission a device if needed
# TODO: Add utilities to keep track of controllers/fabrics

# Type aliases for common patterns to improve readability
StepNumber: TypeAlias = int | str  # Test step numbers can be integers or strings
OptionalTimeout: TypeAlias = int | None  # Optional timeout values

LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)

DiscoveryFilterType = ChipDeviceCtrl.DiscoveryFilterType

_SUMMARY_MAX_HEX_CHARS = 128


class TestError(Exception):
    pass


def clear_queue(report_queue: queue.Queue):
    """Flush all contents of a report queue. Useful to get back to empty point."""
    while not report_queue.empty():
        try:
            report_queue.get(block=False)
        except queue.Empty:
            break


def get_first_setup_code(dev_ctrl: ChipDeviceCtrl.ChipDeviceControllerBase, matter_test_config: MatterTestConfig) -> str | None:
    created_codes = []
    for idx, discriminator in enumerate(matter_test_config.discriminators):
        created_codes.append(dev_ctrl.CreateManualCode(discriminator, matter_test_config.setup_passcodes[idx]))

    setup_codes = matter_test_config.qr_code_content + matter_test_config.manual_code + created_codes
    if not setup_codes:
        return None
    return setup_codes[0]


@dataclass
class AttributeValue:
    endpoint_id: int
    attribute: ClusterObjects.ClusterAttributeDescriptor
    value: Any
    timestamp_utc: datetime | None = None


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
    def from_callable(description: str, matcher: Callable[[AttributeValue], bool]) -> AttributeMatcher:
        """Take a single callable and wrap it into an AttributeMatcher object. Useful to wrap closures."""
        class AttributeMatcherFromCallable(AttributeMatcher):
            def __init__(self, description, matcher: Callable[[AttributeValue], bool]):
                super().__init__(description)
                self._matcher = matcher

            def matches(self, report: AttributeValue) -> bool:
                return self._matcher(report)

        return AttributeMatcherFromCallable(description, matcher)


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


class BackgroundWildcardSubscriptionCache:
    """Framework-internal cache backing MatterBaseTest's background wildcard subscription.

    Test authors should not instantiate this directly.  It is created and managed by
    MatterBaseTest._start_wildcard_subscription and consulted from
    verify_attribute_subscription_value (which is invoked transparently by
    read_single_attribute, read_single_attribute_check_success, and
    read_single_attribute_all_endpoints).  For per-attribute or per-event
    subscriptions inside a test, use AttributeSubscriptionHandler or
    EventSubscriptionHandler from matter.testing.event_attribute_reporting
    instead.

    The class plays two roles:
        1. **Subscription callback** — receives every attribute report from a wildcard
           subscription via __call__ and records it.
        2. **Latest-value cache** — exposes the most recent reported value for any
           (endpoint_id, cluster_id, attribute_id) tuple via _latest_values,
           seeded from the priming read in start().

    Attributes with the Changes Omitted (C) or Quieter Reporting (Q) spec quality flags can be
    excluded by passing their (cluster_id, attribute_id) pairs as `excluded_attribute_ids`.
    Excluded attributes are silently dropped from the queue and cache so that subscription
    verification logic does not falsely flag them as missing.

    Attributes:
        _subscription: The active subscription transaction object.
        _excluded_attribute_ids: Frozen set of (cluster_id, attr_id) pairs to ignore.
        _q: Queue of AttributeValue records for received updates.
        _attribute_reports: Dictionary mapping (endpoint, cluster, attribute) tuples to a
            history list of AttributeValue records.
        _latest_values: Dictionary mapping (endpoint_id, cluster_id, attr_id) -> latest value,
            seeded from the priming read and updated on every non-excluded report.
        _lock: Threading lock for thread-safe access to internal data structures.
    """

    def __init__(self, excluded_attribute_ids: frozenset[tuple[int, int]] | None = None):
        """Initialize the background wildcard subscription cache.

        Parameters:
            excluded_attribute_ids: Optional frozenset of (cluster_id, attribute_id) integer
                pairs whose reports should be silently dropped.  Build this set from
                XmlAttribute.changes_omitted / XmlAttribute.quieter_reporting flags in
                spec_parsing to exclude C- and Q-quality attributes from subscription checks.
        """
        self._subscription: Any | None = None
        self._excluded_attribute_ids: frozenset[tuple[int, int]] = excluded_attribute_ids or frozenset()
        self._q: queue.Queue = queue.Queue()
        self._attribute_reports: dict[tuple, list[AttributeValue]] = {}
        # (endpoint_id: int, cluster_id: int, attr_id: int) -> latest reported value.
        # Seeded from the priming read in start() and kept up-to-date by __call__.
        self._latest_values: dict[tuple[int, int, int], Any] = {}
        self._lock = threading.Lock()

    def __call__(self, path: TypedAttributePath, transaction: SubscriptionTransaction):
        """
        Callback invoked when an attribute report is received via subscription.

        Drops reports for attributes in `excluded_attribute_ids` (C/Q quality flags).
        For all other attributes, stores the report in a queue, tracks it in the
        internal history, and updates the latest-value cache.

        Parameters:
            path (TypedAttributePath): Contains endpoint, cluster and attribute metadata for the report.
            transaction (SubscriptionTransaction): Provides access to the actual reported value.
        """
        # Drop C/Q-quality attributes: they are not required to report on every change,
        # so including them in subscription verification would produce false failures.
        if (path.ClusterId, path.AttributeId) in self._excluded_attribute_ids:
            return

        # TypedAttributePath invariants (enforced in its __post_init__) guarantee that
        # ClusterType / AttributeType / ClusterId / AttributeId are populated.  Subscription
        # callbacks from the SDK also populate Path (and Path.EndpointId).  Asserts are for
        # type narrowing; if any of these were ever None at runtime we would want to fail
        # loudly rather than corrupt the cache.
        assert path.Path is not None and path.Path.EndpointId is not None
        assert path.AttributeType is not None
        assert path.ClusterId is not None and path.AttributeId is not None

        data = transaction.GetAttribute(path)

        endpoint_id: int = path.Path.EndpointId
        report_key = (endpoint_id, path.ClusterType, path.AttributeType)
        cache_key: tuple[int, int, int] = (endpoint_id, path.ClusterId, path.AttributeId)

        # Single AttributeValue feeds both the queue (for ordered consumption) and
        # the per-attribute history list.  Matches the AttributeValue contract used
        # by AttributeSubscriptionHandler in event_attribute_reporting.py.
        report = AttributeValue(
            endpoint_id=endpoint_id,
            attribute=path.AttributeType,
            value=data,
            timestamp_utc=datetime.now(UTC),
        )

        self._q.put(report)

        with self._lock:
            if report_key not in self._attribute_reports:
                self._attribute_reports[report_key] = []
            self._attribute_reports[report_key].append(report)
            self._latest_values[cache_key] = data

    async def start(self, dev_ctrl, node_id: int, attributes: list,
                    fabric_filtered: bool = False,
                    min_interval_sec: int = 0,
                    max_interval_sec: int = 30,
                    keepSubscriptions: bool = False,
                    autoResubscribe: bool = False) -> Any:
        """
        Start a wildcard subscription for the specified attribute paths.

        After the subscription is established the priming read values (all attribute values
        returned as part of the initial SubscribeResponse) are used to seed `_latest_values`
        so callers can query the current state of any non-excluded attribute immediately.

        Parameters:
            dev_ctrl: Device controller to use for the subscription.
            node_id: Node ID of the device to subscribe to.
            attributes: List of attribute paths (can include wildcards).
            fabric_filtered: Whether to filter by fabric.  Defaults to False so the cache
                reflects data from all fabrics; comparison logic in
                verify_attribute_subscription_value handles the difference between the
                unfiltered cache and fabric-filtered reads.
            min_interval_sec: Minimum reporting interval in seconds.
            max_interval_sec: Maximum reporting interval in seconds.  Defaults to 30 to
                match the interval used by _start_wildcard_subscription.
            keepSubscriptions: Whether to keep existing subscriptions.
            autoResubscribe: Whether to automatically resubscribe on subscription loss.

        Returns:
            The subscription transaction object.
        """
        self._subscription = await dev_ctrl.ReadAttribute(
            nodeId=node_id,
            attributes=attributes,
            reportInterval=(int(min_interval_sec), int(max_interval_sec)),
            fabricFiltered=fabric_filtered,
            keepSubscriptions=keepSubscriptions,
            autoResubscribe=autoResubscribe
        )
        assert self._subscription is not None
        self._subscription.SetAttributeUpdateCallback(self.__call__)

        # Seed _latest_values from the priming read.  GetAttributes() returns the
        # full attribute cache as {endpoint_id: {ClusterType: {AttributeType: value}}}.
        # The callback is only invoked for *subsequent* updates, so we must populate
        # the initial snapshot here.
        self._seed_latest_values_from_priming_read()

        return self._subscription

    def _seed_latest_values_from_priming_read(self) -> None:
        """Populate _latest_values from the subscription's priming read cache.

        Called once from start() after ReadAttribute returns.  Skips excluded (C/Q)
        attributes and any value that is an error/decode failure.
        """
        if self._subscription is None:
            return

        from matter.clusters.Attribute import ValueDecodeFailure  # local import avoids top-level cost
        try:
            priming_data = self._subscription.GetAttributes()
        except Exception as e:
            LOGGER.warning("[BackgroundWildcardSubscriptionCache] Could not read priming attribute cache: %s", e)
            return

        with self._lock:
            for endpoint_id, clusters in priming_data.items():
                if not isinstance(endpoint_id, int):
                    continue
                for cluster_type, attrs in clusters.items():
                    if not isinstance(attrs, dict):
                        continue
                    try:
                        cluster_id: int = cluster_type.id
                    except AttributeError:
                        continue
                    for attr_type, value in attrs.items():
                        try:
                            attr_id: int = attr_type.attribute_id
                        except AttributeError:
                            continue
                        if (cluster_id, attr_id) in self._excluded_attribute_ids:
                            continue
                        if isinstance(value, ValueDecodeFailure):
                            continue
                        self._latest_values[(endpoint_id, cluster_id, attr_id)] = value

    def was_attribute_reported(self, endpoint_id: int, cluster_type, attribute_type) -> bool:
        """
        Check if a specific attribute has received at least one report.

        Parameters:
            endpoint_id: The endpoint ID to check.
            cluster_type: The cluster class type.
            attribute_type: The attribute class type.

        Returns:
            True if the attribute has been reported, False otherwise.
        """
        report_key = (endpoint_id, cluster_type, attribute_type)
        with self._lock:
            return report_key in self._attribute_reports and len(self._attribute_reports[report_key]) > 0

    def get_attribute_report_count(self, endpoint_id: int, cluster_type, attribute_type) -> int:
        """
        Get the number of reports received for a specific attribute.

        Parameters:
            endpoint_id: The endpoint ID to check.
            cluster_type: The cluster class type.
            attribute_type: The attribute class type.

        Returns:
            Number of reports received for this attribute.
        """
        report_key = (endpoint_id, cluster_type, attribute_type)
        with self._lock:
            return len(self._attribute_reports.get(report_key, []))

    def get_all_reported_attributes(self) -> list[tuple]:
        """
        Get a list of all (endpoint, cluster, attribute) tuples that have received reports.

        Returns:
            List of tuples (endpoint_id, cluster_type, attribute_type).
        """
        with self._lock:
            return list(self._attribute_reports.keys())

    def get_latest_value(self, endpoint_id: int, cluster_id: int, attr_id: int) -> Any | None:
        """Return the most recently reported value for the given attribute, or None if not yet seen.

        Parameters:
            endpoint_id: Endpoint the attribute lives on.
            cluster_id: Integer cluster ID (e.g. 0x0006 for On/Off).
            attr_id: Integer attribute ID within that cluster.

        Returns:
            The latest cached value, or None if no report has been received yet.
        """
        with self._lock:
            return self._latest_values.get((endpoint_id, cluster_id, attr_id))

    @property
    def latest_values(self) -> dict[tuple[int, int, int], Any]:
        """Return a snapshot of the entire latest-value cache.

        Keys are (endpoint_id, cluster_id, attr_id) integer tuples.
        Values are the last reported attribute value (never a ValueDecodeFailure).
        """
        with self._lock:
            return dict(self._latest_values)

    def reset(self) -> None:
        """Reset all tracking data, clearing the queue, report history, and latest-value cache."""
        with self._lock:
            self._attribute_reports.clear()
            self._latest_values.clear()
        self.flush_reports()

    def flush_reports(self) -> None:
        """Flush the entire queue, discarding all pending reports."""
        while True:
            try:
                self._q.get(block=False)
            except queue.Empty:
                return

    @property
    def attribute_queue(self) -> queue.Queue:
        """Get the internal queue of attribute reports."""
        return self._q

    @property
    def subscription(self):
        """Get the underlying subscription transaction object."""
        return self._subscription

    def shutdown(self) -> None:
        """Shutdown the subscription."""
        if self._subscription:
            self._subscription.Shutdown()


@dataclass
class TestCleanupConfig:
    """
    A class to keep track of which cleanup steps should be performed.
    Default behavior: all cleanup steps are enabled. Test classes can disable individual steps by
    setting flags in setup_test or in the test method body, after calling super().
    """

    # DUT clean-up items
    disarm_failsafes: bool = True              # sends ArmFailSafe(expiryLengthSeconds=0) on GeneralCommissioning
    reset_acls_to_default: bool = True         # restores ACL on endpoint 0 to the state captured before the test ran
    close_commissioning_windows: bool = True   # sends RevokeCommissioning on AdministratorCommissioning
    remove_extra_fabrics: bool = True          # removes all fabrics on the DUT except TH1's via RemoveFabric
    purge_scenes: bool = True                  # calls RemoveAllScenes per group on every ScenesManagement endpoint
    purge_groups: bool = True                  # removes all non-IPK group key sets and clears GroupKeyMap
    purge_group_memberships: bool = True       # calls RemoveAllGroups on every Groups endpoint
    purge_doorlock: bool = True                # clears all DoorLock credentials and users
    purge_tls_endpoints: bool = True           # removes all provisioned endpoints via TlsClientManagement
    unregister_icd_clients: bool = True        # unregisters all entries from IcdManagement.RegisteredClients

    # Controller clean-up items
    shutdown_extra_controllers: bool = True    # shuts down extra controllers and removes their CAs from storage

    @classmethod
    def disabled(cls) -> TestCleanupConfig:
        """Returns a config with all cleanup steps disabled."""
        return cls(**{f.name: False for f in fields(cls)})


class MatterBaseTest(base_test.BaseTestClass):
    """Base class for Matter Python tests.

    Wildcard subscription (see setup_test):

    * Set class attribute requires_dut = False for tests that do not interact with a
      real DUT (e.g. parser/conformance unit tests under test_testing/).  Such tests
      will skip the background wildcard subscription so they don't try to subscribe to a
      device that isn't there.  Default is True.
    * Set class attribute disable_wildcard_subscription = True to skip the background
      wildcard subscription and its ACL side effects — same effect as --no-wildcard-subscription.
    * When a wildcard subscription is active, read_single_attribute_check_success compares
      each read to the subscription cache unless verify_wildcard_subscription=False is passed,
      or the class sets default_verify_wildcard_subscription = False.
    """
    requires_dut: bool = True

    def __init_subclass__(cls, **kwargs):
        super().__init_subclass__(**kwargs)
        if 'teardown_test' in cls.__dict__:
            original = cls.__dict__['teardown_test']

            def _wrapped_teardown(self, _original=original):
                _original(self)
                MatterBaseTest.teardown_test(self)

            cls.teardown_test = _wrapped_teardown

    def __init__(self, *args):
        super().__init__(*args)

        # List of accumulated problems across all tests
        self.problems = []
        self.is_commissioning = False
        self.cached_steps: dict[str, list[TestStep] | None] = {}
        self.cleanup_config = TestCleanupConfig()
        self._extra_controllers: list[ChipDeviceCtrl.ChipDeviceController] = []
        self._extra_cas: list[matter.CertificateAuthority.CertificateAuthority] = []
        self._original_acl = None
        self._framework_cleanup_done = False
        # Set to True by commission_devices() on success; gates the per-test ACL read in
        # setup_test so unit tests (which never commission) incur zero network overhead.
        self._dut_confirmed_available = False
        # Prevents double-execution when the override calls super().teardown_test()
        # and __init_subclass__ also calls it afterward.
        self._teardown_ran = False

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

        # Set a hook on FabricAdmin so every NewController() call during this test automatically
        # populates self._extra_controllers. This is used during cleanup in teardown_class.
        matter.FabricAdmin.FabricAdmin._new_controller_hook = self._on_new_controller_created

        # Mappings of cluster IDs to names and metadata.
        # TODO: Move to using non-generated code and rather use data model description (.matter or .xml)
        self.cluster_mapper = ClusterMapper(self.default_controller._Cluster)
        self.current_step_index = 0
        self.step_start_time = datetime.now(UTC)
        self.step_skipped = False
        # self.stored_global_wildcard stores value of self.global_wildcard after first async call.
        # Because setup_class can be called before commissioning, this variable is lazy-initialized
        # where the read is deferred until the first guard function call that requires global attributes.
        self.stored_global_wildcard = None

        # Populated by _start_wildcard_subscription (called from setup_test) with the
        # BackgroundWildcardSubscriptionCache that drives subscription-cache verification.
        # Shut down in teardown_test.
        self.wildcard_subscription_handler = None

        # Secondary controller for the background wildcard subscription.  A separate node_id
        # is used so that keepSubscriptions=False on default_controller does not cancel this
        # background subscription (they use different CASE sessions).
        # Created lazily in _start_wildcard_subscription; shut down in teardown_class.
        self.subscription_controller = None

        # ACL snapshot taken immediately before the subscription controller's Administer
        # entry is appended.  Restored in teardown_test so every test sees a clean,
        # unmodified ACL regardless of what the test did to it.
        self._pre_subscription_acl = None

        # (cluster_id, attr_id) pairs excluded from the background wildcard subscription:
        # attributes carrying the Changes Omitted (C) or Quieter Reporting (Q) spec quality
        # flags, unioned with _CQ_EXPECTED_BUT_NOT_YET_MARKED for attributes that should be C
        # in practice but are not yet flagged in the data-model XML.  Built once per class
        # here (not per test) because the underlying XML parse is expensive.
        self._cq_excluded_attr_ids: frozenset[tuple[int, int]] = self._build_cq_excluded_ids()

    def teardown_class(self):
        """Final teardown after all tests: run framework cleanup, log problems, dump attributes.

        Runs _run_framework_cleanup() once after all test methods in the class have
        completed. Framework cleanup is class-scoped rather than per-test because
        multi-test classes run sequentially — between-test cleanup is not required
        and would interfere with intentional class-scoped DUT state.

        Test authors may overwrite this method in the derived class to perform teardown
        that is common for all tests. This function is called only once per class.
        Test authors that implement this method should ensure super().teardown_class()
        is called after any custom teardown code.
        """
        self.event_loop.run_until_complete(self._run_framework_cleanup())

        # Clear the hook set in setup_class; self._extra_controllers, if any, is fully populated by now.
        matter.FabricAdmin.FabricAdmin._new_controller_hook = None

        if len(self.problems) > 0:
            # Attempt to dump device attribute data for debugging when problems are found during Confirmation Tests
            if getattr(self, 'matter_test_config', None) is not None and self.matter_test_config.debug:
                self._dump_device_attributes_on_failure()

            LOGGER.info("###########################################################")
            LOGGER.info("Problems found:")
            LOGGER.info("===============")
            for problem in self.problems:
                LOGGER.info(str(problem))
            LOGGER.info("###########################################################")

        if getattr(self, 'subscription_controller', None) is not None:
            try:
                self.subscription_controller.Shutdown()
            except Exception as e:
                LOGGER.warning("[MatterBaseTest] Error shutting down subscription controller: %s", e)
            self.subscription_controller = None

        self._log_execution_parameters_summary()
        super().teardown_class()

    _CQ_EXPECTED_BUT_NOT_YET_MARKED: frozenset[tuple[int, int]] = frozenset({
    })

    @staticmethod
    def _build_cq_excluded_ids(
            dm_dir: PrebuiltDataModelDirectory = PrebuiltDataModelDirectory.k1_6
    ) -> frozenset[tuple[int, int]]:
        """Return a frozenset of (cluster_id, attr_id) pairs for C/Q-quality attributes.

        Parses the Matter spec data-model XML for `dm_dir` and collects every attribute
        that carries the Changes Omitted (changeOmitted="true") or Quieter Reporting
        (quieterReporting="true") quality flag, then unions in
        _CQ_EXPECTED_BUT_NOT_YET_MARKED for attributes that should be C in practice
        but are not yet flagged in the XML (see that constant for the tracking issues).

        These attributes are excluded from the background wildcard subscription so that
        subscription-verification logic does not flag them as unexpectedly silent.

        Args:
            dm_dir: Which pre-built DM version to use.  Override in subclasses if the
                    device's spec version is known (e.g. via BasicInformation.SpecificationVersion).
        """
        try:
            xml_clusters, _ = build_xml_clusters(dm_dir)
        except SpecParsingException as e:
            LOGGER.warning("Could not build XML clusters for C/Q exclusion set: %s", e)
            return MatterBaseTest._CQ_EXPECTED_BUT_NOT_YET_MARKED

        xml_ids = frozenset(
            (int(cluster_id), int(attr_id))
            for cluster_id, cluster in xml_clusters.items()
            for attr_id, attr in cluster.attributes.items()
            if attr.changes_omitted or attr.quieter_reporting or attr.atomic_write
        )
        return xml_ids | MatterBaseTest._CQ_EXPECTED_BUT_NOT_YET_MARKED

    def _wildcard_subscription_disabled(self) -> bool:
        """True if this run must not start the background wildcard subscription."""
        return (
            self.matter_test_config.no_wildcard_subscription
            or getattr(self, "disable_wildcard_subscription", False)
        )

    def _effective_verify_wildcard_subscription(self, verify_wildcard_subscription: bool | None) -> bool:
        """Resolve whether to compare a read against the wildcard subscription cache."""
        if verify_wildcard_subscription is not None:
            return verify_wildcard_subscription
        return bool(getattr(type(self), "default_verify_wildcard_subscription", True))

    def _start_wildcard_subscription(self) -> None:
        """Start a background wildcard attribute subscription on the DUT.

        Subscribes to *all* attributes on all endpoints via a wildcard path.  The
        subscription handler's callback silently drops reports for attributes that carry
        Changes Omitted (C) or Quieter Reporting (Q) spec quality flags (using the
        exclusion set built in setup_class), so those never enter the cache.  All
        other reports update a latest-value cache that verify_attribute_subscription_value
        compares against direct reads.

        The subscription uses fabricFiltered=False so the cache reflects data from all
        fabrics.  Comparison logic in verify_attribute_subscription_value and
        _fabric_filtered_match handles the difference between the unfiltered cache and
        fabric-filtered reads.

        A secondary controller (node_id = controller_node_id + 123456) is used so that
        keepSubscriptions=False from default_controller (issued by tests that manage their
        own subscriptions) does not cancel this background subscription.

        Before the subscription starts the current ACL is snapshotted into
        _pre_subscription_acl and a single Administer entry for the secondary controller is
        appended.  teardown_test restores the ACL from the snapshot so every test sees an
        unmodified ACL regardless of what the test did to it.

        autoResubscribe=False is intentional: if a test removes the subscription
        controller's ACL entry (e.g. via a full ACL overwrite), the subscription stops
        receiving reports rather than repeatedly retrying with failing re-subscriptions.

        The subscription handler is stored as self.wildcard_subscription_handler and is
        shut down automatically in teardown_test.

        This is a synchronous wrapper around an async operation; it uses self.event_loop
        (set by the test runner before setup_class is called).
        """
        LOGGER.info("[MatterBaseTest] Building wildcard subscription handler")
        handler = BackgroundWildcardSubscriptionCache(
            excluded_attribute_ids=self._cq_excluded_attr_ids
        )

        subscription_node_id = self.matter_test_config.controller_node_id + 123456

        async def _start():
            # Create the secondary controller once per class; reused across all tests.
            if self.subscription_controller is None:
                fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
                self.subscription_controller = fabric_admin.NewController(
                    nodeId=subscription_node_id,
                    paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path),
                )
                LOGGER.info("[MatterBaseTest] Subscription controller created "
                            "(node_id=0x%016X)", subscription_node_id)

                # NewController fired _new_controller_hook synchronously, which registered
                # this controller AND its certificate authority for generic cleanup in
                # _shutdown_extra_controllers.  That CA is the SHARED default CA
                # (activeCaList[0]) the default controller also uses; removing it from
                # admin_storage.json corrupts fabric state for any subsequent
                # --no-factory-reset run, which then finds no CA in storage, mints a new one
                # with a different root key, and can no longer resolve the DUT it commissioned
                # under the original fabric.  The subscription controller is shut down
                # explicitly in teardown_class, so untrack it here and make sure the shared CA
                # is never scheduled for removal.
                self.subscription_controller._skip_cleanup_tracking = True
                if self.subscription_controller in self._extra_controllers:
                    self._extra_controllers.remove(self.subscription_controller)
                sub_ca = self.subscription_controller.fabricAdmin.certificateAuthority
                if sub_ca in self._extra_cas:
                    self._extra_cas.remove(sub_ca)

            # Snapshot the current ACL, then append an Administer entry for the subscription
            # controller.  teardown_test will restore from this snapshot after every test.
            acl_result = await self.default_controller.ReadAttribute(
                nodeId=self.dut_node_id,
                attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
            )
            self._pre_subscription_acl = (
                acl_result[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]
            )
            sub_entry = Clusters.AccessControl.Structs.AccessControlEntryStruct(
                privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
                authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
                subjects=[subscription_node_id],
                targets=NullValue,
            )
            await self.default_controller.WriteAttribute(
                nodeId=self.dut_node_id,
                attributes=[(0, Clusters.AccessControl.Attributes.Acl(
                    list(self._pre_subscription_acl) + [sub_entry]
                ))],
            )

            # Start the wildcard subscription using the secondary controller.
            await handler.start(
                dev_ctrl=self.subscription_controller,
                node_id=self.dut_node_id,
                attributes=[Attribute.AttributePath(None, None, None)],
                min_interval_sec=0,
                max_interval_sec=30,
                keepSubscriptions=False,
                autoResubscribe=False,
            )

        try:
            LOGGER.info("[MatterBaseTest] Starting wildcard subscription")
            self.event_loop.run_until_complete(_start())
            self.wildcard_subscription_handler = handler
            LOGGER.info(
                "[MatterBaseTest] Wildcard subscription started (%d C/Q attrs excluded, "
                "%d attrs cached from priming read)",
                len(self._cq_excluded_attr_ids), len(handler.latest_values),
            )
        except Exception as e:
            LOGGER.warning("[MatterBaseTest] Could not start wildcard subscription: %s", e)
            if self._pre_subscription_acl is not None:
                try:
                    self.event_loop.run_until_complete(
                        self.default_controller.WriteAttribute(
                            nodeId=self.dut_node_id,
                            attributes=[(0, Clusters.AccessControl.Attributes.Acl(list(self._pre_subscription_acl)))],
                        )
                    )
                except Exception as restore_err:
                    LOGGER.warning("[MatterBaseTest] Error restoring ACL after wildcard subscription failure: %s", restore_err)
            self._pre_subscription_acl = None
            self.wildcard_subscription_handler = None

    def get_subscription_acl_entry(self):
        """Return the ACL entry for the subscription controller, or None.

        Tests that write ACLs can include this entry in their ACL list to maintain
        subscription coverage during the test.  Without it, the subscription controller
        loses access and subscription reports are denied by the DUT.

        Example usage in a test::

            my_acl = [primary_entry, other_entry]
            sub_entry = self.get_subscription_acl_entry()
            if sub_entry is not None:
                my_acl.append(sub_entry)
            await self.default_controller.WriteAttribute(...)
        """
        if getattr(self, 'subscription_controller', None) is None:
            return None
        sub_node_id = self.matter_test_config.controller_node_id + 123456
        return Clusters.AccessControl.Structs.AccessControlEntryStruct(
            privilege=Clusters.AccessControl.Enums.AccessControlEntryPrivilegeEnum.kAdminister,
            authMode=Clusters.AccessControl.Enums.AccessControlEntryAuthModeEnum.kCase,
            subjects=[sub_node_id],
            targets=NullValue,
        )

    async def async_teardown_test(self) -> None:
        """Override to add async class-level teardown without @async_test_body boilerplate.

        Called once from _run_framework_cleanup (which runs in teardown_class) before
        any framework cleanup steps run, so the DUT and all controllers are still
        fully available.

        For per-test async teardown, override teardown_test with @async_test_body instead.
        """
        pass

    async def _run_framework_cleanup(self) -> None:
        """Runs all enabled cleanup steps once at class teardown.

        Called from teardown_class after all test methods have run. DUT-side cleanup
        runs first (while the default controller is still active), followed by
        controller-side cleanup. Each step is gated by TestCleanupConfig so individual
        steps can be disabled by test authors when needed.

        Wildcard attributes are pre-fetched once so all cluster-presence checks within
        a single cleanup pass share the same read.
        """

        # If a teardown_test override already called this (per-test cleanup
        # opt-in), teardown_class will skip it to avoid running cleanup twice.
        if self._framework_cleanup_done:
            return
        self._framework_cleanup_done = True
        await self.async_teardown_test()

        # If setup_test could not read the ACL, the DUT was unreachable at test
        # start, skip DUT cleanup to avoid a slow network discovery attempt.
        dut_reachable = self._original_acl is not None
        if dut_reachable:
            try:
                # Lightweight reachability check, confirm the DUT is still alive before attempting cleanup.
                await self.default_controller.Read(
                    self.dut_node_id,
                    [(0, Clusters.BasicInformation.Attributes.VendorID)]
                )
            except Exception as e:  # DUT may be unreachable or mid-reboot; skip all DUT cleanup rather than failing the test
                LOGGER.warning("[CLN] DUT is unreachable, skipping all DUT cleanup: %s", e)
                dut_reachable = False

        if dut_reachable:
            await self._populate_wildcard()
            # DUT cleanup (run first as controller must still be alive to send commands)
            # - Scenes must be removed before group memberships: RemoveAllScenes requires the target
            #   group to still exist on the DUT, so group memberships cannot be cleared first.
            if self.cleanup_config.disarm_failsafes:
                await self._disarm_failsafes()
            if self.cleanup_config.reset_acls_to_default:
                await self._reset_acls_to_default()
            if self.cleanup_config.close_commissioning_windows:
                await self._close_commissioning_windows()
            if self.cleanup_config.remove_extra_fabrics:
                await self._remove_extra_fabrics()
            if self.cleanup_config.purge_scenes:
                await self._purge_scenes()
            if self.cleanup_config.purge_groups:
                await self._purge_groups()
            if self.cleanup_config.purge_group_memberships:
                await self._purge_group_memberships()
            if self.cleanup_config.purge_doorlock:
                await self._purge_doorlock()
            if self.cleanup_config.purge_tls_endpoints:
                await self._purge_tls_endpoints()
            if self.cleanup_config.unregister_icd_clients:
                await self._unregister_icd_clients()

        # Controller cleanup (runs regardless, no DUT connection needed)
        if self.cleanup_config.shutdown_extra_controllers:
            self._shutdown_extra_controllers()

    def _on_new_controller_created(self, controller: ChipDeviceCtrl.ChipDeviceController) -> None:
        """Hook set and fired by FabricAdmin for every NewController() call.

        Skips the default controller and any controller that opts out of per-test
        cleanup tracking by setting _skip_cleanup_tracking = True. Use the opt-out
        for class-scoped controllers that must outlive individual tests.

        Args:
            controller: The controller that was created.
        """
        if not getattr(controller, '_is_default_controller', False) and not getattr(controller, '_skip_cleanup_tracking', False):

            # Track controller for shutdown in teardown_class
            self._extra_controllers.append(controller)

            # Track the controller's CA so it can be removed from
            # persistent storage after controller shutdown
            fa = controller.fabricAdmin
            if fa is not None:
                ca = fa.certificateAuthority
                if ca not in self._extra_cas:
                    self._extra_cas.append(ca)

    def _shutdown_extra_controllers(self) -> None:
        """Shuts down all extra controllers created during the test run and
        removes their CAs from persistent storage (admin_storage.json).
        """
        for ctrl in self._extra_controllers:
            try:
                LOGGER.info("[CLN] shutting down controller nodeId=%#x", ctrl.nodeId)
                ctrl.Shutdown()
                LOGGER.info("[CLN] controller nodeId=%#x shut down successfully", ctrl.nodeId)
            except Exception as e:  # Shutdown can fail if the controller is already stopped or the stack is in a bad state
                LOGGER.warning("[CLN] controller shutdown failed: %s", e)
        self._extra_controllers.clear()

        # Shut down each CA and remove it from the manager's active list and from
        # persistent storage (caList in admin_storage.json) directly.
        mgr = self.certificate_authority_manager

        # Never tear down the CA the default controller depends on. Removing it from
        # admin_storage.json corrupts fabric state for subsequent --no-factory-reset runs,
        # which then mint a fresh CA with a new root key and can no longer resolve a DUT
        # commissioned under the original fabric.
        default_ca = None
        try:
            default_ctrl = self.default_controller
            if default_ctrl is not None and default_ctrl.fabricAdmin is not None:
                default_ca = default_ctrl.fabricAdmin.certificateAuthority
        except Exception:
            default_ca = None

        for ca in self._extra_cas:
            if ca is default_ca:
                LOGGER.info("[CLN] skipping shared default CA index %d (still in use)", ca.caIndex)
                continue
            try:
                LOGGER.info("[CLN] shutting down CA index %d", ca.caIndex)
                ca.Shutdown()
                if ca in mgr._activeCaList:
                    mgr._activeCaList.remove(ca)
                ca_list = mgr._persistentStorage.GetKey('caList') or {}
                if str(ca.caIndex) in ca_list:
                    del ca_list[str(ca.caIndex)]
                    mgr._persistentStorage.SetKey('caList', ca_list)
                LOGGER.info("[CLN] CA index %d removed successfully", ca.caIndex)
            except Exception as e:  # Storage may be inconsistent if the controller was shut down in a bad state
                LOGGER.warning("[CLN] CA removal failed: %s", e)
        self._extra_cas.clear()

    async def _disarm_failsafes(self) -> None:
        """Sends ArmFailSafe(expiryLengthSeconds=0) to disarm any active failsafe on the DUT."""
        LOGGER.info("[CLN] sending ArmFailSafe(0) to disarm any active failsafe")
        try:
            resp = typing.cast(
                Clusters.GeneralCommissioning.Commands.ArmFailSafeResponse,
                await self.send_single_cmd(
                    cmd=Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=uint(0)),
                    endpoint=0
                )
            )
            if resp.errorCode != Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk:
                LOGGER.warning("[CLN] disarm failsafe returned errorCode %s", resp.errorCode)
            else:
                LOGGER.info("[CLN] failsafe disarmed successfully")
        except Exception as e:  # DUT may be unreachable or session may have expired; log and continue cleanup
            LOGGER.warning("[CLN] disarm failsafe failed: %s", e)

    async def _reset_acls_to_default(self) -> None:
        """Restores the ACL on endpoint 0 to the state captured before the test ran.

        Uses the ACL saved in setup_test (_original_acl).
        """
        if self._original_acl is None:
            LOGGER.warning("[CLN] no pre-test ACL captured, skipping ACL restore")
            return
        LOGGER.info("[CLN] restoring ACL to pre-test state")
        try:
            result = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(0, Clusters.AccessControl.Attributes.Acl(self._original_acl))]
            )
            if result[0].Status != Status.Success:
                LOGGER.warning("[CLN] ACL reset returned status %s", result[0].Status)
            else:
                LOGGER.info("[CLN] ACL restored successfully")
        except Exception as e:  # Session may have expired or DUT ACL may be in a state that rejects the write
            LOGGER.warning("[CLN] ACL reset failed: %s", e)

    async def _remove_extra_fabrics(self) -> None:
        """Removes any fabric on the DUT that is not the default controller's fabric."""
        try:
            # Read TH1's fabric index on the DUT via the default controller
            th1_fabric_index = await self.read_single_attribute_check_success(
                cluster=Clusters.OperationalCredentials,  # type: ignore[arg-type]
                attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex,
                endpoint=0
            )

            # Read all fabrics unfiltered so we see every fabric, not just TH1's
            fabrics = typing.cast(
                list[Clusters.OperationalCredentials.Structs.FabricDescriptorStruct],
                await self.read_single_attribute_check_success(
                    cluster=Clusters.OperationalCredentials,  # type: ignore[arg-type]
                    attribute=Clusters.OperationalCredentials.Attributes.Fabrics,
                    endpoint=0,
                    fabric_filtered=False
                )
            )
        except Exception as e:  # DUT may be unreachable or session may have expired after a multi-fabric test
            LOGGER.warning(
                "[CLN] could not read fabric list (DUT unreachable, session expired, or attribute read error), skipping fabric removal: %s", e)
            return

        extra_fabric_indices = [f.fabricIndex for f in fabrics if f.fabricIndex != th1_fabric_index]

        if not extra_fabric_indices:
            LOGGER.info("[CLN] no extra fabrics to remove")
            return

        LOGGER.info("[CLN] removing %d extra fabric(s) from DUT", len(extra_fabric_indices))
        for fabric_index in extra_fabric_indices:
            LOGGER.info("[CLN] sending RemoveFabric(fabricIndex=%d)", fabric_index)
            try:
                await self.send_single_cmd(
                    cmd=Clusters.OperationalCredentials.Commands.RemoveFabric(fabricIndex=fabric_index),
                    endpoint=0
                )
                LOGGER.info("[CLN] fabric index %d removed successfully", fabric_index)
            except Exception as e:  # RemoveFabric may fail if the fabric was already removed by the test or a prior cleanup
                LOGGER.warning("[CLN] RemoveFabric(%d) failed: %s", fabric_index, e)

    async def _purge_groups(self) -> None:
        """Removes all non-IPK group key sets and clears the group key map on the DUT.

        Key set 0 (IPK) is skipped as it cannot be removed.
        """
        LOGGER.info("[CLN] purging group key sets and key map")
        try:
            resp = typing.cast(
                Clusters.GroupKeyManagement.Commands.KeySetReadAllIndicesResponse,
                await self.send_single_cmd(
                    cmd=Clusters.GroupKeyManagement.Commands.KeySetReadAllIndices(),
                    endpoint=0
                )
            )

            # Remove all non-IPK key sets, key set 0 (IPK) cannot be removed
            for key_set_id in resp.groupKeySetIDs:
                if key_set_id != 0:
                    LOGGER.info("[CLN] removing group key set %d", key_set_id)
                    await self.send_single_cmd(
                        cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=key_set_id),
                        endpoint=0
                    )
        except Exception as e:  # DUT may be unreachable, or key sets may already be absent; skip rather than aborting cleanup
            LOGGER.warning("[CLN] key set removal failed: %s", e)

        # Clear all group key mappings
        try:
            result = await self.default_controller.WriteAttribute(
                self.dut_node_id,
                [(0, Clusters.GroupKeyManagement.Attributes.GroupKeyMap([]))]
            )
            if result[0].Status != Status.Success:
                LOGGER.warning("[CLN] GroupKeyMap clear returned status %s", result[0].Status)
            else:
                LOGGER.info("[CLN] group key map cleared successfully")
        except Exception as e:  # Write may fail if session expired or the DUT rejected the empty map
            LOGGER.warning("[CLN] GroupKeyMap clear failed: %s", e)

    async def _purge_scenes(self) -> None:
        """Removes all scenes from all groups on every endpoint that has ScenesManagement.

        Must run before _purge_group_memberships since RemoveAllScenes needs the group to
        still exist on the DUT.
        """
        if self.stored_global_wildcard is None:
            LOGGER.info("[CLN] wildcard not available, skipping scene cleanup")
            return

        for endpoint_id in self.stored_global_wildcard.attributes:
            if not _has_cluster(wildcard=self.stored_global_wildcard, endpoint=endpoint_id,
                                cluster=Clusters.ScenesManagement):  # type: ignore[arg-type]
                continue
            if not _has_cluster(wildcard=self.stored_global_wildcard, endpoint=endpoint_id,
                                cluster=Clusters.Groups):  # type: ignore[arg-type]
                continue
            try:
                resp = typing.cast(
                    Clusters.Groups.Commands.GetGroupMembershipResponse,
                    await self.send_single_cmd(
                        cmd=Clusters.Groups.Commands.GetGroupMembership(groupList=[]),
                        endpoint=endpoint_id
                    )
                )
                group_ids = resp.groupList
                if not group_ids:
                    continue
                LOGGER.info("[CLN] removing scenes for groups %s on endpoint %d", group_ids, endpoint_id)
                for gid in group_ids:
                    await self.send_single_cmd(
                        cmd=Clusters.ScenesManagement.Commands.RemoveAllScenes(groupID=gid),
                        endpoint=endpoint_id
                    )
                LOGGER.info("[CLN] scenes cleared on endpoint %d", endpoint_id)
            except Exception as e:  # DUT may be unreachable or the group may have been removed by the test
                LOGGER.warning("[CLN] scene removal failed on endpoint %d: %s", endpoint_id, e)

    async def _purge_group_memberships(self) -> None:
        """Removes all group memberships from the DUT's group table.

        Must run after _purge_scenes since scenes need their groups to still exist for RemoveAllScenes.
        """
        if self.stored_global_wildcard is None:
            LOGGER.info("[CLN] wildcard not available, skipping group membership cleanup")
            return

        found_any = False
        for endpoint_id in self.stored_global_wildcard.attributes:
            if not _has_cluster(wildcard=self.stored_global_wildcard, endpoint=endpoint_id,
                                cluster=Clusters.Groups):  # type: ignore[arg-type]
                continue
            found_any = True
            LOGGER.info("[CLN] sending RemoveAllGroups on endpoint %d", endpoint_id)
            try:
                await self.send_single_cmd(
                    cmd=Clusters.Groups.Commands.RemoveAllGroups(),
                    endpoint=endpoint_id
                )
                LOGGER.info("[CLN] group memberships cleared on endpoint %d", endpoint_id)
            except Exception as e:  # DUT may be unreachable or session may have expired after a multi-fabric test
                LOGGER.warning("[CLN] RemoveAllGroups failed on endpoint %d: %s", endpoint_id, e)
        if not found_any:
            LOGGER.info("[CLN] Groups cluster not present on any endpoint, skipping group membership cleanup")

    async def _purge_doorlock(self) -> None:
        """Clears all DoorLock users and credentials on every endpoint with the DoorLock cluster."""
        if self.stored_global_wildcard is None:
            LOGGER.info("[CLN] wildcard not available, skipping DoorLock cleanup")
            return

        found_any = False
        for endpoint_id in self.stored_global_wildcard.attributes:
            if not _has_cluster(wildcard=self.stored_global_wildcard, endpoint=endpoint_id,
                                cluster=Clusters.DoorLock):  # type: ignore[arg-type]
                continue
            found_any = True
            LOGGER.info("[CLN] clearing DoorLock users and credentials on endpoint %d", endpoint_id)
            try:
                await self.send_single_cmd(
                    cmd=Clusters.DoorLock.Commands.ClearCredential(credential=NullValue),
                    endpoint=endpoint_id,
                    timedRequestTimeoutMs=1000
                )
                await self.send_single_cmd(
                    cmd=Clusters.DoorLock.Commands.ClearUser(userIndex=uint(0xFFFE)),
                    endpoint=endpoint_id,
                    timedRequestTimeoutMs=1000
                )
                LOGGER.info("[CLN] DoorLock users and credentials cleared on endpoint %d", endpoint_id)
            except Exception as e:  # DUT may be unreachable or DoorLock may be in a state that rejects the clear
                LOGGER.warning("[CLN] DoorLock cleanup failed on endpoint %d: %s", endpoint_id, e)
        if not found_any:
            LOGGER.info("[CLN] DoorLock cluster not present on any endpoint, skipping DoorLock cleanup")

    async def _purge_tls_endpoints(self) -> None:
        """Removes all provisioned TLS endpoints on every endpoint with TlsClientManagement.

        Uses stored_global_wildcard (pre-populated by _run_framework_cleanup) to locate
        TlsClientManagement via ServerList, no extra network read needed.
        """
        tls_cluster_id = Clusters.TlsClientManagement.id
        found_any = False
        for endpoint_id, clusters in self.stored_global_wildcard.attributes.items():
            server_list = clusters.get(Clusters.Descriptor, {}).get(Clusters.Descriptor.Attributes.ServerList)
            if server_list is None or tls_cluster_id not in server_list:
                continue
            found_any = True
            try:
                provisioned = typing.cast(
                    list[Clusters.TlsClientManagement.Structs.TLSEndpointStruct],
                    await self.read_single_attribute_check_success(
                        cluster=Clusters.TlsClientManagement,  # type: ignore[arg-type]
                        attribute=Clusters.TlsClientManagement.Attributes.ProvisionedEndpoints,
                        endpoint=endpoint_id
                    )
                )
                if not provisioned:
                    LOGGER.info("[CLN] no TLS endpoints provisioned on endpoint %d", endpoint_id)
                    continue
                LOGGER.info("[CLN] removing %d TLS endpoint(s) on endpoint %d", len(provisioned), endpoint_id)
                for tls_ep in provisioned:
                    await self.send_single_cmd(
                        cmd=Clusters.TlsClientManagement.Commands.RemoveEndpoint(endpointID=tls_ep.endpointID),
                        endpoint=endpoint_id,
                        payloadCapability=ChipDeviceCtrl.TransportPayloadCapability.LARGE_PAYLOAD
                    )
                LOGGER.info("[CLN] TLS endpoints removed on endpoint %d", endpoint_id)
            except Exception as e:  # DUT may be unreachable or TLS endpoint may have already been removed
                LOGGER.warning("[CLN] TLS endpoint cleanup failed on endpoint %d: %s", endpoint_id, e)
        if not found_any:
            LOGGER.info("[CLN] TlsClientManagement cluster not present on any endpoint, skipping TLS endpoint cleanup")

    async def _close_commissioning_windows(self) -> None:
        """Sends RevokeCommissioning to close any open commissioning window on the DUT.

        If no window is open the DUT returns an error, which is expected and logged as info.
        """
        LOGGER.info("[CLN] revoking any open commissioning window")
        try:
            await self.send_single_cmd(
                cmd=Clusters.AdministratorCommissioning.Commands.RevokeCommissioning(),
                endpoint=0,
                timedRequestTimeoutMs=6000
            )
            LOGGER.info("[CLN] commissioning window revoked successfully")
        except Exception as e:  # Expected when no commissioning window is open; the DUT returns an error in that case
            LOGGER.info("[CLN] RevokeCommissioning skipped (likely no window open): %s", e)

    async def _unregister_icd_clients(self) -> None:
        """Unregisters all ICD clients registered on the DUT via the default controller"""
        # Check if the ICD Management cluster is present on the DUT.
        # Wildcard is pre-populated by _run_framework_cleanup; guard for standalone calls.
        if self.stored_global_wildcard is None:
            LOGGER.info("[CLN] wildcard not available, skipping ICD client cleanup")
            return
        if not _has_attribute(wildcard=self.stored_global_wildcard, endpoint=0,
                              attribute=Clusters.IcdManagement.Attributes.RegisteredClients):  # type: ignore[arg-type]
            LOGGER.info("[CLN] ICD Management cluster not present, skipping ICD client cleanup")
            return

        registered_clients = self.stored_global_wildcard.attributes.get(0, {}).get(
            Clusters.IcdManagement, {}).get(Clusters.IcdManagement.Attributes.RegisteredClients)

        if not registered_clients:
            LOGGER.info("[CLN] no ICD clients registered, skipping")
            return

        LOGGER.info("[CLN] unregistering %d ICD client(s)", len(registered_clients))
        for entry in registered_clients:
            try:
                await self.send_single_cmd(
                    cmd=Clusters.IcdManagement.Commands.UnregisterClient(
                        checkInNodeID=entry.checkInNodeID
                    ),
                    endpoint=0
                )
                LOGGER.info("[CLN] unregistered ICD client %#x", entry.checkInNodeID)
            except Exception as e:  # DUT may be unreachable or the client may have already been unregistered
                LOGGER.warning("[CLN] UnregisterClient(%#x) failed: %s", entry.checkInNodeID, e)

    def _format_summary_value(self, key: str, value: Any) -> str:
        """Format values for end-of-test summary logs."""
        if isinstance(value, bytes):
            hex_value = value.hex()
            if len(hex_value) > _SUMMARY_MAX_HEX_CHARS:
                return f"0x{hex_value[:_SUMMARY_MAX_HEX_CHARS]}... (truncated, {len(value)} bytes)"
            return f"0x{hex_value}"
        if isinstance(value, list) and len(value) > 8:
            head = ", ".join(repr(v) for v in value[:5])
            return f"[{head}, ...] (len={len(value)})"
        if key == "pics" and isinstance(value, dict):
            return "Please request if needed"
        return repr(value)

    def _log_execution_parameters_summary(self):
        """Log execution parameters at test end to aid result triage."""
        try:
            meta = asdict(self.matter_test_config)
        except Exception as ex:
            LOGGER.warning("Unable to collect execution parameter summary: %s", ex)
            return

        config_fields: dict[str, Any] = {}
        for key, value in meta.items():
            if key == "global_test_params":
                continue
            if isinstance(value, os.PathLike):
                value = os.fspath(value)
            if value in (None, [], {}, ""):
                continue
            config_fields[key] = value

        named_args: dict[str, Any] = {}
        for key, value in self.matter_test_config.global_test_params.items():
            if key == "meta_config":
                continue
            if value in (None, [], {}, ""):
                continue
            named_args[key] = value

        LOGGER.info("===== EXECUTION FLAGS SUMMARY BEGIN =====")

        if config_fields:
            LOGGER.info("Config values:")
            for key in sorted(config_fields.keys()):
                LOGGER.info("  - %s: %s", key, self._format_summary_value(key, config_fields[key]))

        if named_args:
            LOGGER.info("\n\nNamed args:")
            for key in sorted(named_args.keys()):
                LOGGER.info("  - %s: %s", key, self._format_summary_value(key, named_args[key]))

        if self.is_pics_sdk_ci_only:
            test_name = self.__class__.__name__
            LOGGER.info("===== PICS_SDK_CI_ONLY is enabled (True) for test '%s'.", test_name)

        LOGGER.info("===== EXECUTION FLAGS SUMMARY END =====")

    def _dump_device_attributes_on_failure(self):
        """
        Dump device attribute data when problems are found for debugging purposes.

        This method attempts to generate a device attribute dump if the test has
        collected endpoint data.
        """
        try:
            # Check if we have endpoints_tlv data (from BasicCompositionTests or similar)
            if hasattr(self, 'endpoints_tlv') and self.endpoints_tlv:
                # Check if we have the dump_wildcard method (from BasicCompositionTests)
                if hasattr(self, 'dump_wildcard'):
                    _, txt_str = self.dump_wildcard(None)
                    # Only dump the text format - it's more readable for debugging
                    self.log_structured_data('==== FAILURE_DUMP_txt: ', txt_str)
        except (AttributeError, KeyError, ValueError, TypeError):
            # Don't let data access or serialization errors interfere with the original test failure
            pass

    def log_structured_data(self, start_tag: str, dump_string: str):
        """Log structured data with a clear start and end marker.

        This function is used to output device attribute dumps and other structured
        data to logs in a format that can be easily extracted for debugging.

        Args:
            start_tag: A prefix tag to identify the type of data being logged
            dump_string: The data to be logged
        """
        lines = dump_string.splitlines()
        LOGGER.info('%sBEGIN (%s lines)====', start_tag, len(lines))
        for line in lines:
            LOGGER.info('%s%s', start_tag, line)
        LOGGER.info('%sEND ====', start_tag)

    def setup_test(self):
        """Set up for each individual test execution.

        Resets test state, starts timers, notifies runner hooks, and establishes a background
        wildcard attribute subscription (excluding C/Q-quality attributes) so that tests have
        an up-to-date attribute cache available without extra round-trip reads.

        Called before each test method by the Mobly framework.

        Test authors may overwrite this method in the derived class to perform setup that is common for all tests.
        This is called once before each test_ in the class.

        Test authors that implement this method should ensure super().setup_test() is called before any custom setup.
        """
        self.current_step_index = 0
        self.test_start_time = datetime.now(UTC)
        self.step_start_time = datetime.now(UTC)
        self.step_skipped = False
        self.failed = False
        self._teardown_ran = False
        self._framework_cleanup_done = False
        self.cleanup_config = TestCleanupConfig()
        self._validate_test_parameters()
        # Capture the ACL before the test runs so _reset_acls_to_default can restore it
        # in teardown_class. Skip when the DUT is not known to be available: unit tests
        # never commission a device so _dut_confirmed_available stays False, and
        # commissioning_method is None, eliminating any network overhead for them.
        # For runner-commissioned tests commissioning_method is set; for in-test
        # commissioning the flag is set by commission_devices() on success.
        # is_commissioning is True for CommissionDeviceTest, where the DUT is not yet
        # on the fabric, an operational read there would send CASE Sigma1 to an
        # uncommissioned device, triggering unexpected DUT behaviour.
        dut_expected = (
            not self.is_commissioning
            and (
                self._dut_confirmed_available
                or self.matter_test_config.commissioning_method is not None
            )
        )
        if dut_expected:
            try:
                self._original_acl = self.event_loop.run_until_complete(
                    self.read_single_attribute_check_success(
                        cluster=Clusters.AccessControl,
                        attribute=Clusters.AccessControl.Attributes.Acl,
                        endpoint=0
                    )
                )
            except Exception:
                self._original_acl = None

        if self.runner_hook and not self.is_commissioning:
            # Start the background wildcard subscription only for tests that interact with a
            # real DUT (requires_dut = True, the default) and unless the test has opted out
            # via --no-wildcard-subscription or disable_wildcard_subscription = True on
            # the test class (e.g. tests that directly manipulate the ACL or tests that count
            # the TH entries).
            #
            # Parser/unit tests under test_testing/ override requires_dut = False so they
            # don't try to subscribe to a device that isn't there (which would hang ~30s per
            # test and blow the CI timeout).  Gating on requires_dut rather than on
            # --commissioning-method keeps this decoupled from harness CLI flags whose
            # semantics may shift at certification time.
            if not self._wildcard_subscription_disabled() and self.requires_dut:
                self._start_wildcard_subscription()
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

    def teardown_test(self):
        """Per-test teardown called by the Mobly framework after every test_ method.


        Shuts down the background wildcard subscription.

        Framework cleanup (DUT state restoration, extra controller shutdown) runs
        once at class end in teardown_class, not here. Override this method to add
        custom per-test teardown.

        Subclasses do not need to call super().teardown_test() — __init_subclass__
        wraps every override so this base method always runs after the override
        completes, regardless of whether super() was called.

        Idempotency: _teardown_ran prevents double-execution if super() was called
        explicitly from the override.
        """
        _config = getattr(self, 'matter_test_config', None)
        if _config is None or not self._wildcard_subscription_disabled():
            # Restore the ACL snapshot taken when starting the subscription controller so each test
            # runs with the same baseline ACL.
            if getattr(self, '_pre_subscription_acl', None) is not None:
                try:
                    self.event_loop.run_until_complete(
                        self.default_controller.WriteAttribute(
                            nodeId=self.dut_node_id,
                            attributes=[(0, Clusters.AccessControl.Attributes.Acl(list(self._pre_subscription_acl)))],
                        )
                    )
                except Exception as e:
                    LOGGER.warning("[MatterBaseTest] Error restoring ACL after wildcard subscription: %s", e)
                self._pre_subscription_acl = None
            if getattr(self, 'wildcard_subscription_handler', None) is not None:
                try:
                    self.wildcard_subscription_handler.shutdown()
                except Exception as e:
                    LOGGER.warning("[MatterBaseTest] Error shutting down wildcard subscription: %s", e)
                self.wildcard_subscription_handler = None
            LOGGER.info("Wildcard subscription shut down")

        if not self._teardown_ran:
            self._teardown_ran = True
            super().teardown_test()

    def on_fail(self, record):
        """Handle test failure callback from Mobly framework.

            This is called by the base framework.
            Tests should not call this directly.
            Tests should not overwrite this method.

        Args:
            record: TestResultRecord containing failure information.
        """
        self.failed = True
        if not self.is_commissioning:
            test_method = getattr(self, self.current_test_info.name, None)
            param_dump = format_declared_parameters_for_failure(
                test_method, self.user_params, self.matter_test_config
            )
            if param_dump:
                LOGGER.error(param_dump)
        if self.runner_hook and not self.is_commissioning:
            exception = record.termination_signal.exception

            try:
                step_duration = (datetime.now(UTC) - self.step_start_time) / timedelta(microseconds=1)
            except AttributeError:
                # If we failed during setup, these may not be populated
                step_duration = 0
            try:
                test_duration = (datetime.now(UTC) - self.test_start_time) / timedelta(microseconds=1)
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

                if isinstance(exception, (signals.TestError, signals.TestFailure)):
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
            LOGGER.error(textwrap.dedent(f"""

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
            step_duration = (datetime.now(UTC) - self.step_start_time) / timedelta(microseconds=1)
            test_duration = (datetime.now(UTC) - self.test_start_time) / timedelta(microseconds=1)
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
            test_duration = (datetime.now(UTC) - self.test_start_time) / timedelta(microseconds=1)
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
    def first_setup_code(self) -> str | None:
        return get_first_setup_code(self.default_controller, self.matter_test_config)

    @property
    def is_pics_sdk_ci_only(self) -> bool:
        """Checks if the 'PICS_SDK_CI_ONLY' PICS flag is enabled."""
        return self.check_pics('PICS_SDK_CI_ONLY')

    @property
    def default_endpoint(self) -> int:
        return 0

    #
    # Matter Test API - Parameter Getters
    #

    def pixit(self, name: str, default: Any = None) -> Any:
        """Get a declared PIXIT value by name.

        Retrieves the value from user_params. If not found, optional PIXITs may
        fall back to the default specified in the @pixit decorator; required
        PIXITs do not use decorator defaults (setup validation must supply them).
        Otherwise falls back to the ``default`` argument of this method.

        Args:
            name: The PIXIT parameter name (as declared in @pixit).
            default: Fallback default if no value is found and no decorator default exists.

        Returns:
            The PIXIT value, or the default.
        """
        value = self.user_params.get(name)
        if value is not None:
            return value

        test_name = self.current_test_info.name
        test_method = getattr(self, test_name, None)
        if test_method:
            for pixit_def in get_pixit_definitions(test_method):
                if pixit_def.name == name:
                    if (not pixit_def.required
                            and pixit_def.default is not _PIXIT_NO_DEFAULT):
                        return pixit_def.default
                    return default
        return default

    def harness_param(self, name: str) -> Any:
        """Return a declared harness parameter value from ``matter_test_config``.

        Returns the direct config value when available. For ``discriminator`` /
        ``passcode`` satisfied only via ``--qr-code`` or ``--manual-code``, returns
        ``None`` because the decoded value is not on ``MatterTestConfig``.

        Args:
            name: Logical name declared with ``@harness_params`` (registry key).

        Raises:
            ValueError: If ``name`` is not a registered harness parameter.
        """
        return resolve_harness_value(name, self.matter_test_config)

    def _validate_test_parameters(self):
        """Validate declared PIXITs and harness parameters before each test.

        Called from setup_test(). Fails with a combined message if any required
        PIXIT or harness parameter is missing.
        """
        test_name = self.current_test_info.name
        test_method = getattr(self, test_name, None)
        if test_method is None:
            return

        error_msg = format_missing_test_parameters(
            test_name, test_method, self.user_params, self.matter_test_config
        )
        if error_msg:
            asserts.fail(error_msg)

    def get_endpoint(self) -> int:
        """Gets the target endpoint ID from config, with a fallback default."""
        endpoint = self.matter_test_config.endpoint
        if endpoint is not None:
            return endpoint
        return self.default_endpoint

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

    def get_setup_payload_info(self) -> list[SetupPayloadInfo]:
        """
        Get and builds the payload info provided in the execution.
        Returns:
            List[SetupPayloadInfo]: List of Payload used by the test case
        """
        return get_setup_payload_info_config(self.matter_test_config)

    def get_random_port(self) -> int:
        """Selects a random port and checks that it is not yet in use.

        Note that this check can be racy, but the way this function is generally used
        is assumed fine.
        """
        if not hasattr(self, '_allocated_ports'):
            self._allocated_ports: set[int] = set()

        def is_port_available(p: int) -> bool:
            import errno

            # Verify TCP and UDP on all IPv4 interfaces
            for sock_type in (socket.SOCK_STREAM, socket.SOCK_DGRAM):
                with socket.socket(socket.AF_INET, sock_type) as s:
                    try:
                        s.bind(('', p))
                    except OSError as e:
                        if e.errno == errno.EADDRINUSE:
                            return False
                        raise

            # Verify TCP and UDP on all IPv6 interfaces if available
            if socket.has_ipv6:
                for sock_type in (socket.SOCK_STREAM, socket.SOCK_DGRAM):
                    with socket.socket(socket.AF_INET6, sock_type) as s:
                        try:
                            s.bind(('::', p))
                        except OSError as e:
                            if e.errno == errno.EADDRINUSE:
                                return False
                            raise
            return True

        while True:
            # The chosen safe range (35000-45000) naturally avoids well-known bad/conflicting
            # ports like 5353 (mDNS) and 5550-5555 (Matter standard ports).
            port = random.randint(35000, 45000)
            if port in self._allocated_ports:
                continue
            if is_port_available(port):
                self._allocated_ports.add(port)
                return port

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

    def get_defined_test_steps(self, test: str) -> list[TestStep] | None:
        """Retrieves test steps from a 'steps_*' function or AST extraction, using a cache.

        Checks for an explicit steps_* method first. If none exists, falls back to
        extracting steps from self.step() calls in the test method's source code.

        Returns None if no steps are defined by either mechanism.
        """
        if test in self.cached_steps:
            return self.cached_steps[test]

        steps = None
        if steps_method := getattr(self, 'steps_' + test.removeprefix('test_'), None):
            steps = steps_method()
        else:
            test_method = getattr(self, test)
            from matter.testing.step_extractor import extract_steps_from_method
            steps = extract_steps_from_method(test_method) or None

        self.cached_steps[test] = steps
        return steps

    def get_restart_flag_file(self) -> str | None:
        if self.matter_test_config.restart_flag_file is None:
            return None
        return str(self.matter_test_config.restart_flag_file)

    def get_test_pics(self, test: str) -> list[str]:
        ''' Retrieves a list of top-level PICS that should be checked before running this test

            An empty list means the test will always be run.

            PICS are defined in a function called pics_<functionname>.
            ex. for test test_TC_TEST_1_1, the pics are in a function called
            pics_TC_TEST_1_1.
        '''
        pics = self._get_defined_pics(test)
        return [] if pics is None else pics

    def _get_defined_pics(self, test: str) -> list[str] | None:
        """Retrieve PICS list from a 'pics_*' function or @pics decorator.

        The pics_* method takes precedence over the @pics decorator.

        Args:
            test: Name of the test to get PICS for.

        Returns:
            List of PICS strings if pics function or decorator exists, None otherwise.
        """
        if pics_method := getattr(self, 'pics_' + test.removeprefix('test_'), None):
            return pics_method()
        test_method = getattr(self, test)
        return getattr(test_method, '_pics', None)  # set by @pics

    def get_test_desc(self, test: str) -> str:
        ''' Returns a description of this test

            Test description is defined in the function called desc_<functionname>,
            or as a docstring on the test method itself.
            The desc_* method takes precedence if both are defined.

            Format:
            <Test plan reference> [<test plan number>] <test plan name>

            ex:
            133.1.1. [TC-ACL-1.1] Global attributes
        '''
        if desc_method := getattr(self, 'desc_' + test.removeprefix('test_'), None):
            return desc_method()
        test_method = getattr(self, test)
        if doc := test_method.__doc__:
            return doc.strip()
        return test

    #
    # Matter Test API - Step Management & Execution
    #
    # These methods are used to mark test progress for the test harness and logs, to help with test
    # debugging, issue creation and log analysis by the test labs.

    def step(self, step: int | str, description: str = "", *,
             is_commissioning: bool = False, expectation: str = ""):
        """Execute a test step and manage step progression.

        Validates step order, prints step information, and notifies runner hooks.

        Args:
            step: The step number or identifier to execute.
            description: Step description for inline step definitions.
                Should always be provided (not empty) when any keyword arguments
                (is_commissioning, expectation) are passed.
            is_commissioning: Mark this step as the commissioning step (keyword-only).
            expectation: Expected outcome for test plan generation (keyword-only).

            All arguments to step() must be constants for automatic extraction of
            the test step list to work. If dynamic step() parameters are required
            an explicit `steps_` method must be defined.

        Raises:
            AssertionError: If steps are called out of order or step doesn't exist.
        """
        # description, is_commissioning, and expectation are not used at runtime.
        # They exist so the AST step extractor can read them from source code to
        # build the step list without needing a separate steps_* method.
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
                step_duration = (datetime.now(UTC) - self.step_start_time) / timedelta(microseconds=1)
                self.runner_hook.step_success(logger=None, logs=None, duration=step_duration, request=None)

            # TODO: it seems like the step start should take a number and a name
            name = f'{step} : {current_step.description}'
            self.runner_hook.step_start(name=name)

        self.step_start_time = datetime.now(tz=UTC)
        self.current_step_index = self.current_step_index + 1
        self.step_skipped = False

    def print_step(self, stepnum: int | str, title: str) -> None:
        """Print test step information to logs.

        Args:
            stepnum: The step number or identifier.
            title: The descriptive title of the step.
        """
        LOGGER.info('***** Test Step %s : %s', stepnum, title)

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
        LOGGER.info('**** Skipping: %s', num)
        self.step_skipped = True

    def mark_all_remaining_steps_skipped(self, starting_step_number: int | str) -> None:
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

    def mark_step_range_skipped(self, starting_step_number: int | str, ending_step_number: int | str | None) -> None:
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
        if not hasattr(self, 'stored_global_wildcard') or self.stored_global_wildcard is None:
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
        dut_node_ids: list[int] = self.matter_test_config.dut_node_ids
        setup_payloads: list[SetupPayloadInfo] = self.get_setup_payload_info()
        commissioning_info: CommissioningInfo = CommissioningInfo(
            commissionee_ip_address_just_for_testing=self.matter_test_config.commissionee_ip_address_just_for_testing,
            commissioning_method=self.matter_test_config.commissioning_method,
            thread_operational_dataset=self.matter_test_config.thread_operational_dataset,
            wifi_passphrase=self.matter_test_config.wifi_passphrase,
            wifi_ssid=self.matter_test_config.wifi_ssid,
            tc_version_to_simulate=self.matter_test_config.tc_version_to_simulate,
            tc_user_response_to_simulate=self.matter_test_config.tc_user_response_to_simulate,
            thread_ba_host=self.matter_test_config.thread_ba_host,
            thread_ba_port=self.matter_test_config.thread_ba_port,
        )

        result = await commission_devices(dev_ctrl, dut_node_ids, setup_payloads, commissioning_info)
        if result:
            self._dut_confirmed_available = True
        return result

    async def commission_ntl_device(self, setup_payload: SetupPayload) -> bool:
        """Commission a single DUT devices over NTL.
        The discovery_cap_bitmask is patched to keep only the NTL bit ON.

        Uses the default controller to commission a device over NTL based on setup payload
        and commissioning configuration.

        Returns:
            True if commissioning succeeded, False otherwise.
        """
        dev_ctrl: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        LOGGER.info(
            "commission_ntl_device. Payload fields: passcode=%s discriminator=%s short_discriminator=%s vendor_id=%s product_id=%s discovery_cap_bitmask=%s commissioning_flow=%s",
            setup_payload.setup_passcode,
            setup_payload.long_discriminator,
            setup_payload.short_discriminator,
            setup_payload.vendor_id,
            setup_payload.product_id,
            format(setup_payload.rendezvous_information, '03b'),
            setup_payload.commissioning_flow,
        )

        # Ensure exactly one DUT node id is configured
        dut_node_ids: list[int] = self.matter_test_config.dut_node_ids
        LOGGER.info("Configured DUT node ids: %s", dut_node_ids)
        asserts.assert_equal(len(dut_node_ids), 1, "Expected exactly one DUT node id in matter_test_config.dut_node_ids")
        dut_node_id = dut_node_ids[0]

        # Retrieve the long_discriminator
        long_discriminator = setup_payload.long_discriminator
        asserts.assert_is_not_none(long_discriminator, "Expected setup payload to contain a long discriminator")
        long_discriminator = typing.cast(int, long_discriminator)

        # Create a new SetupPayload where only the NTL bit (0b10000) is kept in the discovery capabilities bitmask
        ntl_onboarding_data = SetupPayload().GenerateQrCode(
            passcode=setup_payload.setup_passcode,
            vendorId=setup_payload.vendor_id,
            productId=setup_payload.product_id,
            discriminator=long_discriminator,
            customFlow=setup_payload.commissioning_flow,
            capabilities=0b10000,
            version=setup_payload.version
        )

        # Create SetupPayloadInfo from Onboarding data
        ntl_setup_payload_info = SetupPayloadInfo()
        ntl_setup_payload_info.filter_type = discovery.FilterType.LONG_DISCRIMINATOR
        ntl_setup_payload_info.filter_value = long_discriminator
        ntl_setup_payload_info.passcode = setup_payload.setup_passcode
        ntl_setup_payload_info.setup_code = ntl_onboarding_data

        commissioning_info: CommissioningInfo = CommissioningInfo(
            commissionee_ip_address_just_for_testing=self.matter_test_config.commissionee_ip_address_just_for_testing,
            commissioning_method=self.matter_test_config.commissioning_method,
            thread_operational_dataset=self.matter_test_config.thread_operational_dataset,
            wifi_passphrase=self.matter_test_config.wifi_passphrase,
            wifi_ssid=self.matter_test_config.wifi_ssid,
            tc_version_to_simulate=self.matter_test_config.tc_version_to_simulate,
            tc_user_response_to_simulate=self.matter_test_config.tc_user_response_to_simulate,
            thread_ba_host=self.matter_test_config.thread_ba_host,
            thread_ba_port=self.matter_test_config.thread_ba_port,
        )

        pairing_status = await commission_device(dev_ctrl, dut_node_id, ntl_setup_payload_info, commissioning_info)
        result = bool(pairing_status)
        if result:
            self._dut_confirmed_available = True
        return result

    async def open_commissioning_window(self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController | None = None, node_id: int | None = None, timeout: int = 900) -> CustomCommissioningParameters:
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
            commissioning_params = await dev_ctrl.OpenCommissioningWindow(nodeId=node_id, timeout=timeout, iteration=1000,
                                                                          discriminator=rnd_discriminator, option=dev_ctrl.CommissioningWindowPasscode.kTokenWithRandomPin)
            return CustomCommissioningParameters(commissioning_params, rnd_discriminator)

        except InteractionModelError as e:
            asserts.fail(e.status, 'Failed to open commissioning window')
            raise  # Help mypy understand this never returns

    async def read_single_attribute(
            self, dev_ctrl: ChipDeviceCtrl.ChipDeviceController, node_id: int, endpoint: int, attribute: type[ClusterObjects.ClusterAttributeDescriptor], fabricFiltered: bool = True, verify_wildcard_subscription: bool | None = None) -> object:
        """Read a single attribute value from a device.

        Args:
            dev_ctrl: Device controller to use for the read operation.
            node_id: Node ID of the target device.
            endpoint: Endpoint ID where the attribute resides.
            attribute: The attribute to read.
            fabricFiltered: Whether to apply fabric filtering.
            verify_wildcard_subscription: Per-call override for the background
                wildcard-subscription verification.  None (default) defers to the
                class-level default_verify_wildcard_subscription; False skips
                the check for this read only.

        Returns:
            The attribute value.
        """
        result = await dev_ctrl.ReadAttribute(node_id, [(endpoint, attribute)], fabricFiltered=fabricFiltered)
        data = result[endpoint]
        attr_ret = list(data.values())[0][attribute]

        # Route this read through the same subscription-cache check used by
        # read_single_attribute_check_success so that any test calling this
        # lower-level helper still benefits from wildcard-subscription verification.
        # Downstream skip conditions (C/Q attributes, no active subscription,
        # cross-fabric reads, ACL-removed fallback) are evaluated inside
        # verify_attribute_subscription_value.
        read_ok = attr_ret is not None and not isinstance(attr_ret, Clusters.Attribute.ValueDecodeFailure)
        if read_ok and node_id == self.dut_node_id and self._effective_verify_wildcard_subscription(verify_wildcard_subscription):
            await self.verify_attribute_subscription_value(
                attribute=attribute,
                read_value=attr_ret,
                endpoint_id=endpoint,
                assert_on_error=True,
                dev_ctrl=dev_ctrl,
            )

        return attr_ret

    async def read_single_attribute_all_endpoints(
            self, cluster: ClusterObjects.Cluster, attribute: type[ClusterObjects.ClusterAttributeDescriptor],
            dev_ctrl: ChipDeviceCtrl.ChipDeviceController | None = None, node_id: int | None = None,
            verify_wildcard_subscription: bool | None = None):
        """Reads a single attribute of a specified cluster across all endpoints.

        Args:
            cluster: The cluster object the attribute belongs to.
            attribute: The attribute to read.
            dev_ctrl: Device controller to use for the read operation.  Defaults to
                self.default_controller.
            node_id: Node ID of the target device.  Defaults to self.dut_node_id.
            verify_wildcard_subscription: Per-call override for the background
                wildcard-subscription verification.  None (default) defers to the
                class-level default_verify_wildcard_subscription; False skips
                the check for this read only.

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

        # Verify each endpoint's value against the background wildcard-subscription
        # cache, mirroring the single-endpoint path in read_single_attribute_check_success.
        # Skip individual endpoints whose read failed (None / ValueDecodeFailure); the
        # remaining skip conditions are evaluated inside verify_attribute_subscription_value.
        if node_id == self.dut_node_id and self._effective_verify_wildcard_subscription(verify_wildcard_subscription):
            for endpoint, value in attrs.items():
                if value is None or isinstance(value, Clusters.Attribute.ValueDecodeFailure):
                    continue
                await self.verify_attribute_subscription_value(
                    attribute=attribute,
                    read_value=value,
                    endpoint_id=endpoint,
                    assert_on_error=True,
                    dev_ctrl=dev_ctrl,
                )

        return attrs

    async def read_single_attribute_check_success(
            self, cluster: ClusterObjects.Cluster, attribute: type[ClusterObjects.ClusterAttributeDescriptor],
            dev_ctrl: ChipDeviceCtrl.ChipDeviceController | None = None, node_id: int | None = None, endpoint: int | None = None, fabric_filtered: bool = True, assert_on_error: bool = True, test_name: str = "", payloadCapability: int = ChipDeviceCtrl.TransportPayloadCapability.MRP_PAYLOAD, verify_wildcard_subscription: bool | None = None) -> object:
        if dev_ctrl is None:
            dev_ctrl = self.default_controller
        if node_id is None:
            node_id = self.dut_node_id
        if endpoint is None:
            endpoint = self.get_endpoint()
        result = await dev_ctrl.ReadAttribute(node_id, [(endpoint, attribute)], fabricFiltered=fabric_filtered, payloadCapability=payloadCapability)
        attr_ret = result[endpoint][cluster][attribute]
        read_err_msg = f"Error reading {str(cluster)}:{str(attribute)} = {attr_ret}"
        desired_type = attribute.attribute_type.Type
        type_err_msg = f'Returned attribute {attribute} is wrong type expected {desired_type}, got {type(attr_ret)}'
        read_ok = attr_ret is not None and not isinstance(attr_ret, Clusters.Attribute.ValueDecodeFailure)
        type_ok = matchers.is_type(attr_ret, desired_type)
        if assert_on_error:
            asserts.assert_true(read_ok, read_err_msg)
            asserts.assert_true(type_ok, type_err_msg)
        else:
            location = AttributePathLocation(endpoint_id=endpoint, cluster_id=cluster.id,
                                             attribute_id=attribute.attribute_id)
            if not read_ok:
                self.record_error(test_name=test_name, location=location, problem=read_err_msg)
                return None
            if not type_ok:
                self.record_error(test_name=test_name, location=location, problem=type_err_msg)
                return None

        # Compare the read value against the background wildcard subscription cache.
        # Uses assert_on_error=True so a confirmed subscription mismatch fails the test,
        # surfacing DUT bugs where an attribute changes without a subscription report.
        #
        # Gated here so verification only runs for successful reads that target this DUT and
        # when the test (or class) has not opted out.  Further skip conditions (C/Q
        # attributes, no active subscription, cross-fabric reads, and the ACL-removed
        # fallback) are evaluated inside verify_attribute_subscription_value.
        #
        # Pass verify_wildcard_subscription=False (or set default_verify_wildcard_subscription
        # on the test class) to skip this check while still allowing a wildcard subscription
        # to run in the background.
        if read_ok and node_id == self.dut_node_id and self._effective_verify_wildcard_subscription(verify_wildcard_subscription):
            await self.verify_attribute_subscription_value(
                attribute=attribute,
                read_value=attr_ret,
                endpoint_id=endpoint,
                test_name=test_name,
                assert_on_error=True,
                dev_ctrl=dev_ctrl,
            )

        return attr_ret

    async def verify_attribute_subscription_value(
            self,
            attribute: type[ClusterObjects.ClusterAttributeDescriptor],
            read_value: Any,
            endpoint_id: int | None = None,
            test_name: str = "",
            assert_on_error: bool = True,
            dev_ctrl: ChipDeviceCtrl.ChipDeviceController | None = None) -> bool:
        """Compare a freshly-read attribute value against the background wildcard subscription cache.

        Called automatically from the base-class read helpers so any single-attribute read
        performed through the test harness is validated against the background subscription:

        - read_single_attribute_check_success()
        - read_single_attribute()
        - read_single_attribute_all_endpoints() (verifies each endpoint)

        Direct callers may also invoke this method after an ad-hoc read to confirm that
        the device is reporting the same value through its subscription as it returns on a
        direct read.  A mismatch means the subscription is either stale or not firing.

        The comparison is resolved through a series of checks, in order:

        1. **C/Q skip** — Attributes with Changes Omitted (C) or Quieter Reporting (Q) spec
           quality flags are skipped automatically; they are not required to reflect every
           change in subscription reports.
        2. **Cross-fabric skip** — When the reading controller (dev_ctrl) is on a different
           fabric than the subscription controller, validation is skipped because
           fabric-scoped attributes legitimately return different values per-fabric.
        3. **Exact match** — If the read value equals the cached value, the check passes.
        4. **Retry with delay** — On a mismatch, the method retries up to 3 times (1 s apart)
           to allow the subscription report to arrive.
        5. **Fabric-scoped filtering** — During each retry (and once more after retries are
           exhausted), _fabric_filtered_match() checks whether the mismatch is caused by
           the subscription cache containing entries from multiple fabrics while the read
           (which defaults to fabricFiltered=True) only returns the reading fabric's
           entries.  If the cache, filtered to the read value's fabric indices, matches the
           read value, the difference is purely cross-fabric data and not a DUT bug.
        6. **ACL conflict check** — If retries and fabric filtering both fail, the method
           reads the DUT's current ACL to check whether the subscription controller's entry
           was removed by the test.  If so, the mismatch is an ACL conflict (not a DUT bug)
           and is logged as a warning.
        7. **Fail** — If none of the above resolve the mismatch, it is reported as a test
           failure (assert_on_error=True) or recorded problem (assert_on_error=False).

        Args:
            attribute:      Attribute descriptor class (e.g. Clusters.OnOff.Attributes.OnOff).
            read_value:     Value returned by read_single_attribute_check_success().
            endpoint_id:    Endpoint to check; defaults to self.get_endpoint().
            test_name:      Included in recorded problems when assert_on_error is False.
            assert_on_error: If True, calls asserts.fail() on mismatch.
                             If False, records a problem via record_error() and returns False.
            dev_ctrl:       Controller used for the direct read.  When its fabric differs
                            from the subscription controller's fabric, validation is skipped.

        Returns:
            True if the values match or the check was skipped; False on mismatch.
        """
        if endpoint_id is None:
            endpoint_id = self.get_endpoint()

        cluster_id: int = attribute.cluster_id
        attr_id: int = attribute.attribute_id
        location = AttributePathLocation(endpoint_id=endpoint_id, cluster_id=cluster_id, attribute_id=attr_id)

        # C/Q-quality attributes are never stored in the subscription cache because they
        # are not required to report on every change.  Skip without error.
        if (cluster_id, attr_id) in self._cq_excluded_attr_ids:
            return True

        if self.wildcard_subscription_handler is None:
            return True

        if dev_ctrl is not None and self.subscription_controller is not None:
            try:
                if dev_ctrl.fabricId != self.subscription_controller.fabricId:
                    LOGGER.info(
                        "[verify_subscription] Skipping validation for %s on endpoint %s: "
                        "reading controller fabric %s differs from subscription controller fabric %s "
                        "(fabric-scoped attributes will legitimately differ)",
                        attribute.__name__, endpoint_id, dev_ctrl.fabricId,
                        self.subscription_controller.fabricId,
                    )
                    return True
            except AttributeError:
                pass

        cached_value = self.wildcard_subscription_handler.get_latest_value(endpoint_id, cluster_id, attr_id)

        if cached_value is None:
            problem = (
                f"Attribute {attribute.__name__} (cluster 0x{cluster_id:04X}, "
                f"attr 0x{attr_id:04X}) on endpoint {endpoint_id} "
                f"has no value in subscription cache — never reported via subscription"
            )
            if assert_on_error:
                asserts.fail(problem)
            else:
                self.record_error(test_name=test_name, location=location, problem=problem)
            return False

        if cached_value != read_value:
            LOGGER.info(
                "[verify_subscription] Mismatch on first check for %s on endpoint %s: "
                "read=%r, cache=%r. Retrying after delay...",
                attribute.__name__, endpoint_id, read_value, cached_value,
            )
            for attempt in range(3):
                await asyncio.sleep(1)
                cached_value = self.wildcard_subscription_handler.get_latest_value(endpoint_id, cluster_id, attr_id)
                if cached_value == read_value:
                    LOGGER.info(
                        "[verify_subscription] %s on endpoint %s matched after %ds retry: %s",
                        attribute.__name__, endpoint_id, attempt + 1, read_value,
                    )
                    return True
                if self._fabric_filtered_match(read_value, cached_value):
                    LOGGER.info(
                        "[verify_subscription] %s on endpoint %s matched after fabric-scoped filtering (retry %d)",
                        attribute.__name__, endpoint_id, attempt + 1,
                    )
                    return True
                LOGGER.info(
                    "[verify_subscription] Retry %d/3: still mismatched, cache=%r",
                    attempt + 1, cached_value,
                )

            if self._fabric_filtered_match(read_value, cached_value):
                LOGGER.info(
                    "[verify_subscription] %s on endpoint %s differs only by cross-fabric entries (not a DUT bug)",
                    attribute.__name__, endpoint_id,
                )
                return True

            if await self._is_subscription_acl_removed():
                LOGGER.warning(
                    "[verify_subscription] Subscription controller ACL entry removed by test, "
                    "skipping validation for %s on endpoint %s (read=%r, cache=%r)",
                    attribute.__name__, endpoint_id, read_value, cached_value,
                )
                return True

            problem = (
                f"Subscription cache mismatch for {attribute.__name__} "
                f"(cluster 0x{cluster_id:04X}, attr 0x{attr_id:04X}) "
                f"on endpoint {endpoint_id}: "
                f"read returned {read_value!r}, subscription cache has {cached_value!r} "
                f"(after 3s retry)"
            )
            if assert_on_error:
                asserts.fail(problem)
            else:
                self.record_error(test_name=test_name, location=location, problem=problem)
            return False

        LOGGER.info(
            "[verify_subscription] %s on endpoint %s matches read value: %s",
            attribute.__name__, endpoint_id, read_value,
        )
        return True

    @staticmethod
    def _fabric_filtered_match(read_value: Any, cached_value: Any) -> bool:
        """Check whether a read-vs-cache mismatch is caused by fabric-scoped filtering.

        The wildcard subscription uses fabricFiltered=False so its cache contains entries
        from all fabrics.  read_single_attribute_check_success defaults to
        fabricFiltered=True, so the direct read only returns entries for the reading
        controller's fabric.  For fabric-scoped list attributes (structs with a
        fabricIndex field) this means the cache legitimately has more entries than the
        read.

        Algorithm:
            1. Both values must be non-empty lists and the cache must be strictly longer.
            2. Every entry in the cached list must have a fabricIndex attribute
               (i.e. the attribute is fabric-scoped).
            3. Collect the set of fabricIndex values present in the read value.
            4. Filter the cached list to only entries whose fabricIndex is in that set.
            5. Compare the filtered cache against the read value.

        Returns:
            True if the read value matches the fabric-filtered cache, confirming the
            difference is purely cross-fabric data and not a missing subscription report.
            False if the values differ for any other reason, or if either value is not a
            list of fabric-scoped structs.
        """
        if not isinstance(read_value, list) or not isinstance(cached_value, list):
            return False
        if len(read_value) == 0 or len(cached_value) == 0:
            return False
        if len(cached_value) <= len(read_value):
            return False
        if not all(hasattr(entry, 'fabricIndex') for entry in cached_value):
            return False

        read_fabric_indices = {entry.fabricIndex for entry in read_value if hasattr(entry, 'fabricIndex')}
        if not read_fabric_indices:
            return False

        filtered_cache = [entry for entry in cached_value if entry.fabricIndex in read_fabric_indices]
        return filtered_cache == read_value

    async def _is_subscription_acl_removed(self) -> bool:
        """Check whether the subscription controller's ACL entry has been removed from the DUT.

        Returns True if the subscription controller no longer has an ACL entry,
        meaning subscription reports would be denied by the DUT's access control.
        """
        if self.subscription_controller is None:
            return False
        sub_node_id = self.matter_test_config.controller_node_id + 123456
        try:
            acl_result = await self.default_controller.ReadAttribute(
                nodeId=self.dut_node_id,
                attributes=[(0, Clusters.AccessControl.Attributes.Acl)],
            )
            current_acl = acl_result[0][Clusters.AccessControl][Clusters.AccessControl.Attributes.Acl]
            return all(not (entry.subjects and sub_node_id in entry.subjects) for entry in current_acl)
        except Exception as e:
            LOGGER.warning("[verify_subscription] Could not read ACL to check for conflict: %s", e)
            return False

    async def poll_until_attributes_in_range(
            self, cluster: ClusterObjects.Cluster,
            attribute_bounds: list[tuple[type[ClusterObjects.ClusterAttributeDescriptor], int, int]],
            timeout_sec: int = 1) -> None:
        """Poll attributes until each value falls within [min_value, max_value].

        Args:
            cluster: Cluster to read attributes from.
            attribute_bounds: List of (attribute, min_value, max_value) tuples.
            timeout_sec: Maximum time to wait for each attribute to be in range.

        Raises:
            TimeoutError: If any attribute does not reach its expected range before timeout.
        """
        for attribute, min_value, max_value in attribute_bounds:
            deadline = time.monotonic() + timeout_sec
            value = await self.read_single_attribute_check_success(cluster, attribute)
            while value < min_value or value > max_value:  # type: ignore[operator]
                if time.monotonic() >= deadline:
                    raise TimeoutError(
                        f"Timeout waiting for {attribute} to be in range [{min_value}, {max_value}], last value: {value}")
                await asyncio.sleep(0.1)
                value = await self.read_single_attribute_check_success(cluster, attribute)

    async def read_single_attribute_expect_error(
            self, cluster: ClusterObjects.Cluster, attribute: type[ClusterObjects.ClusterAttributeDescriptor],
            error: Status, dev_ctrl: ChipDeviceCtrl.ChipDeviceController | None = None, node_id: int | None = None, endpoint: int | None = None,
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

    async def write_single_attribute(self, attribute_value: ClusterObjects.ClusterAttributeDescriptor, endpoint_id: int | None = None, expect_success: bool = True) -> Status:
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

    def read_from_app_pipe(
        self,
        app_pipe_out: str | None = None,
        timeout: float = 2.0,
        max_bytes: int = 66536,
        chunk: int = 4096,
        ip_env_var: str | None = None,
    ) -> Any:
        """
        Read an out-of-band command from a Matter app.

        Args:
            app_pipe_out: Name of the cluster pipe file (e.g. /tmp/..._out). If None, uses value from config (--app-pipe-out).
            ip_env_var: Name of the environment variable containing the DUT IP. If not provided (or None), forces local FIFO read.

        Environment variables used when reading remotely:
            - <ip_env_var> (typically LINUX_DUT_IP): if set, the FIFO is read on the remote DUT via SSH.
            - LINUX_DUT_USER: required when <ip_env_var> is set.
        """
        if app_pipe_out is None:
            app_pipe_out = self.matter_test_config.pipe_name_out

        if not isinstance(app_pipe_out, str):
            raise TypeError("The named pipe must be provided as a string value")

        if not os.path.exists(app_pipe_out):
            LOGGER.error("Named pipe %r does NOT exist", app_pipe_out)
            raise FileNotFoundError("CANNOT FIND %r" % app_pipe_out)

        dut_ip: str | None = os.getenv(ip_env_var) if ip_env_var else None

        # If no DUT IP is provided, the Matter app is assumed to be local and the command
        # is read directly from the named pipe. If a DUT IP is present, the pipe is read
        # remotely via SSH from the target device.
        if dut_ip is None:
            # Use manual chunked reads instead of readline(): FIFO is opened non-blocking
            # and we need explicit timeout handling and a hard size limit for safety. We also
            # preserve any extra bytes (e.g. multiple queued messages) across calls.
            if not hasattr(self, "_app_pipe_out_buf"):
                self._app_pipe_out_buf = bytearray()

            fd = os.open(app_pipe_out, os.O_RDONLY | os.O_NONBLOCK)
            try:
                buf: bytearray = self._app_pipe_out_buf

                while True:
                    if b"\n" in buf:
                        line, _, rest = buf.partition(b"\n")
                        self._app_pipe_out_buf = bytearray(rest)

                        line = line.strip()
                        if not line:
                            continue
                        return json.loads(line.decode("utf-8"))

                    if buf:
                        try:
                            obj = json.loads(buf.decode("utf-8"))
                            self._app_pipe_out_buf = bytearray()
                            return obj
                        except json.JSONDecodeError:
                            pass

                    r, _, _ = select.select([fd], [], [], timeout)
                    if not r:
                        raise TimeoutError(f"No data within {timeout}")

                    chunk_bytes = os.read(fd, chunk)
                    if not chunk_bytes:
                        if buf:
                            try:
                                obj = json.loads(buf.decode("utf-8"))
                                self._app_pipe_out_buf = bytearray()
                                return obj
                            except json.JSONDecodeError as ex:
                                raise EOFError(f"Incomplete JSON response: {ex}") from ex
                        raise EOFError("Empty command response")

                    buf += chunk_bytes
                    if len(buf) > max_bytes:
                        raise ValueError("Command too large")
            finally:
                os.close(fd)

        LOGGER.info("Using DUT IP address: %s", dut_ip)

        dut_uname = os.getenv("LINUX_DUT_USER")
        asserts.assert_true(dut_uname is not None, "The LINUX_DUT_USER environment variable must be set")
        LOGGER.info("Using DUT user name: %s", dut_uname)

        # `cat` returns the remote FIFO contents. Parse as JSON for consistency with local behavior.
        out = subprocess.check_output(["ssh", f"{dut_uname}@{dut_ip}", "cat", app_pipe_out])
        out_str = out.decode("utf-8").strip()
        return json.loads(out_str)

    def write_to_app_pipe(self, command_dict: dict, app_pipe: str | None = None, ip_env_var: str | None = None):
        """
        Send an out-of-band command to a Matter app.
        Args:
            command_dict (dict): dictionary with the command and data.
            app_pipe (Optional[str], optional): Name of the cluster pipe file  (i.e. /tmp/chip_all_clusters_fifo_55441 or /tmp/chip_rvc_fifo_11111). Raises
            FileNotFoundError if pipe file is not found. If None takes the value from the CI argument --app-pipe,  arg --app-pipe has his own file exists check.
            ip_env_var: Optional[str]: is an optional argument. Name of the environment variable containing the DUT IP.

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
        if app_pipe is None:
            app_pipe = self.matter_test_config.pipe_name

        if not isinstance(app_pipe, str):
            raise TypeError("The named pipe must be provided as a string value")

        if not os.path.exists(app_pipe):
            LOGGER.error("Named pipe %r does NOT exist", app_pipe)
            raise FileNotFoundError("CANNOT FIND %r" % app_pipe)

        if not isinstance(command_dict, dict):
            raise TypeError("The command must be passed as a dictionary value")

        command = json.dumps(command_dict)

        dut_ip: str | None = os.getenv(ip_env_var) if ip_env_var else None

        # If no DUT IP is provided, the Matter app is assumed to be local and the command
        # is read directly from the named pipe. If a DUT IP is present, the pipe is read
        # remotely via SSH from the target device.
        if dut_ip is None:
            with open(app_pipe, "w") as app_pipe_fp:
                LOGGER.info("Sending out-of-band command: %s to file: %s", command, app_pipe)
                app_pipe_fp.write(json.dumps(command_dict) + "\n")
            # TODO(#31239): remove the need for sleep
            # This was tested with matter.js as being reliable enough
            time.sleep(0.05)
        else:
            LOGGER.info("Using DUT IP address: %s", dut_ip)

            dut_uname = os.getenv('LINUX_DUT_USER')
            asserts.assert_true(dut_uname is not None, "The LINUX_DUT_USER environment variable must be set")
            LOGGER.info("Using DUT user name: %s", dut_uname)
            command_fixed = shlex.quote(json.dumps(command_dict))
            cmd = "echo \"%s\" | ssh %s@%s \'cat > %s\'" % (command_fixed, dut_uname, dut_ip, app_pipe)
            os.system(cmd)

    async def send_single_cmd(
            self, cmd: Clusters.ClusterObjects.ClusterCommand,
            dev_ctrl: ChipDeviceCtrl.ChipDeviceController | None = None, node_id: int | None = None, endpoint: int | None = None,
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

        return await dev_ctrl.SendCommand(nodeId=node_id, endpoint=endpoint, payload=cmd, timedRequestTimeoutMs=timedRequestTimeoutMs,
                                          payloadCapability=payloadCapability)

    async def send_test_event_triggers(self, eventTrigger: int, enableKey: bytes | None = None):
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
                enableKey = bytes(list(range(16)))
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
            LOGGER.info("Legacy test event trigger activated")
        else:
            LOGGER.info("Legacy test event trigger deactivated")
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
                            default_value: str = "y") -> str | None:
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

        LOGGER.info("========= USER PROMPT for Endpoint %s =========", endpoint_id)
        LOGGER.info(">>> %s (press enter to confirm)", prompt_msg.rstrip())
        try:
            return input()
        except EOFError:
            LOGGER.info("========= EOF on STDIN =========")
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

            LOGGER.info("========= USER PROMPT for Image Validation =========")

            try:
                result = input()
                if result != '1':  # User did not select 'PASS'
                    raise TestError("Image validation failed")
            except EOFError:
                LOGGER.info("========= EOF on STDIN =========")
                return

    def _user_verify_prompt(self, prompt_msg: str, hook_method_name: str, validation_name: str, error_message: str) -> bool:
        """Helper to show a prompt and wait for user validation in TH."""
        # Only run when TC is being executed in TH
        if self.runner_hook and hasattr(self.runner_hook, hook_method_name):
            hook_method = getattr(self.runner_hook, hook_method_name)
            hook_method(msg=prompt_msg)

            LOGGER.info("========= USER PROMPT for %s =========", validation_name)

            try:
                result = input()
                if result != '1':  # User did not select 'PASS'
                    raise TestError(error_message)
            except EOFError:
                LOGGER.info("========= EOF on STDIN =========")
            return False
        return True  # Indicating skipped

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
        self._user_verify_prompt(
            prompt_msg=prompt_msg,
            hook_method_name='show_video_prompt',
            validation_name='Video Stream Validation',
            error_message='Video stream validation failed'
        )

    def user_verify_two_way_talk(self,
                                 prompt_msg: str) -> None:
        """Show Two Way Talk Verification Prompt and wait for user validation.
           This method will be executed only when TC is running in TH.

        Args:
            prompt_msg (str): Message for TH UI prompt and input function.
            Indicates what is expected from the user.

        Returns:
            Returns nothing indicating success so the test can go on.

        Raises:
            TestError: Indicating Two Way Talk validation step failed.
        """
        self._user_verify_prompt(
            prompt_msg=prompt_msg,
            hook_method_name='show_two_way_talk_prompt',
            validation_name='Two Way Talk Validation',
            error_message='Two way talk validation failed'
        )

    def user_verify_push_av_stream(self, prompt_msg: str) -> bool:
        """Show Push AV Stream Verification Prompt and wait for user validation.
           This method will be executed only when TC is running in TH.

        Args:
            prompt_msg (str): Message for TH UI prompt and input function.
            Indicates what is expected from the user.

        Returns:
            True if validation was skipped, False otherwise.

        Raises:
            TestError: Indicating Push AV Stream validation step failed.
        """
        return self._user_verify_prompt(
            prompt_msg=prompt_msg,
            hook_method_name='show_push_av_stream_prompt',
            validation_name='Push AV Stream Validation',
            error_message='Push AV Stream validation failed'
        )

    def _expire_sessions_on_all_controllers(self):
        """Helper method to expire sessions on all active controllers via the fabric admin interface.

        This method iterates through all certificate authorities and their fabric admins to expire
        sessions on all active controllers. This ensures all controllers can reconnect after a device
        reboot or factory reset.
        """
        LOGGER.info("Expiring sessions on all active controllers")
        for ca in self.matter_stack.certificate_authorities:
            for fabric_admin in ca.adminList:
                for controller in fabric_admin._activeControllers:
                    if controller.isActive:
                        try:
                            controller.ExpireSessions(self.dut_node_id)
                            LOGGER.info("Expired sessions on controller with nodeId %s", controller.nodeId)
                        except ChipStackError as e:  # chipstack-ok
                            LOGGER.warning("Failed to expire sessions on controller %s: %s", controller.nodeId, e)

    async def request_device_reboot(self):
        """Request a reboot of the Device Under Test (DUT).

        This method handles device reboots in both CI and development environments (via run_python_test.py test runner script)
        and also manual testing scenarios (via user input). It expires existing sessions to allow for controllers to reconnect
        to the DUT after the reboot.

        Returns:
            None
        """
        # Check if restart flag file is available (indicates test runner supports app restart)
        restart_flag_file = self.get_restart_flag_file()

        if not restart_flag_file:
            # No restart flag file: ask user to manually reboot
            self.wait_for_user_input(prompt_msg="Reboot the DUT. Press Enter when ready.\n")

            # After manual reboot, expire previous sessions so that we can re-establish connections
            self._expire_sessions_on_all_controllers()
            LOGGER.info("Manual device reboot completed")

        else:
            try:
                # Create the restart flag file to signal the test runner
                # Allow for multiple reboots like SW update tests do using the "restart" mode
                restart_text = "restart"
                with open(restart_flag_file, "w") as f:
                    f.write(restart_text)
                LOGGER.info("Created restart flag file to signal app reboot")

                # Expire sessions before the monitor picks up the flag
                self._expire_sessions_on_all_controllers()

                await self.wait_for_restart_flag_file_removal(restart_flag_file, restart_text)

                LOGGER.info("App reboot completed successfully")

            except Exception as e:
                LOGGER.error("Failed to reboot app: %s", e)
                asserts.fail(f"App reboot failed: {e}")

    async def request_device_factory_reset(self, reset_ctrl: bool = False) -> None:
        """Request a factory reset of the Device Under Test (DUT).

        This method handles factory resets in both CI and development environments and also manual
        testing scenarios (via user input). It expires existing sessions to allow for controllers
        to reconnect to the DUT after the factory reset.

        Args:
            reset_ctrl (bool): If True, removes app, REPL configs, and controller config.
                               If False, removes app and REPL configs but keeps controller config.
                               Defaults to False.

        Returns:
            None
        """
        # Check if restart flag file is available (indicates test runner supports app factory reset)
        restart_flag_file = self.get_restart_flag_file()

        if not restart_flag_file:
            # No restart flag file: ask user to manually factory reset
            self.wait_for_user_input(prompt_msg="Factory reset the DUT. Press Enter when ready.\n")

            # After manual factory reset, expire previous sessions so that we can re-establish connections
            self._expire_sessions_on_all_controllers()
            LOGGER.info("Manual device factory reset completed")

        else:
            restart_flag_text = "factory reset" if reset_ctrl else "factory reset app only"
            try:
                # Create the restart flag file to signal the test runner
                with open(restart_flag_file, "w") as f:
                    f.write(restart_flag_text)
                    LOGGER.info("Created restart flag file to signal %s request", restart_flag_text)

                # Expire sessions and re-establish connections
                self._expire_sessions_on_all_controllers()
                LOGGER.info("%s request sent successfully", restart_flag_text.capitalize())

                await self.wait_for_restart_flag_file_removal(restart_flag_file, restart_flag_text)

            except Exception as e:
                err = f"Failed to {restart_flag_text}: {e}"
                LOGGER.error(err)
                asserts.fail(err)

    async def wait_for_restart_flag_file_removal(self, restart_flag_file, restart_flag_text, timeout_sec=30.0):
        # Wait for the monitor thread to remove the flag file
        # The monitor deletes the flag file AFTER the restart completes, so this ensures
        # the app has fully rebooted and is ready before we continue
        start_time = time.time()
        while os.path.exists(restart_flag_file):
            if time.time() - start_time > timeout_sec:
                asserts.fail(f"App {restart_flag_text} did not complete within timeout (flag file still exists)")
            await asyncio.sleep(0.1)


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
    return [e for e in wildcard.attributes
            if accept_function(wildcard, e)]

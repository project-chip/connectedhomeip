#
#    Copyright (c) 2025 Project CHIP Authors
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

"""
Support module for CADMIN test modules containing shared functionality.
"""

import asyncio
import logging
import random
from dataclasses import dataclass
from datetime import datetime, timedelta
from enum import IntEnum
from typing import Optional

from mdns_discovery import mdns_discovery
from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.commissioning import CustomCommissioningParameters
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.matter_testing import AttributeMatcher, MatterBaseTest

log = logging.getLogger(__name__)


class CommissioningWindowOption(IntEnum):
    ORIGINAL_SETUP_CODE = 0  # kOriginalSetupCode: Original commissioning window (PASE)
    TOKEN_WITH_RANDOM_PIN = 1  # kTokenWithRandomPIN: Enhanced commissioning window (ECM)


class CADMINSupport:
    def __init__(self, test_instance):
        self.test = test_instance


class CADMINBaseTest(MatterBaseTest):
    async def get_fabrics(self, th: ChipDeviceCtrl, fabric_filtered: bool = True) -> int:
        """Get fabrics information from the device."""
        OC_cluster = Clusters.OperationalCredentials
        return await self.read_single_attribute_check_success(
            dev_ctrl=th,
            fabric_filtered=fabric_filtered,
            endpoint=0,
            cluster=OC_cluster,
            attribute=OC_cluster.Attributes.Fabrics
        )

    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        """Read the current fabric index from the device."""
        cluster = Clusters.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        return await self.read_single_attribute_check_success(
            dev_ctrl=th,
            endpoint=0,
            cluster=cluster,
            attribute=attribute
        )

    def calculate_clock_skew_factor(self, duration_seconds: int) -> int:
        """
        Calculate the clock skew factor for commissioning window monitoring.

        Args:
            duration_seconds: The duration of the commissioning window in seconds

        Returns:
            Clock skew factor in milliseconds (1% of duration or 100ms, whichever is greater)
        """
        skew_percentage = 1 / 100.0
        return max(int(duration_seconds * 1000 * skew_percentage), 100)

    async def create_window_status_subscription(
        self,
        th: ChipDeviceCtrl,
        node_id: int,
        min_interval_sec: int = 0,
        max_interval_sec: int = 30
    ) -> AttributeSubscriptionHandler:
        """
        Create a subscription to WindowStatus attribute using AttributeSubscriptionHandler.

        Args:
            th: Controller to use
            node_id: Target node ID
            min_interval_sec: Minimum reporting interval in seconds
            max_interval_sec: Maximum reporting interval in seconds

        Returns:
            AttributeSubscriptionHandler for WindowStatus
        """
        window_status_accumulator = AttributeSubscriptionHandler(
            Clusters.AdministratorCommissioning,
            Clusters.AdministratorCommissioning.Attributes.WindowStatus
        )

        await window_status_accumulator.start(
            th, node_id, 0, fabric_filtered=False,
            min_interval_sec=min_interval_sec,
            max_interval_sec=max_interval_sec
        )

        log.info("Created WindowStatus subscription for node %s", node_id)
        return window_status_accumulator

    async def wait_for_window_status_change(
        self,
        window_status_accumulator: AttributeSubscriptionHandler,
        is_open_expected: bool,
        timeout_sec: float = 10.0
    ) -> None:
        """
        Wait for window status to change to expected value, asserting on timeout.

        Args:
            window_status_accumulator: The subscription accumulator
            is_open_expected: Expected window status (False=closed, True=open)
            timeout_sec: Timeout in seconds

        Raises:
            AssertionError: If the window status doesn't change to expected value within timeout
        """
        status_name = "CLOSED" if not is_open_expected else "OPEN"
        log.info("Waiting for window status to change to %s (status=%s)", status_name, is_open_expected)
        log.info("Timeout set to: %ss", timeout_sec)

        status_match = AttributeMatcher.from_callable(
            f"WindowStatus is {is_open_expected}",
            lambda report: report.value == is_open_expected
        )

        try:
            window_status_accumulator.await_all_expected_report_matches([status_match], timeout_sec=timeout_sec)
            log.info("✅ Window status changed to %s (status=%s)", status_name, is_open_expected)
        except TimeoutError as e:
            error_msg = f"Timeout waiting for window status {is_open_expected} ({status_name}) after {timeout_sec}s: {e}"
            log.error("❌ %s", error_msg)
            asserts.fail(error_msg)

    def log_timing_results(self, results: 'CADMINBaseTest.TimingResults', test_step: str = ""):
        """
        Log timing results prominently for easy visibility in test output.

        Args:
            results: TimingResults dataclass from monitor_commissioning_window_closure_with_subscription
            test_step: Optional test step identifier
        """
        step_prefix = f"[{test_step}] " if test_step else ""

        log.info("%s=== COMMISSIONING WINDOW TIMING RESULTS ===", step_prefix)
        log.info("%sWindow closed: ✅ YES", step_prefix)
        log.info("%sTiming valid: %s", step_prefix, '✅ YES' if results.timing_valid else '❌ NO')

        if results.actual_duration_seconds is not None:
            actual = results.actual_duration_seconds
            expected = results.expected_duration_seconds
            max_allowed = results.max_allowed_duration_seconds
            skew_ms = results.clock_skew_ms

            log.info("%s⏱️  TIMING BREAKDOWN:", step_prefix)
            log.info("%s   Expected duration: %ss", step_prefix, expected)
            log.info("%s   Actual duration: %.2fs", step_prefix, actual)
            log.info("%s   Clock skew applied: %sms", step_prefix, skew_ms)
            log.info("%s   Maximum allowed: %.2fs", step_prefix, max_allowed)

            if actual <= expected:
                early_by = expected - actual
                log.info("%s   ✅ Window closed EARLY by %.2fs", step_prefix, early_by)
            elif actual <= max_allowed:
                late_by = actual - expected
                log.info("%s   ⚠️  Window closed LATE but within tolerance by %.2fs", step_prefix, late_by)
            else:
                over_by = actual - max_allowed
                log.error("%s   ❌ Window closed TOO LATE by %.2fs", step_prefix, over_by)

            log.info("%s   Start time: %s", step_prefix, results.start_time)
            log.info("%s   End time: %s", step_prefix, results.end_time)
            log.info("%s   Total monitoring time: %.2fs", step_prefix, actual)

        log.info("%s=== END TIMING RESULTS ===", step_prefix)

    async def monitor_commissioning_window_closure_with_subscription(
        self,
        th: ChipDeviceCtrl,
        node_id: int,
        expected_duration_seconds: int,
        min_interval_sec: int = 0,
        max_interval_sec: int = 30,
        window_status_accumulator: Optional[AttributeSubscriptionHandler] = None
    ) -> 'CADMINBaseTest.TimingResults':
        """
        Monitor commissioning window closure using subscription (replaces hardcoded sleep).

        Args:
            th: Controller to use
            node_id: Target node ID
            expected_duration_seconds: Expected duration of the window
            min_interval_sec: Minimum reporting interval for subscription
            max_interval_sec: Maximum reporting interval for subscription

        Returns:
            TimingResults dataclass with monitoring results
        """
        start_time = datetime.now()
        timeout_buffer_sec = 10
        clock_skew_ms = self.calculate_clock_skew_factor(expected_duration_seconds)
        max_allowed_duration = expected_duration_seconds + (clock_skew_ms / 1000)
        monitoring_timeout = max_allowed_duration + timeout_buffer_sec

        log.info("=== COMMISSIONING WINDOW MONITORING STARTED ===")
        log.info("Monitoring commissioning window closure for node %s", node_id)
        log.info("Expected duration: %ss", expected_duration_seconds)
        log.info("Clock skew factor: %sms", clock_skew_ms)
        log.info("Maximum allowed duration: %.2fs", max_allowed_duration)
        log.info("Monitoring started at: %s", start_time)
        log.info("Expected closure by: %s", start_time + timedelta(seconds=expected_duration_seconds))
        log.info("Latest acceptable closure: %s", start_time + timedelta(seconds=max_allowed_duration))

        try:
            # Wait for window to close (status = 0) - will assert on timeout
            await self.wait_for_window_status_change(
                window_status_accumulator=window_status_accumulator,
                is_open_expected=False,
                timeout_sec=monitoring_timeout
            )

            # Calculate actual duration - window definitely closed if we get here
            end_time = datetime.now()
            actual_duration = (end_time - start_time).total_seconds()

            # Verify timing
            timing_valid = actual_duration <= max_allowed_duration

            results = self.TimingResults(
                window_closed=True,  # Always true if we reach this point
                expected_duration_seconds=expected_duration_seconds,
                actual_duration_seconds=actual_duration,
                clock_skew_ms=clock_skew_ms,
                max_allowed_duration_seconds=max_allowed_duration,
                timing_valid=timing_valid,
                start_time=start_time,
                end_time=end_time
            )

            # Log results
            self.log_timing_results(results)

        finally:
            # Clean up subscription accumulator and return results
            if window_status_accumulator is not None:
                window_status_accumulator.cancel()
            return results

    async def open_commissioning_window_with_subscription_monitoring(
        self,
        th: ChipDeviceCtrl,
        timeout: int,
        node_id: int,
        discriminator: int = None,
        commissioning_option: CommissioningWindowOption = CommissioningWindowOption.TOKEN_WITH_RANDOM_PIN,
        iteration: int = 10000,
        min_interval_sec: int = 0,
        max_interval_sec: int = 30
    ) -> tuple[CustomCommissioningParameters, AttributeSubscriptionHandler]:
        """
        Open a commissioning window and create subscription for monitoring.

        Args:
            th: Controller to use
            timeout: Window timeout in seconds
            node_id: Target node ID
            discriminator: Optional discriminator value
            commissioning_option: Commissioning window option (CommissioningWindowOption, default: TOKEN_WITH_RANDOM_PIN)
                - ORIGINAL_SETUP_CODE (0): Original commissioning window (PASE)
                - TOKEN_WITH_RANDOM_PIN (1): Enhanced commissioning window (ECM)
            iteration: Number of iterations (default: 10000)
            min_interval_sec: Minimum reporting interval for subscription
            max_interval_sec: Maximum reporting interval for subscription

        Returns:
            Tuple of (CustomCommissioningParameters, AttributeSubscriptionHandler)
        """
        # Create subscription first
        window_status_accumulator = await self.create_window_status_subscription(
            th=th,
            node_id=node_id,
            min_interval_sec=min_interval_sec,
            max_interval_sec=max_interval_sec
        )

        try:
            comm_params = await th.OpenCommissioningWindow(
                nodeId=node_id,
                timeout=timeout,
                iteration=iteration,
                discriminator=discriminator if discriminator is not None else random.randint(0, 4095),
                option=commissioning_option.value
            )
            params = CustomCommissioningParameters(comm_params, discriminator)

        except Exception as e:
            log.exception('Error running OpenCommissioningWindow %s', e)
            asserts.fail('Failed to open commissioning window')

        return params, window_status_accumulator

    async def write_nl_attr(self, dut_node_id: int, th: ChipDeviceCtrl, attr_val: object):
        result = await th.WriteAttribute(nodeId=dut_node_id, attributes=[(0, attr_val)])
        asserts.assert_equal(result[0].Status, Status.Success, f"{th} node label write failed")

    async def read_nl_attr(self, dut_node_id: int, th: ChipDeviceCtrl, attr_val: object):
        try:
            await th.ReadAttribute(nodeId=dut_node_id, attributes=[(0, attr_val)])
        except Exception as e:
            asserts.assert_equal(e.err, "Received error message from read attribute attempt")
            self.print_step(0, e)

    async def get_window_status(self, th: ChipDeviceCtrl) -> int:
        """Get the current commissioning window status."""
        AC_cluster = Clusters.AdministratorCommissioning
        return await self.read_single_attribute_check_success(
            dev_ctrl=th,
            fabric_filtered=False,
            endpoint=0,
            cluster=AC_cluster,
            attribute=AC_cluster.Attributes.WindowStatus
        )

    def generate_unique_random_value(self, exclude_value: int) -> int:
        """Generate a random value that's different from the specified value."""
        while True:
            random_value = random.randint(10000000, 99999999)
            if random_value != exclude_value:
                return random_value

    async def revoke_commissioning(self, th: ChipDeviceCtrl, node_id: int):
        """Revoke the current commissioning window."""
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await th.SendCommand(
            nodeId=node_id,
            endpoint=0,
            payload=revokeCmd,
            timedRequestTimeoutMs=6000
        )
        # The failsafe cleanup is scheduled after the command completes
        await asyncio.sleep(1)

    @dataclass
    class TimingResults:
        """Results from commissioning window timing monitoring."""
        window_closed: bool
        expected_duration_seconds: int
        actual_duration_seconds: Optional[float]
        clock_skew_ms: int
        max_allowed_duration_seconds: float
        timing_valid: bool
        start_time: datetime
        end_time: Optional[datetime] = None

    @dataclass
    class ParsedService:
        service: mdns_discovery.MdnsServiceInfo
        cm: Optional[int] = None
        d: Optional[int] = None

        def __post_init__(self):
            # Safely convert CM value to int if present
            cm_value = self.service.txt.get('CM')
            if cm_value is not None:
                try:
                    self.cm = int(cm_value)
                except (ValueError, TypeError):
                    log.warning("Could not convert CM value '%s' to integer", cm_value)
                    self.cm = None

            # Safely convert D value to int if present
            d_value = self.service.txt.get('D')
            if d_value is not None:
                try:
                    self.d = int(d_value)
                except (ValueError, TypeError):
                    log.warning("Could not convert discriminator value '%s' to integer", d_value)
                    self.d = None

        def __str__(self) -> str:
            return f"Service CM={self.cm}, D={self.d}"

        def matches(self, expected_cm: int, expected_d: int) -> bool:
            """Check if this service matches the expected CM and discriminator values."""
            cm_match = self.cm == expected_cm
            d_match = self.d == expected_d
            return cm_match and d_match

    async def wait_for_correct_cm_value(
            self,
            expected_cm_value: int,
            expected_discriminator: int,
            max_attempts: int = 5,
            delay_sec: int = 5):
        """Wait for the correct CM value and discriminator in DNS-SD with retries."""
        for attempt in range(max_attempts):
            discovery = mdns_discovery.MdnsDiscovery()
            raw_services = await discovery.get_commissionable_services(discovery_timeout_sec=240, log_output=True)

            services = [self.ParsedService(service) for service in raw_services]

            # Look through all services for a match
            for parsed_service in services:
                if parsed_service.matches(expected_cm_value, expected_discriminator):
                    log.info("Found matching service: %s", parsed_service)
                    return parsed_service.service  # Return the original service object

            # Log what we found for debugging purposes
            log.info("Found %s services, but none match CM=%s, D=%s", len(services), expected_cm_value, expected_discriminator)
            for service in services:
                log.info("  %s", service)
            else:
                log.info("No services found in this attempt")

            # Not on last attempt, wait and retry
            if attempt < max_attempts - 1:
                log.info("Waiting for service with CM=%s and D=%s, attempt %s/%s",
                         expected_cm_value, expected_discriminator, attempt + 1, max_attempts)
                await asyncio.sleep(delay_sec)
            else:
                # Final retry attempt failed
                asserts.fail(f"Failed to find DNS-SD advertisement with CM={expected_cm_value} and "
                             f"discriminator={expected_discriminator} after {max_attempts} attempts. "
                             f"Found services: {[str(s) for s in services]}")
        return None

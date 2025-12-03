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

        logging.info(f"Created WindowStatus subscription for node {node_id}")
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
        logging.info(f"Waiting for window status to change to {status_name} (status={is_open_expected})")
        logging.info(f"Timeout set to: {timeout_sec}s")

        status_match = AttributeMatcher.from_callable(
            f"WindowStatus is {is_open_expected}",
            lambda report: report.value == is_open_expected
        )

        try:
            window_status_accumulator.await_all_expected_report_matches([status_match], timeout_sec=timeout_sec)
            logging.info(f"✅ Window status changed to {status_name} (status={is_open_expected})")
        except asyncio.TimeoutError as e:
            error_msg = f"Timeout waiting for window status {is_open_expected} ({status_name}) after {timeout_sec}s: {e}"
            logging.error(f"❌ {error_msg}")
            asserts.fail(error_msg)

    def log_timing_results(self, results: 'CADMINBaseTest.TimingResults', test_step: str = ""):
        """
        Log timing results prominently for easy visibility in test output.

        Args:
            results: TimingResults dataclass from monitor_commissioning_window_closure_with_subscription
            test_step: Optional test step identifier
        """
        step_prefix = f"[{test_step}] " if test_step else ""

        logging.info(f"{step_prefix}=== COMMISSIONING WINDOW TIMING RESULTS ===")
        logging.info(f"{step_prefix}Window closed: ✅ YES")
        logging.info(f"{step_prefix}Timing valid: {'✅ YES' if results.timing_valid else '❌ NO'}")

        if results.actual_duration_seconds is not None:
            actual = results.actual_duration_seconds
            expected = results.expected_duration_seconds
            max_allowed = results.max_allowed_duration_seconds
            skew_ms = results.clock_skew_ms

            logging.info(f"{step_prefix}⏱️  TIMING BREAKDOWN:")
            logging.info(f"{step_prefix}   Expected duration: {expected}s")
            logging.info(f"{step_prefix}   Actual duration: {actual:.2f}s")
            logging.info(f"{step_prefix}   Clock skew applied: {skew_ms}ms")
            logging.info(f"{step_prefix}   Maximum allowed: {max_allowed:.2f}s")

            if actual <= expected:
                early_by = expected - actual
                logging.info(f"{step_prefix}   ✅ Window closed EARLY by {early_by:.2f}s")
            elif actual <= max_allowed:
                late_by = actual - expected
                logging.info(f"{step_prefix}   ⚠️  Window closed LATE but within tolerance by {late_by:.2f}s")
            else:
                over_by = actual - max_allowed
                logging.error(f"{step_prefix}   ❌ Window closed TOO LATE by {over_by:.2f}s")

            logging.info(f"{step_prefix}   Start time: {results.start_time}")
            logging.info(f"{step_prefix}   End time: {results.end_time}")
            logging.info(f"{step_prefix}   Total monitoring time: {actual:.2f}s")

        logging.info(f"{step_prefix}=== END TIMING RESULTS ===")

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

        logging.info("=== COMMISSIONING WINDOW MONITORING STARTED ===")
        logging.info(f"Monitoring commissioning window closure for node {node_id}")
        logging.info(f"Expected duration: {expected_duration_seconds}s")
        logging.info(f"Clock skew factor: {clock_skew_ms}ms")
        logging.info(f"Maximum allowed duration: {max_allowed_duration:.2f}s")
        logging.info(f"Monitoring started at: {start_time}")
        logging.info(f"Expected closure by: {start_time + timedelta(seconds=expected_duration_seconds)}")
        logging.info(f"Latest acceptable closure: {start_time + timedelta(seconds=max_allowed_duration)}")

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
                await window_status_accumulator.cancel()
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
            logging.exception('Error running OpenCommissioningWindow %s', e)
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
                    logging.warning(f"Could not convert CM value '{cm_value}' to integer")
                    self.cm = None

            # Safely convert D value to int if present
            d_value = self.service.txt.get('D')
            if d_value is not None:
                try:
                    self.d = int(d_value)
                except (ValueError, TypeError):
                    logging.warning(f"Could not convert discriminator value '{d_value}' to integer")
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
                    logging.info(f"Found matching service: {parsed_service}")
                    return parsed_service.service  # Return the original service object

            # Log what we found for debugging purposes
            logging.info(f"Found {len(services)} services, but none match CM={expected_cm_value}, D={expected_discriminator}")
            for service in services:
                logging.info(f"  {service}")
            else:
                logging.info("No services found in this attempt")

            # Not on last attempt, wait and retry
            if attempt < max_attempts - 1:
                logging.info(f"Waiting for service with CM={expected_cm_value} and D={expected_discriminator}, "
                             f"attempt {attempt+1}/{max_attempts}")
                await asyncio.sleep(delay_sec)
            else:
                # Final retry attempt failed
                asserts.fail(f"Failed to find DNS-SD advertisement with CM={expected_cm_value} and "
                             f"discriminator={expected_discriminator} after {max_attempts} attempts. "
                             f"Found services: {[str(s) for s in services]}")
        return None

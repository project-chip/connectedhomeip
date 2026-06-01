#
#    Copyright (c) 2026 Project CHIP Authors
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
Support module for ICD test modules containing shared functionality.
"""

import asyncio
import contextlib
import logging
import random
import re
import time
from enum import IntEnum

from mdns_discovery.mdns_discovery import MdnsDiscovery, MdnsServiceType
from mdns_discovery.utils.asserts import assert_valid_icd_key
from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError
from matter.testing.matter_testing import MatterBaseTest

log = logging.getLogger(__name__)

cluster = Clusters.Objects.IcdManagement
attributes = cluster.Attributes
commands = cluster.Commands

# CI-specific constants
MAX_CI_IDLE_CYCLE_WAIT_S = 10

# Buffer for MaxInterval accounting for OS jitter, network latency, etc.
SUBSCRIPTION_REPORT_TIMING_TOLERANCE_S = 1.0

# Base for ICD test-event triggers: (IcdManagement cluster ID << 48).
_ICD_CLUSTER_CODE = Clusters.Objects.IcdManagement.id << 48


class ICDTestEventTriggerOperations(IntEnum):
    """
    Mirrors ICDTestEventTriggerEvent from ICDManager.cpp.
    Values are derived from the SDK cluster ID to stay in sync automatically.
    """
    kAddActiveModeReq = _ICD_CLUSTER_CODE | 0x0000_00000001
    kRemoveActiveModeReq = _ICD_CLUSTER_CODE | 0x0000_00000002
    kInvalidateHalfCounterValues = _ICD_CLUSTER_CODE | 0x0000_00000003
    kInvalidateAllCounterValues = _ICD_CLUSTER_CODE | 0x0000_00000004
    kForceMaximumCheckInBackOffState = _ICD_CLUSTER_CODE | 0x0000_00000005
    kDSLSForceSitMode = _ICD_CLUSTER_CODE | 0x0000_00000006
    kDSLSWithdrawSitMode = _ICD_CLUSTER_CODE | 0x0000_00000007


# *** UserActiveModeTriggerBitmap helpers ***
_cluster = Clusters.Objects.IcdManagement
uat = _cluster.Bitmaps.UserActiveModeTriggerBitmap

# BitMask for hints that are dependent on UserActiveModeTriggerInstruction
kUatInstructionDependentBitMask = (
    uat.kCustomInstruction | uat.kActuateSensorSeconds | uat.kActuateSensorTimes |
    uat.kActuateSensorLightsBlink | uat.kResetButtonLightsBlink | uat.kResetButtonSeconds |
    uat.kResetButtonTimes | uat.kSetupButtonSeconds | uat.kSetupButtonLightsBlink |
    uat.kSetupButtonTimes | uat.kAppDefinedButton)

# BitMask for hints that REQUIRE the presence of UserActiveModeTriggerInstruction
kUatInstructionMandatoryBitMask = (
    uat.kCustomInstruction | uat.kActuateSensorSeconds | uat.kActuateSensorTimes |
    uat.kResetButtonSeconds | uat.kResetButtonTimes | uat.kSetupButtonSeconds |
    uat.kSetupButtonTimes | uat.kAppDefinedButton)

# BitMask for hints where UserActiveModeTriggerInstruction is a uint (number)
kUatNumberInstructionBitMask = (
    uat.kActuateSensorSeconds | uat.kActuateSensorTimes |
    uat.kResetButtonSeconds | uat.kResetButtonTimes |
    uat.kSetupButtonSeconds | uat.kSetupButtonTimes)

# BitMask for hints where UserActiveModeTriggerInstruction is a color
kUatColorInstructionBitMask = (
    uat.kActuateSensorLightsBlink | uat.kResetButtonLightsBlink |
    uat.kSetupButtonLightsBlink)


def uat_bit_name(bit):
    """Derive a readable name from the SDK enum, e.g. 'kResetButtonLightsBlink' -> 'Reset Button Lights Blink'."""
    return re.sub(r'([a-z])([A-Z])', r'\1 \2', bit.name.removeprefix('k'))


def uat_set_hints(hint_bitmap):
    """Return a list of individual UAT bits set in the given bitmap, logging each one."""
    hints = [bit for bit in uat if hint_bitmap & bit.value]
    log.info(f"UserActiveModeTriggerHint has {len(hints)} bit(s) set:")
    for bit in hints:
        log.info(f"  - {uat_bit_name(bit)} (0x{bit.value:05X})")
    return hints


async def _wait_for_subscription_heartbeat(subscription, max_interval_ceiling_s: float,
                                           buffer_s: float) -> float | None:
    """Wait up to `max_interval_ceiling_s + buffer_s` for one empty heartbeat report.

    Returns the elapsed seconds if a report arrived, or None on timeout.
    """
    report_time: float | None = None
    wait_start = time.time()
    event = asyncio.Event()
    loop = asyncio.get_event_loop()

    def on_report():
        nonlocal report_time
        report_time = time.time()
        loop.call_soon_threadsafe(event.set)

    subscription.SetNotifySubscriptionStillActiveCallback(on_report)

    with contextlib.suppress(asyncio.TimeoutError):
        await asyncio.wait_for(event.wait(), timeout=max_interval_ceiling_s + buffer_s)

    return (report_time - wait_start) if report_time is not None else None


async def assert_subscription_heartbeat_received(subscription, max_interval_ceiling_s: float,
                                                 buffer_s: float = SUBSCRIPTION_REPORT_TIMING_TOLERANCE_S) -> float:
    """Assert that a heartbeat report arrives within `max_interval_ceiling_s + buffer_s` seconds."""
    elapsed = await _wait_for_subscription_heartbeat(subscription, max_interval_ceiling_s, buffer_s)
    asserts.assert_is_not_none(
        elapsed,
        f"No subscription heartbeat report received within {max_interval_ceiling_s + buffer_s:.1f}s"
    )
    log.info(f"Subscription heartbeat received in {elapsed:.1f}s (MaxInterval={max_interval_ceiling_s}s)")
    return elapsed


async def assert_subscription_no_heartbeat(subscription, max_interval_ceiling_s: float,
                                           buffer_s: float = SUBSCRIPTION_REPORT_TIMING_TOLERANCE_S) -> None:
    """Assert that no heartbeat report arrives within `max_interval_ceiling_s + buffer_s` seconds."""
    elapsed = await _wait_for_subscription_heartbeat(subscription, max_interval_ceiling_s, buffer_s)
    elapsed_str = f"{elapsed:.1f}s" if elapsed is not None else "unknown"
    asserts.assert_is_none(
        elapsed,
        f"Unexpected subscription heartbeat report received after {elapsed_str}"
    )


# ============================================================================
# ICDTransition - ICD state transition types for wait helpers
# ============================================================================


class ICDTransition(IntEnum):
    """ICD state transition types used by compute_wait_time and wait_for_transition."""
    ActiveToIdle = 1  # DUT transitions from Active to Idle (one ActiveModeDuration + 1s buffer)
    IdleToActive = 2  # DUT completes one full Idle cycle and transitions to Active (one IdleModeDuration + 1s buffer)
    FullCycle = 3     # ActiveToIdle followed by IdleToActive


# ============================================================================
# ICDBaseTest - Main Base Class
# ============================================================================


class ICDBaseTest(MatterBaseTest):
    """Base test class for ICD tests with shared functionality."""

    ROOT_NODE_ENDPOINT_ID = 0

    def get_dut_instance_name(self) -> str:
        compressed_fabric_id = self.default_controller.GetCompressedFabricId()
        return f'{compressed_fabric_id:016X}-{self.dut_node_id:016X}'

    async def get_icd_txt_key(self) -> int:
        """Retrieve the ICD DNS-SD TXT key from the DUT's operational service record."""
        mdns = MdnsDiscovery()

        dut_instance_name = self.get_dut_instance_name()
        instance_qname = f"{dut_instance_name}.{MdnsServiceType.OPERATIONAL.value}"

        txt_record = await mdns.get_txt_record(
            service_name=instance_qname,
            service_type=MdnsServiceType.OPERATIONAL.value,
            log_output=True
        )

        icd_value = txt_record.txt['ICD']
        assert_valid_icd_key(icd_value)
        return int(icd_value)

    async def read_icdm_attribute_expect_success(self, attribute, controller=None, node_id=None):
        return await self.read_single_attribute_check_success(
            endpoint=self.ROOT_NODE_ENDPOINT_ID,
            cluster=cluster,
            attribute=attribute,
            dev_ctrl=controller,
            node_id=node_id)

    async def send_single_icdm_command(self, command, controller=None, node_id=None):
        return await self.send_single_cmd(
            command,
            endpoint=self.ROOT_NODE_ENDPOINT_ID,
            dev_ctrl=controller,
            node_id=node_id)

    def compute_wait_time(self, transition: ICDTransition, *,
                          active_mode_duration_ms: int = None,
                          idle_mode_duration_s: int = None) -> float:
        """Return the number of seconds to wait for the given ICD state transition.

        ActiveToIdle: Requires active_mode_duration_ms; idle_mode_duration_s must not be provided.
        IdleToActive: Requires idle_mode_duration_s; active_mode_duration_ms must not be provided.
        FullCycle: Requires both.
        """
        if transition == ICDTransition.ActiveToIdle:
            if active_mode_duration_ms is None:
                raise ValueError("ActiveToIdle requires active_mode_duration_ms")
            if idle_mode_duration_s is not None:
                raise ValueError("ActiveToIdle does not use idle_mode_duration_s")
            wait_s = (active_mode_duration_ms / 1000.0) + 1.0
            log.info(f"ActiveToIdle: active_mode_duration_ms={active_mode_duration_ms} -> Wait time: {wait_s}s")
            return wait_s

        if transition == ICDTransition.IdleToActive:
            if idle_mode_duration_s is None:
                raise ValueError("IdleToActive requires idle_mode_duration_s")
            if active_mode_duration_ms is not None:
                raise ValueError("IdleToActive does not use active_mode_duration_ms")
            wait_s = idle_mode_duration_s + 1.0
            log.info(f"IdleToActive: idle_mode_duration_s={idle_mode_duration_s} -> Wait time: {wait_s}s")
            return wait_s

        if transition == ICDTransition.FullCycle:
            if active_mode_duration_ms is None:
                raise ValueError("FullCycle requires active_mode_duration_ms")
            if idle_mode_duration_s is None:
                raise ValueError("FullCycle requires idle_mode_duration_s")
            wait_s = (active_mode_duration_ms / 1000.0) + 1.0 + idle_mode_duration_s + 1.0
            log.info(
                f"FullCycle: active_mode_duration_ms={active_mode_duration_ms}, idle_mode_duration_s={idle_mode_duration_s} -> Wait time: {wait_s}s")
            return wait_s

        raise ValueError(f"Unknown ICDTransition: {transition}")

    async def wait_for_transition(self, transition: ICDTransition, *,
                                  active_mode_duration_ms: int = None,
                                  idle_mode_duration_s: int = None) -> None:
        """Sleep for the duration needed for the given ICD state transition."""
        wait_s = self.compute_wait_time(transition,
                                        active_mode_duration_ms=active_mode_duration_ms,
                                        idle_mode_duration_s=idle_mode_duration_s)
        log.info(f"Waiting {wait_s}s for {transition.name}...")
        await asyncio.sleep(wait_s)

    def create_new_controller(
            self,
            *,
            same_fabric: bool = False,
            node_id: int | None = None,
            enable_icd_registration: bool = False):
        """Create and return a new controller.

        Args:
            same_fabric: If True, reuse the existing CA and fabric admin.
                If False, create a new CertificateAuthority and FabricAdmin.
            node_id: If provided, assigned to the new controller.
                Otherwise, a random node ID is assigned automatically.
            enable_icd_registration: If True, enables the controller to
                register as an ICD client on an ICD server during commissioning.

        Returns:
            The new controller.
        """
        if node_id is None:
            node_id = random.randint(1, 0xFFFFFFEFFFFFFFFF)

        if same_fabric:
            fabric_admin = self.certificate_authority_manager.activeCaList[0].adminList[0]
        else:
            ca = self.certificate_authority_manager.NewCertificateAuthority()
            fabric_admin = ca.NewFabricAdmin(
                vendorId=0xFFF1,
                fabricId=random.randint(1, 0xFFFFFFFFFFFFFFFE)
            )

        controller = fabric_admin.NewController(nodeId=node_id, useTestCommissioner=True)

        if enable_icd_registration:
            icd_params = controller.GenerateICDRegistrationParameters()
            controller.EnableICDRegistration(icd_params)

        return controller

    async def unregister_all_clients(self):
        """Unregisters all entries in the DUT's RegisteredClients attribute."""
        registeredClients = await self.read_icdm_attribute_expect_success(attributes.RegisteredClients)

        if not registeredClients:
            log.info("RegisteredClients is empty.")
            return

        log.info("RegisteredClients is not empty; unregistering all clients...")
        for client in registeredClients:
            try:
                log.info(f"Unregistering client: {client}...")
                await self.send_single_icdm_command(commands.UnregisterClient(checkInNodeID=client.checkInNodeID))
            except InteractionModelError as e:
                asserts.assert_fail(f"Unexpected error returned when unregistering client: {e}")

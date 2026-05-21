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
import logging
import re
from enum import IntEnum

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

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
#

# Setup-payload field checks shared by TC_DD_1_1.py (QR code) and TC_DD_1_3.py (NFC tag),
# both of which validate the same onboarding payload fields, just read from a different
# transport. Ported from the onboarding_payload_test_suite in the retired
# project-chip/matter-test-scripts repository.

import logging

from mobly import asserts

import matter.discovery
from matter.setup_payload import SetupPayload

log = logging.getLogger(__name__)

# Discovery Capabilities Bitmask bits, per spec section 5.1.2.
_DISCOVERY_CAP_BLE = 1 << 1
_DISCOVERY_CAP_ON_NETWORK = 1 << 2
_DISCOVERY_CAP_WIFI_PAF = 1 << 3
_DISCOVERY_CAP_NFC = 1 << 4
_ALL_DISCOVERY_CAPS = _DISCOVERY_CAP_BLE | _DISCOVERY_CAP_ON_NETWORK | _DISCOVERY_CAP_WIFI_PAF | _DISCOVERY_CAP_NFC

# Manual pairing codes that a passcode SHALL NOT match, per spec section 5.1.7.
_DISALLOWED_PASSCODES = {
    0, 11111111, 22222222, 33333333, 44444444, 55555555,
    66666666, 77777777, 88888888, 99999999, 12345678, 87654321,
}


class OnboardingPayloadChecks:
    """Mixin with the setup-payload field checks shared by TC-DD-1.1 and TC-DD-1.3."""

    def check_payload_version(self, payload: SetupPayload) -> None:
        asserts.assert_equal(payload.version, 0, f"Invalid onboarding payload version: {payload.version}")

    def check_discovery_capabilities_bitmask(self, payload: SetupPayload) -> None:
        bitmask = payload.rendezvous_information
        asserts.assert_equal(bitmask & ~_ALL_DISCOVERY_CAPS, 0,
                             f"Discovery Capabilities Bitmask has reserved bits set: {bin(bitmask)}")
        asserts.assert_not_equal(bitmask & _ALL_DISCOVERY_CAPS, 0,
                                 "Discovery Capabilities Bitmask does not advertise any discovery method")

    def check_passcode_range(self, payload: SetupPayload) -> None:
        passcode = payload.setup_passcode
        asserts.assert_true(0x1 <= passcode <= 0x5F5E0FE, f"Passcode is not a 27-bit value: {hex(passcode)}")

    def check_passcode_validity(self, payload: SetupPayload) -> None:
        passcode = payload.setup_passcode
        asserts.assert_not_in(passcode, _DISALLOWED_PASSCODES, f"Passcode is a disallowed default value: {passcode}")

    def check_code_prefix(self, prefix: str) -> None:
        asserts.assert_equal(prefix, "MT:", f"Invalid onboarding code prefix: {prefix}")

    def check_commissioning_flow(self, payload: SetupPayload) -> None:
        asserts.assert_true(0x0 <= payload.commissioning_flow <= 0x2,
                            f"Invalid Commissioning Flow value: {payload.commissioning_flow}")

    def log_vendor_and_product_id_not_verified(self, payload: SetupPayload) -> None:
        # Cross-referencing VID/PID against the Distributed Compliance Ledger is not automated here.
        log.info("VID/PID from payload: VID=0x%04X, PID=0x%04X (not verified against the DCL)",
                 payload.vendor_id, payload.product_id)
        self.mark_current_step_skipped()

    async def check_advertised_discriminator(self, discriminator: int, long: bool) -> None:
        filter_type = matter.discovery.FilterType.LONG_DISCRIMINATOR if long else matter.discovery.FilterType.SHORT_DISCRIMINATOR
        responses = await self.default_controller.DiscoverCommissionableNodes(
            filterType=filter_type, filter=discriminator, stopOnFirst=True)
        asserts.assert_greater_equal(
            len(responses), 1,
            "No commissionable device found advertising the discriminator encoded in the onboarding payload")

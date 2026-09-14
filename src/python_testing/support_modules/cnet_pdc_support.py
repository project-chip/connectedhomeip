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

"""Shared helpers for the Network Commissioning Per-Device Credentials test cases.

TC-CNET-4.25 through TC-CNET-4.29 all drive the same handful of commands and make the same
assertions about PDC identities, so those live here rather than in each test case.
"""

import logging

from cryptography.exceptions import InvalidSignature
from mobly import asserts
from support_modules.network_identity import (COMPACT_IDENTITY_LENGTH, NETWORK_IDENTITY_IDENTIFIER_LENGTH,
                                              POSSESSION_SIGNATURE_LENGTH, validate_compact_identity, verify_possession_signature)

import matter.clusters as Clusters
from matter.clusters.Types import Nullable, NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterTestCommissionedDevice

log = logging.getLogger(__name__)

cnet = Clusters.NetworkCommissioning
cgen = Clusters.GeneralCommissioning

# Expiry for the fail-safe these test cases arm while they manipulate network configurations.
# They never invoke ConnectNetwork, so the armed window is a handful of round trips; a short
# expiry bounds how long a DUT stays modified if the TH dies before it can disarm.
FAILSAFE_EXPIRY_SECONDS = 60
MAX_DEBUG_TEXT_LENGTH = 512

# Stands in for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS where the TH has none; see
# CNETPDCBaseTest.non_empty_credentials.
_FALLBACK_CREDENTIALS = "placeholder-credentials"


class CNETPDCBaseTest(MatterTestCommissionedDevice):
    """Base class for the Wi-Fi Per-Device Credentials test cases of the Network Commissioning cluster."""

    # These test cases rewrite the Networks attribute repeatedly and read it back wherever they
    # care about the contents, so a background wildcard subscription reporting every one of those
    # changes is pure overhead.
    disable_wildcard_subscription = True

    def non_empty_credentials(self) -> bytes:
        """Returns a non-empty value for the Credentials field of AddOrUpdateWiFiNetwork.

        Prefers PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS (--wifi-passphrase) and falls back to
        a placeholder when the TH was not configured with any, which is the case when the DUT was
        commissioned on-network. The steps calling this only need the field to be non-empty, so
        the exact value does not matter.
        """
        return (self.get_credentials() or _FALLBACK_CREDENTIALS).encode()

    def assert_nullable_identifier(self, value: bytes | Nullable | None, context: str) -> bytes | None:
        """Asserts that a NetworkIdentifier / ClientIdentifier field is null or a 20 octet identifier.

        Returns the identifier, or None if the field is null. An absent field is a failure: the
        test plan requires every entry to report the field as either null or a 20 octet identifier.
        """
        asserts.assert_is_not_none(value, f"{context} is absent even though the DUT supports PDC.")
        if value is NullValue:
            return None
        asserts.assert_equal(len(value), NETWORK_IDENTITY_IDENTIFIER_LENGTH,
                             f"{context} must be null or an octstr of length {NETWORK_IDENTITY_IDENTIFIER_LENGTH}.")
        return value

    def assert_valid_identity(self, identity: bytes | None, context: str) -> None:
        asserts.assert_is_not_none(identity, f"{context} is missing.")
        asserts.assert_equal(len(identity), COMPACT_IDENTITY_LENGTH,
                             f"{context} must be a PDC identity in Compact Identity Format of length "
                             f"{COMPACT_IDENTITY_LENGTH}.")
        try:
            validate_compact_identity(identity)
        except (ValueError, InvalidSignature) as e:
            asserts.fail(f"{context} is not a valid self-signed PDC identity: {e}")

    def assert_valid_possession_signature(self, identity: bytes, nonce: bytes, signature: bytes | None,
                                          context: str) -> None:
        asserts.assert_is_not_none(signature, f"{context} is missing.")
        asserts.assert_equal(len(signature), POSSESSION_SIGNATURE_LENGTH,
                             f"{context} must be an octstr of length {POSSESSION_SIGNATURE_LENGTH}.")
        try:
            verify_possession_signature(identity, nonce, signature)
        except (ValueError, InvalidSignature) as e:
            asserts.fail(f"{context} is not a valid ec-signature over (ClientIdentity || PossessionNonce): {e}")

    def assert_network_config_success(self, response: object, context: str) -> None:
        asserts.assert_is_instance(response, cnet.Commands.NetworkConfigResponse,
                                   f"{context} did not return a NetworkConfigResponse.")
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"{context} did not report NetworkingStatus Success (DebugText: {response.debugText}).")

    def find_network(self, networks: list[cnet.Structs.NetworkInfoStruct],
                     network_id: bytes) -> cnet.Structs.NetworkInfoStruct:
        """Returns the single entry of the Networks attribute with the given NetworkID."""
        matches = [n for n in networks if n.networkID == network_id]
        asserts.assert_equal(len(matches), 1,
                             f"Expected exactly one entry with NetworkID {network_id!r} in the Networks attribute, "
                             f"found {len(matches)}.")
        return matches[0]

    def network_at(self, networks: list[cnet.Structs.NetworkInfoStruct], index: int) -> cnet.Structs.NetworkInfoStruct:
        """Returns the entry of the Networks attribute at the given index."""
        asserts.assert_greater(len(networks), index,
                               f"Networks has {len(networks)} entries, so it has no entry at index {index}.")
        return networks[index]

    async def expect_status(self, cmd: Clusters.ClusterObjects.ClusterCommand, endpoint: int,
                            expected_status: Status, context: str) -> None:
        try:
            response = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.fail(f"{context} should have failed with {expected_status.name}, but returned {response}.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, f"{context} returned the wrong status.")

    async def read_networks(self, endpoint: int) -> list[cnet.Structs.NetworkInfoStruct]:
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cnet, attribute=cnet.Attributes.Networks)

    async def query_identity(self, endpoint: int, key_identifier: bytes,
                             nonce: bytes | None = None) -> cnet.Commands.QueryIdentityResponse:
        response = await self.send_single_cmd(
            cmd=cnet.Commands.QueryIdentity(keyIdentifier=key_identifier, possessionNonce=nonce), endpoint=endpoint)
        asserts.assert_is_instance(response, cnet.Commands.QueryIdentityResponse,
                                   "QueryIdentity did not return a QueryIdentityResponse.")
        return response

    async def arm_failsafe(self, expiry_length_seconds: int) -> None:
        response = await self.send_single_cmd(
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=expiry_length_seconds), endpoint=0)
        asserts.assert_is_instance(response, cgen.Commands.ArmFailSafeResponse,
                                   "ArmFailSafe did not return an ArmFailSafeResponse.")
        asserts.assert_equal(response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"ArmFailSafe failed (DebugText: {response.debugText}).")

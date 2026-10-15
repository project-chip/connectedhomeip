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
assertions about PDC identities, so those live here rather than in each test case. The
access point TC-CNET-4.29 needs is here as well: it is a Wi-Fi fixture role class on one
side and the PDC encoders on the other, and belongs to neither.
"""

import asyncio
import hashlib
import logging
import time
from collections.abc import Mapping

from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import serialization
from mobly import asserts
from support_modules.network_identity import (COMPACT_IDENTITY_LENGTH, NETWORK_IDENTITY_IDENTIFIER_LENGTH,
                                              POSSESSION_SIGNATURE_LENGTH, encode_network_identity_certificate,
                                              generate_network_identity, network_identity_certificate_pem,
                                              network_identity_identifier, validate_compact_identity, verify_possession_signature)
from support_modules.wifi_fixture import AccessPointFixture, Radio, hostapd_file

import matter.clusters as Clusters
from matter.clusters.Types import Nullable, NullValue
from matter.exceptions import ChipStackError
from matter.interaction_model import InteractionModelError, Status
from matter.testing.matter_testing import MatterTestCommissionedDevice

log = logging.getLogger(__name__)

cnet = Clusters.NetworkCommissioning
cgen = Clusters.GeneralCommissioning

# Expiry for the fail-safe these test cases arm while they manipulate network configurations.
# A short expiry bounds how long a DUT stays modified if the TH dies before it can disarm, and
# it is also how long a test case that cannot reach the DUT has to wait for the timer instead.
FAILSAFE_EXPIRY_SECONDS = 60
MAX_DEBUG_TEXT_LENGTH = 512

# How long to wait for the response to a command that moves the DUT to another Wi-Fi network.
# The DUT answers over the network it is leaving and nothing routes between the two, so the
# response is expected to be lost rather than slow; this is how long it takes to establish that.
RESPONSE_TIMEOUT = 30

# Operational discovery on a network the DUT has moved to: mDNS and a CASE handshake, both over
# a link that has only just come up.
REACHABLE_TIMEOUT = 60
_REACHABLE_ATTEMPT_TIMEOUT = 20
_RETRY_DELAY = 5

# Stands in for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_CREDENTIALS where the TH has none; see
# CNETPDCBaseTest.non_empty_credentials.
_FALLBACK_CREDENTIALS = "placeholder-credentials"


class PDCAccessPointFixture(AccessPointFixture):
    """An access point that authenticates clients with Per-Device Credentials.

    The Network Identity is generated per instance, so the DUT sees a network it has never
    been configured for. The options a caller passes are the RSN half of the configuration,
    the part a test plan varies; the EAP and TLS half is fixed by the Per-Device Credentials
    requirements of the specification and is filled in here.

    No client is authorized until authorize is called, which is the state a test case needs
    to verify the DUT reporting a connection failure. Note that authorizing or de-authorizing
    a client reloads the configuration, and so deauthenticates every station.
    """

    _FIXTURE_OWNED_OPTIONS = AccessPointFixture._FIXTURE_OWNED_OPTIONS + (
        "ieee8021x", "eap_server", "eap_user_file", "eap_tls_cert_pinning", "server_cert", "private_key",
        "tls_flags", "openssl_ciphers", "openssl_ecdh_curves")

    def __init__(self, radio: Radio, ssid: str, options: Mapping[str, object]) -> None:
        # The caller's options go to the base class on their own, since that is where they
        # are checked against the options this class owns. Those are added afterwards.
        super().__init__(radio, {"ssid": ssid, **options})

        self._private_key, self._network_identity = generate_network_identity()
        self._certificate_path = hostapd_file(radio, "network.pem")
        self._private_key_path = hostapd_file(radio, "network.key")
        self._eap_user_path = hostapd_file(radio, "eap_user")
        self._authorized: set[bytes] = set()

        self.options.update({
            "ieee8021x": 1,
            "eap_server": 1,
            "eap_user_file": self._eap_user_path,
            "server_cert": self._certificate_path,
            "private_key": self._private_key_path,
            # Pin only: a client is authorized by the SHA-256 of the certificate it presents
            # rather than by a chain, which is what PDC calls for. No ca_cert is needed.
            "eap_tls_cert_pinning": 2,
            # The Common TLS Requirements of the Matter core specification: TLS 1.3 only, and
            # one cipher suite and one curve.
            "tls_flags": "[DISABLE-TLSv1.0][DISABLE-TLSv1.1][DISABLE-TLSv1.2][ENABLE-TLSv1.3]",
            "openssl_ciphers": "TLS_AES_128_CCM_SHA256",
            "openssl_ecdh_curves": "P-256",
        })

    @property
    def ssid(self) -> str:
        return str(self.options["ssid"])

    @property
    def network_identity(self) -> bytes:
        """The Network Identity of this access point, in Compact Identity Format."""
        return self._network_identity

    @property
    def network_identifier(self) -> bytes:
        """The 20 octet key identifier of the Network Identity of this access point."""
        return network_identity_identifier(self._network_identity)

    def start(self) -> None:
        self._certificate_path.write_text(network_identity_certificate_pem(self._network_identity))
        self._private_key_path.write_bytes(self._private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()))
        self._write_eap_user_file()
        super().start()

    def authorize(self, client_identity: bytes) -> None:
        """Makes a Network Client Identity an authorized client of this network."""
        log.info("Authorizing %s as a client of %r",
                 network_identity_identifier(client_identity).hex(), self.ssid)
        self._authorized.add(client_identity)
        self._apply_authorized_clients()

    def deauthorize(self, client_identity: bytes) -> None:
        """Withdraws the authorization of a Network Client Identity.

        An identity that was never authorized is not an error, so this is usable from the
        cleanup path of a test case that may not have got as far as authorizing one.
        """
        log.info("De-authorizing %s as a client of %r",
                 network_identity_identifier(client_identity).hex(), self.ssid)
        self._authorized.discard(client_identity)
        self._apply_authorized_clients()

    def _apply_authorized_clients(self) -> None:
        self._write_eap_user_file()
        # Before the access point is up there is nothing to reload, and start() reads the
        # file that has just been written anyway.
        if self._started:
            self.reload()

    def _write_eap_user_file(self) -> None:
        # An entry per authorized client, which hostapd matches against the SHA-256 of the
        # DER of the certificate the supplicant presented, and then a wildcard that selects
        # EAP-TLS for whatever identity a client offers in phase 1. The wildcard authorizes
        # nothing: with eap_tls_cert_pinning=2 only a cert-sha256- entry is looked up for
        # that, and a wildcard is explicitly not accepted in its place.
        lines = [f'"cert-sha256-{self._certificate_fingerprint(identity)}" TLS'
                 for identity in sorted(self._authorized)]
        lines.append("* TLS")
        self._eap_user_path.write_text("\n".join(lines) + "\n")

    @staticmethod
    def _certificate_fingerprint(compact_identity: bytes) -> str:
        return hashlib.sha256(encode_network_identity_certificate(compact_identity)).hexdigest()


class CNETPDCBaseTest(MatterTestCommissionedDevice):
    """Base class for the Wi-Fi Per-Device Credentials test cases of the Network Commissioning cluster."""

    # These test cases rewrite the Networks attribute repeatedly and read it back wherever they
    # care about the contents, so a background wildcard subscription reporting every one of those
    # changes is pure overhead.
    disable_wildcard_subscription = True

    # When the fail-safe armed last would expire on its own, so that disarm_failsafe knows how
    # long waiting the timer out takes when the DUT cannot be reached to disarm it.
    _failsafe_deadline: float = 0

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
        self._failsafe_deadline = time.monotonic() + expiry_length_seconds

    async def disarm_failsafe(self) -> None:
        """Forcibly disarms the fail-safe, and waits the timer out if the DUT cannot be reached.

        Disarming reverts the network configuration, which can take the DUT off the network the
        command was sent over before it has answered. The test plans allow the substitution
        wherever that is expected.
        """
        try:
            await asyncio.wait_for(self.arm_failsafe(0), timeout=RESPONSE_TIMEOUT)
        except (TimeoutError, ChipStackError) as error:
            log.info("No ArmFailSafeResponse arrived (%s)", error)
            await self.wait_out_failsafe()

    async def wait_out_failsafe(self) -> None:
        """Waits for the fail-safe armed last to expire on its own.

        What a test case is left with where it cannot reach the DUT to disarm the fail-safe,
        since the timer expiring reverts the same changes that disarming it would.
        """
        remaining = max(self._failsafe_deadline - time.monotonic(), 0)
        log.info("Waiting %.0f seconds for the fail-safe timer to expire instead", remaining)
        await asyncio.sleep(remaining)
        self._failsafe_deadline = 0

    async def commissioning_complete(self) -> None:
        response = await self.send_single_cmd(cmd=cgen.Commands.CommissioningComplete(), endpoint=0)
        asserts.assert_is_instance(response, cgen.Commands.CommissioningCompleteResponse,
                                   "CommissioningComplete did not return a CommissioningCompleteResponse.")
        asserts.assert_equal(response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"CommissioningComplete failed (DebugText: {response.debugText}).")
        self._failsafe_deadline = 0

    async def wait_until_reachable(self, timeout: float = REACHABLE_TIMEOUT) -> None:
        """Waits for the DUT to answer again after it has moved to another Wi-Fi network.

        The session the TH had runs over the network the DUT has left, and its address has
        changed with the move, so nothing short of a new CASE session on a freshly resolved
        address reaches it.
        """
        self.default_controller.ExpireSessions(self.dut_node_id)

        deadline = time.monotonic() + timeout
        while True:
            try:
                await self.default_controller.GetConnectedDevice(
                    nodeId=self.dut_node_id, allowPASE=False,
                    timeoutMs=int(_REACHABLE_ATTEMPT_TIMEOUT * 1000))
                return
            except (TimeoutError, ChipStackError) as error:
                if time.monotonic() > deadline:
                    asserts.fail(f"The TH could not reach the DUT within {timeout} seconds of it changing networks: "
                                 f"{error}")
                log.info("The DUT is not reachable yet (%s), retrying in %s seconds", error, _RETRY_DELAY)
                await asyncio.sleep(_RETRY_DELAY)

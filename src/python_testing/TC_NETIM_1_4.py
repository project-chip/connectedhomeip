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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${NETWORK_MANAGEMENT_APP}
#     factory-reset: true
#     quiet: true
#     app-args: >
#       --discriminator 1234 --KVS kvs1
#       --enable-key 000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging
import secrets
from dataclasses import dataclass

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipUtility
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.network_identity import (NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH, encode_network_administrator_secret,
                                             generate_network_client_identity, matter_epoch_now)
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# ImportAdminSecret and AddClient require a Timed Interaction.
_TIMED_REQUEST_TIMEOUT_MS = 5000
# Test event trigger base for "authenticate the client in the low 16 bits against the current NI".
# Matches NetworkIdentityManagementTrigger::kAuthenticateClientAgainstCurrentIdentity in the app.
_TRIGGER_AUTHENTICATE_CLIENT = 0x0450000000000000


# ImportAdminSecret requires a Timed Interaction; the controller enforces that client-side. This
# subclass clears the requirement so the command is sent untimed and the DUT is the party that
# rejects it with NeedsTimedInteraction.
@dataclass
class _ImportAdminSecretNoTimedInvoke(Clusters.NetworkIdentityManagement.Commands.ImportAdminSecret):
    @ChipUtility.classproperty
    def must_use_timed_invoke(cls) -> bool:
        return False


class TC_NETIM_1_4(MatterBaseTest):

    def desc_TC_NETIM_1_4(self) -> str:
        return "[TC-NETIM-1.4] ImportAdminSecret and ExportAdminSecret Command Verification [DUT-Server]"

    def steps_TC_NETIM_1_4(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done. TH generates the Network Administrator Shared Secrets used by "
                        "this test with strictly increasing timestamps.", is_commissioning=True),
            TestStep(2, "TH opens a commissioning window, establishes a PASE session, and sends ImportAdminSecret with "
                        "a valid NASS using a Timed Interaction over PASE.", "DUT responds with UNSUPPORTED_ACCESS."),
            TestStep(3, "TH confirms it can communicate with the DUT over CASE.", "DUT is reachable over CASE."),
            TestStep(4, "Over CASE, TH sends ExportAdminSecret before any secret has been imported.",
                        "DUT responds with NOT_FOUND."),
            TestStep(5, "Over CASE, TH sends ImportAdminSecret with a valid NASS without using a Timed Interaction.",
                        "DUT responds with NEEDS_TIMED_INTERACTION."),
            TestStep(6, "Over CASE, TH sends ImportAdminSecret with a malformed NASS using a Timed Interaction.",
                        "DUT responds with INVALID_COMMAND."),
            TestStep(7, "Over CASE, TH sends ImportAdminSecret with NASSa using a Timed Interaction, then adds a client "
                        "and authenticates it against the resulting Network Identity so it is not retired later.",
                        "DUT responds with SUCCESS; TimeStampA is stored."),
            TestStep(8, "Over CASE, TH sends ImportAdminSecret with NASSa again using a Timed Interaction.",
                        "DUT responds with SUCCESS (idempotent)."),
            TestStep(9, "Over CASE, TH sends ImportAdminSecret with a NASS whose timestamp is greater than TimeStampA, "
                        "then pins the resulting Network Identity with an authenticated client.",
                        "DUT responds with SUCCESS; TimeStampB is stored."),
            TestStep(10, "Over CASE, TH sends ImportAdminSecret with a NASS whose timestamp is less than TimeStampB "
                         "using a Timed Interaction.", "DUT responds with DYNAMIC_CONSTRAINT_ERROR."),
            TestStep(11, "Over CASE, TH sends ImportAdminSecret with a NASS whose timestamp is greater than TimeStampB, "
                         "then pins the resulting Network Identity with an authenticated client.",
                         "DUT responds with SUCCESS; TimeStampC is stored."),
            TestStep(12, "Over CASE, TH sends ImportAdminSecret with a NASS whose timestamp is greater than TimeStampC "
                         "using a Timed Interaction.", "DUT responds with SUCCESS; TimeStampD is stored."),
            TestStep(13, "Over CASE, TH sends ImportAdminSecret with a NASS whose timestamp is greater than TimeStampD "
                         "using a Timed Interaction.", "DUT responds with RESOURCE_EXHAUSTED."),
            TestStep(14, "Over CASE, TH sends ExportAdminSecret.",
                         "DUT responds with ExportAdminSecretResponse containing the most recently imported NASS."),
            TestStep(15, "TH reads ActiveNetworkIdentities.",
                         "Exactly one entry has Current=true with CreatedTimestamp equal to TimeStampD; the prior "
                         "identities are Current=false with valid RemainingClients values."),
        ]

    def pics_TC_NETIM_1_4(self) -> list[str]:
        return [
            "NETIM.S",
            "NETIM.S.C40.Rsp",
            "NETIM.S.C41.Rsp",
        ]

    async def _import_timed(self, nass: bytes, endpoint):
        """Sends ImportAdminSecret using a Timed Interaction over CASE; raises InteractionModelError on failure."""
        await self.send_single_cmd(
            cmd=Clusters.NetworkIdentityManagement.Commands.ImportAdminSecret(networkAdministratorSharedSecret=nass),
            endpoint=endpoint, timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)

    async def _import_expect_status(self, nass: bytes, endpoint, expected_status: Status, failure_message: str):
        """Sends ImportAdminSecret using a Timed Interaction and asserts it fails with the expected status."""
        try:
            await self._import_timed(nass, endpoint)
            asserts.fail(failure_message)
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, failure_message)

    async def _pin_current_network_identity(self, endpoint):
        """Adds a client and authenticates it against the current Network Identity so that identity is
        referenced by a client and will not be retired on subsequent imports."""
        _, client_identity = generate_network_client_identity()
        add_response = await self.send_single_cmd(
            cmd=Clusters.NetworkIdentityManagement.Commands.AddClient(clientIdentity=client_identity),
            endpoint=endpoint, timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)
        asserts.assert_is_instance(add_response, Clusters.NetworkIdentityManagement.Commands.AddClientResponse,
                                   "AddClient did not return an AddClientResponse.")
        await self.send_test_event_triggers(eventTrigger=_TRIGGER_AUTHENTICATE_CLIENT | add_response.clientIndex)

    @run_if_endpoint_matches(has_cluster(Clusters.NetworkIdentityManagement))
    async def test_TC_NETIM_1_4(self):
        cluster = Clusters.NetworkIdentityManagement
        commands = cluster.Commands
        attributes = cluster.Attributes
        endpoint = self.get_endpoint()

        self.step(1)
        # Timestamps are spaced a few seconds apart, strictly increasing, and close to "now" so the DUT's
        # (optional) future-timestamp check is satisfied. Each NASS uses a distinct random raw secret, so
        # each derives a distinct Network Identity; NASSa is reused verbatim for the idempotent re-import.
        base = matter_epoch_now()
        raw_a = secrets.token_bytes(NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH)
        timestamp_d = base + 30
        nass_a = encode_network_administrator_secret(created=base, raw_secret=raw_a)
        nass_b = encode_network_administrator_secret(created=base + 10, raw_secret=secrets.token_bytes(
            NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH))
        nass_c = encode_network_administrator_secret(created=base + 20, raw_secret=secrets.token_bytes(
            NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH))
        nass_d = encode_network_administrator_secret(created=timestamp_d, raw_secret=secrets.token_bytes(
            NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH))
        nass_e = encode_network_administrator_secret(created=base + 40, raw_secret=secrets.token_bytes(
            NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH))
        # A NASS whose timestamp is below TimeStampB (base + 10), used to exercise the monotonic check.
        nass_older_than_b = encode_network_administrator_secret(created=base + 5, raw_secret=secrets.token_bytes(
            NETWORK_ADMINISTRATOR_RAW_SECRET_LENGTH))
        # Not a valid NASS TLV; DecodeNetworkAdministratorSecret must reject it.
        malformed_nass = b"\xde\xad\xbe\xef"

        self.step(2)
        params = await self.open_commissioning_window()
        pase_node_id = self.dut_node_id + 1
        await self.default_controller.FindOrEstablishPASESession(
            setupCode=params.commissioningParameters.setupQRCode, nodeId=pase_node_id)
        try:
            await self.send_single_cmd(cmd=commands.ImportAdminSecret(networkAdministratorSharedSecret=nass_a),
                                       node_id=pase_node_id, endpoint=endpoint,
                                       timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)
            asserts.fail("ImportAdminSecret over PASE should fail with UnsupportedAccess.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.UnsupportedAccess,
                                 "ImportAdminSecret over PASE should fail with UnsupportedAccess.")

        self.step(3)
        # A successful CASE read confirms communication over CASE.
        await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.ClientTableSize)

        self.step(4)
        try:
            await self.send_single_cmd(cmd=commands.ExportAdminSecret(), endpoint=endpoint,
                                       timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)
            asserts.fail("ExportAdminSecret before any import should fail with NotFound.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound, "ExportAdminSecret before any import should fail with NotFound.")

        self.step(5)
        try:
            await self.default_controller.SendCommand(
                self.dut_node_id, endpoint, _ImportAdminSecretNoTimedInvoke(networkAdministratorSharedSecret=nass_a))
            asserts.fail("ImportAdminSecret without a Timed Interaction should fail with NeedsTimedInteraction.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NeedsTimedInteraction,
                                 "ImportAdminSecret without a Timed Interaction should fail with NeedsTimedInteraction.")

        self.step(6)
        await self._import_expect_status(malformed_nass, endpoint, Status.InvalidCommand,
                                         "ImportAdminSecret with a malformed NASS should fail with InvalidCommand.")

        self.step(7)
        await self._import_timed(nass_a, endpoint)
        await self._pin_current_network_identity(endpoint)

        self.step(8)
        await self._import_timed(nass_a, endpoint)

        self.step(9)
        await self._import_timed(nass_b, endpoint)
        await self._pin_current_network_identity(endpoint)

        self.step(10)
        await self._import_expect_status(nass_older_than_b, endpoint, Status.DynamicConstraintError,
                                         "ImportAdminSecret with a non-increasing timestamp should fail with "
                                         "DynamicConstraintError.")

        self.step(11)
        await self._import_timed(nass_c, endpoint)
        await self._pin_current_network_identity(endpoint)

        self.step(12)
        await self._import_timed(nass_d, endpoint)

        self.step(13)
        await self._import_expect_status(nass_e, endpoint, Status.ResourceExhausted,
                                         "ImportAdminSecret beyond the Network Identity capacity should fail with "
                                         "ResourceExhausted.")

        self.step(14)
        export_response = await self.send_single_cmd(cmd=commands.ExportAdminSecret(), endpoint=endpoint,
                                                     timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)
        asserts.assert_is_instance(export_response, commands.ExportAdminSecretResponse,
                                   "ExportAdminSecret did not return an ExportAdminSecretResponse.")
        asserts.assert_equal(export_response.networkAdministratorSharedSecret, nass_d,
                             "ExportAdminSecret did not return the most recently imported NASS (NASSd).")

        self.step(15)
        # Four Network Identities remain: NASSa/b/c (non-current, each pinned by one authenticated client)
        # and NASSd (current). The failed import in step 13 left the table unchanged.
        active_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attributes.ActiveNetworkIdentities)
        asserts.assert_equal(len(active_list), 4, "Expected four active network identities (NASSa..NASSd).")
        current_entries = [identity for identity in active_list if identity.current]
        asserts.assert_equal(len(current_entries), 1, "Exactly one active network identity must be current.")
        asserts.assert_equal(current_entries[0].createdTimestamp, timestamp_d,
                             "The current network identity's CreatedTimestamp must match the last successful import (NASSd).")
        asserts.assert_equal(current_entries[0].remainingClients, NullValue,
                             "RemainingClients must be null for the current network identity.")
        non_current = [identity for identity in active_list if not identity.current]
        asserts.assert_equal(len(non_current), 3, "Expected three prior (non-current) network identities.")
        for identity in non_current:
            asserts.assert_not_equal(identity.remainingClients, NullValue,
                                     "A non-current network identity must report a non-null RemainingClients.")
            asserts.assert_equal(identity.remainingClients, 1,
                                 "Each prior network identity was pinned by exactly one authenticated client.")


if __name__ == "__main__":
    default_matter_test_main()

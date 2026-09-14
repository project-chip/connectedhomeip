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

# The DUT is a Matter commissioner that originates the commands under test, so this test
# is driven by an operator working the DUT's own interface and is not run in CI.
#
# Run it with the path to a Network Identity Management server app for the TH, e.g.:
#
#   python3 src/python_testing/TC_NETIM_1_6.py \
#       --storage-path admin_storage.json \
#       --PICS <the DUT's PICS file> \
#       --string-arg th_server_app_path:out/linux-x64-network-manager/matter-network-manager-app
#
# Every command a client can generate is optional, so the test plan gates steps 2-4 on
# ImportAdminSecret and step 5 on ExportAdminSecret. The PICS file decides which of those
# groups run; without --PICS both read as unsupported and the test skips.

import logging

from mobly import asserts
from support_modules.network_identity import (MATTER_EPOCH_OFFSET_SECONDS, decode_network_administrator_secret,
                                              encode_network_administrator_secret, matter_epoch_now)
from support_modules.network_identity_th_server import NetworkIdentityTHServerTest

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import async_test_body, pics
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# The per-step PICS from the test plan's procedure table. Both are optional for a client, so
# each group of steps runs only if the DUT claims the command that group exercises.
PICS_IMPORT_ADMIN_SECRET = 'NETIM.C.C40.Tx'
PICS_EXPORT_ADMIN_SECRET = 'NETIM.C.C41.Tx'


class TC_NETIM_1_6(NetworkIdentityTHServerTest):

    async def _export_stored_secret(self) -> bytes:
        """Returns the NASS currently stored on the TH server."""
        commands = Clusters.NetworkIdentityManagement.Commands
        response = await self.send_th_server_command(commands.ExportAdminSecret())
        asserts.assert_is_instance(response, commands.ExportAdminSecretResponse,
                                   "ExportAdminSecret did not return an ExportAdminSecretResponse.")
        return response.networkAdministratorSharedSecret

    async def _import_secret_from_harness(self) -> bytes:
        """Imports a valid NASS into the TH server over the harness fabric and returns it."""
        commands = Clusters.NetworkIdentityManagement.Commands
        nass = encode_network_administrator_secret(created=matter_epoch_now())
        await self.send_th_server_command(commands.ImportAdminSecret(networkAdministratorSharedSecret=nass))
        return nass

    @pics('NETIM.C')
    @async_test_body
    async def test_TC_NETIM_1_6(self):
        """[TC-NETIM-1.6] Client Admin Secret Command Access and Response Behavior [DUT-Client]"""
        cluster = Clusters.NetworkIdentityManagement
        commands = cluster.Commands
        attributes = cluster.Attributes
        self.require_any_pics(PICS_IMPORT_ADMIN_SECRET, PICS_EXPORT_ADMIN_SECRET)

        self.step("precondition", "TH confirms its Network Identity Management server starts with no stored Network "
                  "Administrator Shared Secret.",
                  expectation="ActiveNetworkIdentities is empty and ExportAdminSecret fails with NOT_FOUND.")
        asserts.assert_equal(await self.read_th_server_attribute(attributes.ActiveNetworkIdentities), [],
                             "The TH server should hold no active network identities before the test begins.")
        try:
            await self.send_th_server_command(commands.ExportAdminSecret())
            asserts.fail("The TH server returned a NASS before any secret had been imported.")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.NotFound,
                                 "ExportAdminSecret before any import should fail with NotFound.")

        self.step(1, "Commission TH to DUT.",
                  expectation="DUT and TH can communicate over CASE.")
        await self.ask_dut_to_commission_th_server()

        # NASSa and TimeStampA are established by the import steps, or seeded by the harness in
        # step 5 when those steps did not run.
        nass_a = None
        timestamp_a = None

        self.step(2, "DUT sends ImportAdminSecret over CASE with malformed NASS.",
                  expectation="TH responds INVALID_COMMAND.")
        if self.pics_guard(self.check_pics(PICS_IMPORT_ADMIN_SECRET)):
            self.ask_dut_to_send(
                "Using the DUT's interface, send an ImportAdminSecret command over CASE to the TH Network Identity "
                "Management server carrying a malformed Network Administrator Shared Secret.",
                "the TH responded with INVALID_COMMAND.",
                expected_log=self.expect_command_status(commands.ImportAdminSecret.command_id, Status.InvalidCommand))
            asserts.assert_equal(await self.read_th_server_attribute(attributes.ActiveNetworkIdentities), [],
                                 "A malformed NASS must not be stored by the TH server.")

        self.step(3, "DUT sends ImportAdminSecret over CASE with valid NASS.",
                  expectation="TH responds SUCCESS. The timestamp is stored as TimeStampA.")
        if self.pics_guard(self.check_pics(PICS_IMPORT_ADMIN_SECRET)):
            self.ask_dut_to_send(
                "Using the DUT's interface, send an ImportAdminSecret command over CASE to the TH Network Identity "
                "Management server carrying a valid Network Administrator Shared Secret.",
                "the TH responded with SUCCESS.",
                expected_log=self.expect_admin_secret_transfer("imported"))
            active_identities = await self.read_th_server_attribute(attributes.ActiveNetworkIdentities)
            current = [identity for identity in active_identities if identity.current]
            asserts.assert_equal(len(current), 1,
                                 "A successful ImportAdminSecret should leave exactly one current active network identity.")
            # The harness cannot see the bytes the DUT sent, so it reads back what the server stored.
            # That is NASSa: the secret the successful import installed, carried forward as the value
            # step 4 must not displace and step 5 must return, and the source of TimeStampA.
            nass_a = await self._export_stored_secret()
            timestamp_a = decode_network_administrator_secret(nass_a).created
            asserts.assert_equal(current[0].createdTimestamp, timestamp_a,
                                 "CreatedTimestamp does not match the timestamp in the stored NASS.")
            log.info("TimeStampA is %d (Matter epoch seconds)", timestamp_a)

        self.step(4, "DUT sends ImportAdminSecret over CASE with a NASS that has a timestamp less than TimeStampA.",
                  expectation="TH responds DYNAMIC_CONSTRAINT_ERROR.")
        if self.pics_guard(self.check_pics(PICS_IMPORT_ADMIN_SECRET)):
            self.ask_dut_to_send(
                "Using the DUT's interface, send an ImportAdminSecret command over CASE to the TH Network Identity "
                f"Management server carrying a valid NASS whose created timestamp is less than TimeStampA={timestamp_a} "
                f"(Matter epoch seconds, i.e. {timestamp_a + MATTER_EPOCH_OFFSET_SECONDS} in Unix time).",
                "the TH responded with DYNAMIC_CONSTRAINT_ERROR.",
                expected_log=self.expect_command_status(commands.ImportAdminSecret.command_id,
                                                        Status.DynamicConstraintError))
            asserts.assert_equal(await self._export_stored_secret(), nass_a,
                                 "A NASS with a timestamp less than TimeStampA must not replace the stored secret.")

        self.step(5, "DUT sends ExportAdminSecret over CASE after the successful import.",
                  expectation="TH responds with ExportAdminSecretResponse containing the most recently imported NASS.")
        if self.pics_guard(self.check_pics(PICS_EXPORT_ADMIN_SECRET)):
            if nass_a is None:
                # A DUT that generates ExportAdminSecret but not ImportAdminSecret skipped the steps
                # that would have installed NASSa, so the harness imports the secret the DUT's export
                # has to return. The stored NASS is not fabric-scoped, so it is the same secret the
                # DUT's own ExportAdminSecret reads.
                nass_a = await self._import_secret_from_harness()
                timestamp_a = decode_network_administrator_secret(nass_a).created
                log.info("TimeStampA is %d (Matter epoch seconds), from the NASS the harness imported",
                         timestamp_a)
            # NASSa is the most recently *successful* import: step 4's was rejected. Confirm the TH
            # still holds exactly those bytes before quoting them, so the value the operator is asked
            # to match is pinned to the import rather than to whatever the server happens to return now.
            exported_nass = await self._export_stored_secret()
            asserts.assert_equal(exported_nass, nass_a,
                                 "ExportAdminSecret does not return NASSa, the most recently imported NASS.")
            decoded_export = decode_network_administrator_secret(exported_nass)
            asserts.assert_equal(decoded_export.created, timestamp_a,
                                 "The exported NASS does not carry TimeStampA.")
            self.ask_dut_to_send(
                "Using the DUT's interface, send an ExportAdminSecret command over CASE to the TH Network Identity "
                "Management server.",
                "the TH responded with an ExportAdminSecretResponse whose NetworkAdministratorSharedSecret is "
                f"{nass_a.hex()}, the most recently imported NASS.",
                expected_log=self.expect_admin_secret_transfer("exported"))


if __name__ == "__main__":
    default_matter_test_main()

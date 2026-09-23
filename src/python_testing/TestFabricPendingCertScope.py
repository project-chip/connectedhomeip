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
#     app: ${ALL_DEVICES_APP}
#     app-args: --device on-off-light:1 --discriminator 1234 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""RemoveFabric must not disturb operational certificates staged by another operation.

A node keeps a single set of pending operational certificates, owned by one fabric index at a
time. A RemoveFabric naming a different index has no claim on that pending set: the addition or
NOC update that staged it must still complete.
"""

import asyncio
import logging
import random

from mobly import asserts

import matter.clusters as Clusters
from matter.exceptions import ChipStackError
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main
from matter.utils import CommissioningBuildingBlocks

log = logging.getLogger(__name__)

# An index in the valid 1..254 range that holds no fabric on a freshly commissioned DUT.
UNOCCUPIED_FABRIC_INDEX = 200

FAILSAFE_SECONDS = 15


class TestFabricPendingCertScope(MatterBaseTest):
    # The framework wildcard subscription interferes with fail-safe driven fabric changes.
    disable_wildcard_subscription = True

    @property
    def opcreds(self):
        return Clusters.OperationalCredentials

    async def _read_fabric_indices(self) -> list:
        fabrics = await self.read_single_attribute_check_success(
            cluster=self.opcreds, attribute=self.opcreds.Attributes.Fabrics, fabric_filtered=False)
        return [fabric.fabricIndex for fabric in fabrics]

    async def _arm_failsafe(self, dev_ctrl, node_id, seconds: int):
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=seconds, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=dev_ctrl, node_id=node_id, cmd=cmd)
        asserts.assert_equal(
            resp.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            f"ArmFailSafe({seconds}) was not accepted")

    async def _release_failsafe_after_add(self, dev_ctrl, node_id):
        """Give up the fail-safe once a fabric has been added under it.

        A successful AddNOC moves the fail-safe context to the newly added fabric, so a disarm sent
        from the original fabric is answered with BusyWithOtherAdmin. That is correct behaviour, so
        accept it and let the fail-safe expire instead.
        """
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=0, breadcrumb=0)
        resp = await self.send_single_cmd(dev_ctrl=dev_ctrl, node_id=node_id, cmd=cmd)
        enums = Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum
        asserts.assert_in(
            resp.errorCode, [enums.kOk, enums.kBusyWithOtherAdmin],
            "Disarming the fail-safe returned an unexpected error")
        return resp.errorCode == enums.kOk

    async def _wait_for_fabric_list(self, expected: list, timeout_seconds: int = 40):
        """Poll until the fabric list matches, so fail-safe expiry has time to roll a fabric back."""
        deadline = timeout_seconds
        while True:
            current = await self._read_fabric_indices()
            if sorted(current) == sorted(expected):
                return current
            if deadline <= 0:
                return current
            await asyncio.sleep(2)
            deadline -= 2

    async def _remove_unoccupied_fabric_index(self, dev_ctrl, node_id):
        """Send RemoveFabric for an index that holds no fabric and check it is reported as such."""
        present = await self._read_fabric_indices()
        asserts.assert_not_in(
            UNOCCUPIED_FABRIC_INDEX, present,
            f"Test precondition: fabric index {UNOCCUPIED_FABRIC_INDEX} must not be in use, saw {present}")

        cmd = self.opcreds.Commands.RemoveFabric(fabricIndex=UNOCCUPIED_FABRIC_INDEX)
        resp = await self.send_single_cmd(dev_ctrl=dev_ctrl, node_id=node_id, cmd=cmd)
        asserts.assert_equal(
            resp.statusCode,
            self.opcreds.Enums.NodeOperationalCertStatusEnum.kInvalidFabricIndex,
            "RemoveFabric for an index holding no fabric must report InvalidFabricIndex")

    @async_test_body
    async def test_remove_fabric_during_fabric_addition(self):
        """A RemoveFabric for an unrelated index must not abort an addition already in progress."""
        dev_ctrl = self.default_controller
        node_id = self.dut_node_id

        fabrics_before = await self._read_fabric_indices()
        log.info("Fabric indices before the addition: %s", fabrics_before)

        # A separate certificate authority supplies the credentials of the fabric being added.
        new_ca = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        new_vendor_id = 0xFFF2
        new_fabric_admin = new_ca.NewFabricAdmin(vendorId=new_vendor_id, fabricId=2)
        new_admin_node_id = dev_ctrl.nodeId + 1
        new_admin = new_fabric_admin.NewController(nodeId=new_admin_node_id)

        await self._arm_failsafe(dev_ctrl, node_id, FAILSAFE_SECONDS)
        try:
            cmd = self.opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=False)
            csr = await self.send_single_cmd(dev_ctrl=dev_ctrl, node_id=node_id, cmd=cmd)

            certs = await new_admin.IssueNOCChain(csr, node_id)
            asserts.assert_is_not_none(certs.rcacBytes, "Could not issue a certificate chain for the new fabric")

            cmd = self.opcreds.Commands.AddTrustedRootCertificate(certs.rcacBytes)
            await self.send_single_cmd(dev_ctrl=dev_ctrl, node_id=node_id, cmd=cmd)
            log.info("Trusted root staged; the node now holds pending certificates for the new fabric")

            await self._remove_unoccupied_fabric_index(dev_ctrl, node_id)

            cmd = self.opcreds.Commands.AddNOC(
                NOCValue=certs.nocBytes,
                ICACValue=certs.icacBytes,
                IPKValue=certs.ipkBytes,
                caseAdminSubject=new_admin_node_id,
                adminVendorId=new_vendor_id)
            resp = await self.send_single_cmd(dev_ctrl=dev_ctrl, node_id=node_id, cmd=cmd)
            asserts.assert_equal(
                resp.statusCode,
                self.opcreds.Enums.NodeOperationalCertStatusEnum.kOk,
                "AddNOC must still succeed after a RemoveFabric naming an unrelated index")
        finally:
            # The fabric added above is rolled back when the fail-safe is given up.
            await self._release_failsafe_after_add(dev_ctrl, node_id)

        fabrics_after = await self._wait_for_fabric_list(fabrics_before)
        asserts.assert_equal(
            sorted(fabrics_after), sorted(fabrics_before),
            "Giving up the fail-safe should have restored the original fabric list")

    @async_test_body
    async def test_remove_fabric_during_noc_update(self):
        """A RemoveFabric for an unrelated index must not abort a NOC update already in progress."""
        dev_ctrl = self.default_controller
        node_id = self.dut_node_id

        # A second fabric is commissioned so that its NOC can be updated while the first fabric's
        # administrator sends the RemoveFabric.
        th2_ca = self.certificate_authority_manager.NewCertificateAuthority(maximizeCertChains=True)
        th2_fabric_admin = th2_ca.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)
        th2 = th2_fabric_admin.NewController(nodeId=self.matter_test_config.controller_node_id + 1)

        success, _, _ = await CommissioningBuildingBlocks.AddNOCForNewFabricFromExisting(
            commissionerDevCtrl=dev_ctrl, newFabricDevCtrl=th2, existingNodeId=node_id, newNodeId=node_id)
        asserts.assert_true(success, "Could not commission the fabric whose NOC is updated")

        th2_fabric_index = await self.read_single_attribute_check_success(
            dev_ctrl=th2, node_id=node_id, cluster=self.opcreds, attribute=self.opcreds.Attributes.CurrentFabricIndex)
        fabrics_before = await self._read_fabric_indices()
        log.info("Fabric indices before the update: %s, updating %d", fabrics_before, th2_fabric_index)

        try:
            await self._arm_failsafe(th2, node_id, FAILSAFE_SECONDS)

            cmd = self.opcreds.Commands.CSRRequest(CSRNonce=random.randbytes(32), isForUpdateNOC=True)
            csr = await self.send_single_cmd(dev_ctrl=th2, node_id=node_id, cmd=cmd)

            certs = await th2.IssueNOCChain(csr, node_id)
            asserts.assert_is_not_none(certs.nocBytes, "Could not issue an updated certificate chain")

            cmd = self.opcreds.Commands.UpdateNOC(NOCValue=certs.nocBytes, ICACValue=certs.icacBytes)
            resp = await self.send_single_cmd(dev_ctrl=th2, node_id=node_id, cmd=cmd)
            asserts.assert_equal(
                resp.statusCode, self.opcreds.Enums.NodeOperationalCertStatusEnum.kOk, "UpdateNOC was not accepted")
            # The node dropped the session that carried UpdateNOC; the next command opens one under the new NOC.
            th2.ExpireSessions(node_id)
            log.info("Updated NOC staged; the node now holds pending certificates for fabric %d", th2_fabric_index)

            await self._remove_unoccupied_fabric_index(dev_ctrl, node_id)

            # CommissioningComplete needs a new CASE session under the updated NOC, so it only
            # arrives if the node still holds that NOC.
            cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
            try:
                resp = await self.send_single_cmd(dev_ctrl=th2, node_id=node_id, cmd=cmd)
            except ChipStackError as e:
                asserts.fail(f"Could not reach the node under the updated NOC to complete the update: {e}")
            asserts.assert_equal(
                resp.errorCode,
                Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
                "CommissioningComplete must still succeed after a RemoveFabric naming an unrelated index")

            asserts.assert_equal(
                sorted(await self._read_fabric_indices()), sorted(fabrics_before),
                "The fabric whose NOC was updated must still be present")

            nocs = await self.read_single_attribute_check_success(
                dev_ctrl=th2, node_id=node_id, cluster=self.opcreds, attribute=self.opcreds.Attributes.NOCs)
            asserts.assert_equal(nocs[0].noc, certs.nocBytes, "The updated NOC must be the one committed")
        finally:
            if th2_fabric_index in await self._read_fabric_indices():
                cmd = self.opcreds.Commands.RemoveFabric(fabricIndex=th2_fabric_index)
                await self.send_single_cmd(dev_ctrl=dev_ctrl, node_id=node_id, cmd=cmd)


if __name__ == "__main__":
    default_matter_test_main()

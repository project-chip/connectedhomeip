#
#    Copyright (c) 2024 Project CHIP Authors
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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
#   run2:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --tests test_TC_CADMIN_1_4
#       --storage-path admin_storage.json
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: false
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import base64
import random
from time import sleep

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
from chip.interaction_model import Status
from chip.tlv import TLVReader
from matter_testing_infrastructure.chip.testing.matter_testing import (MatterBaseTest, TestStep, async_test_body,
                                                                       default_matter_test_main)
from mdns_discovery import mdns_discovery
from mobly import asserts

opcreds = Clusters.OperationalCredentials
nonce = random.randbytes(32)


class TC_CADMIN(MatterBaseTest):
    async def get_fabrics(self, th: ChipDeviceCtrl) -> int:
        OC_cluster = Clusters.OperationalCredentials
        fabric_info = await self.read_single_attribute_check_success(dev_ctrl=th, fabric_filtered=True, cluster=OC_cluster, attribute=OC_cluster.Attributes.Fabrics)
        return fabric_info

    async def get_txt_record(self):
        discovery = mdns_discovery.MdnsDiscovery(verbose_logging=True)
        comm_service = await discovery.get_commissionable_service(
            discovery_timeout_sec=240,
            log_output=False,
        )
        return comm_service

    async def write_nl_attr(self, th: ChipDeviceCtrl, attr_val: object):
        result = await th.WriteAttribute(nodeid=self.dut_node_id, attributes=[(0, attr_val)])
        asserts.assert_equal(result[0].Status, Status.Success, f"{th} node label write failed")

    async def read_nl_attr(self, th: ChipDeviceCtrl, attr_val: object):
        try:
            await th.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, attr_val)])
        except Exception as e:
            asserts.assert_equal(e.err, "Received error message from read attribute attempt")
            self.print_step(0, e)

    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        cluster = Clusters.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=th, endpoint=0, cluster=cluster, attribute=attribute)
        return current_fabric_index

    async def combined_commission_val_steps(self, commission_type: str):
        """
        Combined test function for commissioning tests.
        :param commission_type: Type of commissioning ("BCM" or "ECM").
        """
        self.step(1)
        if commission_type == "BCM":
            setupPayloadInfo = self.get_setup_payload_info()
            if not setupPayloadInfo:
                asserts.assert_true(
                    False, 'passcode and discriminator must be provided values in order for this test to work due to using BCM, please rerun test with providing --passcode <value> and --discriminator <value>')

        # Establishing TH1
        self.th1 = self.default_controller

        self.step(2)
        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        self.max_window_duration = duration.maxCumulativeFailsafeSeconds

        self.step("3a")
        if commission_type == "ECM":
            params = await self.th1.OpenCommissioningWindow(
                nodeid=self.dut_node_id,
                timeout=self.max_window_duration,
                iteration=1000,
                discriminator=1234,
                option=1
            )

        elif commission_type == "BCM":
            obcCmd = Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(180)
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=obcCmd, timedRequestTimeoutMs=6000)

        else:
            asserts.fail(f"Unknown commissioning type: {commission_type}")

        self.step("3b")
        services = await self.get_txt_record()
        expected_cm_value = "2" if commission_type == "ECM" else "1"
        if services.txt_record['CM'] != expected_cm_value:
            asserts.fail(f"Expected cm record value {expected_cm_value}, but found {services.txt_record['CM']}")

        self.step("3c")
        BI_cluster = Clusters.BasicInformation
        self.nl_attribute = BI_cluster.Attributes.NodeLabel
        await self.write_nl_attr(th=self.th1, attr_val=self.nl_attribute)
        await self.read_nl_attr(th=self.th1, attr_val=self.nl_attribute)

        self.step(4)
        # Establishing TH2
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        if commission_type == "ECM":
            await self.th2.CommissionOnNetwork(
                nodeId=self.dut_node_id,
                setupPinCode=params.setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
                filter=1234
            )
        else:
            setupPayloadInfo = self.get_setup_payload_info()
            if not setupPayloadInfo:
                asserts.fail("Setup payload info is required for basic commissioning.")
            await self.th2.CommissionOnNetwork(
                nodeId=self.dut_node_id,
                setupPinCode=setupPayloadInfo[0].passcode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
                filter=setupPayloadInfo[0].filter_value
            )
        rcac = self.th2.get_rcac()
        th2_rcac_decoded = TLVReader(rcac).get()["Any"][9]

        self.step(5)
        # TH_CR1 reads the Fabrics attribute
        th1_fabric_info = await self.get_fabrics(th=self.th1)
        th1_cam_rcac = TLVReader(base64.b64decode(
            self.certificate_authority_manager.activeCaList[0]._persistentStorage._jsonData["sdk-config"]["f/1/r"])).get()["Any"][9]
        if th1_fabric_info[0].rootPublicKey != th1_cam_rcac:
            asserts.fail("Public keys from fabric and certs for TH1 are not the same.")
        if th1_fabric_info[0].nodeID != self.dut_node_id:
            asserts.fail("DUT node ID from fabric does not equal DUT node ID for TH1 during commissioning.")

        self.step(6)
        # TH_CR2 reads the Fabrics attribute
        th2_fabric_info = await self.get_fabrics(th=self.th2)
        if th2_fabric_info[0].rootPublicKey != th2_rcac_decoded:
            asserts.fail("Public keys from fabric and certs for TH2 are not the same.")
        if th2_fabric_info[0].nodeID != self.dut_node_id:
            asserts.fail("DUT node ID from fabric does not equal DUT node ID for TH2 during commissioning.")

        if commission_type == "ECM":
            self.step(7)
            # TH_CR1 writes and reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE
            await self.write_nl_attr(th=self.th1, attr_val=self.nl_attribute)
            await self.read_nl_attr(th=self.th1, attr_val=self.nl_attribute)

            self.step(8)
            # TH_CR2 writes and reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE
            val = await self.read_nl_attr(th=self.th2, attr_val=self.nl_attribute)
            self.print_step("basic information cluster node label attr value", val)
            await self.write_nl_attr(th=self.th2, attr_val=self.nl_attribute)
            await self.read_nl_attr(th=self.th2, attr_val=self.nl_attribute)

            self.step(9)
            # TH_CR2 opens a commissioning window on DUT_CE for 180 seconds using ECM
            await self.th2.OpenCommissioningWindow(nodeid=self.dut_node_id, timeout=180, iteration=1000, discriminator=0, option=1)

            self.step(10)
            sleep(181)

            self.step(11)
            # TH_CR2 reads the window status to verify the DUT_CE window is closed
            # TODO: Issue noticed when initially attempting to check window status, issue is detailed here: https://github.com/project-chip/connectedhomeip/issues/35983
            # Workaround in place until above issue resolved
            try:
                window_status = await self.th2.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.AdministratorCommissioning.Attributes.WindowStatus)])
            except asyncio.CancelledError:
                window_status = await self.th2.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.AdministratorCommissioning.Attributes.WindowStatus)])

            window_status = window_status[0]
            outer_key = list(window_status.keys())[0]
            inner_key = list(window_status[outer_key].keys())[1]
            if window_status[outer_key][inner_key] != Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kWindowNotOpen:
                asserts.fail("Commissioning window is expected to be closed, but was found to be open")

            self.step(12)
            # TH_CR2 opens a commissioning window on DUT_CE using ECM
            self.discriminator = random.randint(0, 4095)
            # params2 = await self.openCommissioningWindow(dev_ctrl=self.th2, node_id=self.dut_node_id)
            params2 = await self.th2.OpenCommissioningWindow(nodeid=self.dut_node_id, timeout=self.max_window_duration, iteration=1000, discriminator=1234, option=1)

            self.step(13)
            # TH_CR1 starts a commissioning process with DUT_CE before the timeout from step 12
            try:
                await self.th1.CommissionOnNetwork(
                    nodeId=self.dut_node_id, setupPinCode=params2.setupPinCode,
                    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=1234)
            except ChipStackError as e:
                asserts.assert_equal(e.err,  0x0000007E,
                                     "Expected to return Trying to add NOC for fabric that already exists")
            """
            expected error:
                [2024-10-08 11:57:43.144125][TEST][STDOUT][MatterTest] 10-08 11:57:42.777 INFO Device returned status 9 on receiving the NOC
                [2024-10-08 11:57:43.144365][TEST][STDOUT][MatterTest] 10-08 11:57:42.777 INFO Add NOC failed with error src/controller/CHIPDeviceController.cpp:1712: CHIP Error 0x0000007E: Trying to add a NOC for a fabric that already exists
            """

            revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
            # The failsafe cleanup is scheduled after the command completes, so give it a bit of time to do that
            sleep(1)

        if commission_type == "ECM":
            self.step(14)

        elif commission_type == "BCM":
            self.step(7)

        # TH_CR2 reads the CurrentFabricIndex attribute from the Operational Credentials cluster and saves as th2_idx, TH_CR1 sends the RemoveFabric command to the DUT with the FabricIndex set to th2_idx
        th2_idx = await self.th2.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)])
        outer_key = list(th2_idx.keys())[0]
        inner_key = list(th2_idx[outer_key].keys())[0]
        attribute_key = list(th2_idx[outer_key][inner_key].keys())[1]
        removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(th2_idx[outer_key][inner_key][attribute_key])
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)

    def pics_TC_CADMIN_1_3(self) -> list[str]:
        return ["CADMIN.S"]

    def steps_TC_CADMIN_1_3(self) -> list[TestStep]:
        return [
            TestStep(1, "TH_CR1 starts a commissioning process with DUT_CE", is_commissioning=True),
            TestStep(2, "TH_CR1 reads the BasicCommissioningInfo attribute from the General Commissioning cluster and saves the MaxCumulativeFailsafeSeconds field as max_window_duration."),
            TestStep("3a", "TH_CR1 opens a commissioning window on DUT_CE using a commissioning timeout of max_window_duration using ECM",
                     "DUT_CE opens its Commissioning window to allow a second commissioning."),
            TestStep("3b", "DNS-SD records shows DUT_CE advertising", "Verify that the DNS-SD advertisement shows CM=2"),
            TestStep("3c", "TH_CR1 writes and reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE",
                     "Verify DUT_CE responds to both write/read with a success"),
            TestStep(4, "TH creates a controller (TH_CR2) on a new fabric and commissions DUT_CE using that controller. TH_CR2 should commission the device using a different NodeID than TH_CR1.",
                     "Commissioning is successful"),
            TestStep(5, "TH_CR1 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                     "Verify that the RootPublicKey matches the root public key for TH_CR1 and the NodeID matches the node ID used when TH_CR1 commissioned the device."),
            TestStep(6, "TH_CR2 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                     "Verify that the RootPublicKey matches the root public key for TH_CR2 and the NodeID matches the node ID used when TH_CR2 commissioned the device."),
            TestStep(7, "TH_CR1 writes and reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE",
                     "Verify DUT_CE responds to both write/read with a success"),
            TestStep(8, "TH_CR2 reads, writes and then reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE",
                     "Verify the initial read reflect the value written in the above step. Verify DUT_CE responds to both write/read with a success"),
            TestStep(9, "TH_CR2 opens a commissioning window on DUT_CE for 180 seconds using ECM"),
            TestStep(10, "Wait for the commissioning window in step 9 to timeout"),
            TestStep(11, "TH_CR2 reads the window status to verify the DUT_CE window is closed",
                     "DUT_CE windows status shows the window is closed"),
            TestStep(12, "TH_CR2 opens a commissioning window on DUT_CE using ECM",
                     "DUT_CE opens its Commissioning window to allow a new commissioning"),
            TestStep(13, "TH_CR1 starts a commissioning process with DUT_CE before the timeout from step 12",
                     "Since DUT_CE was already commissioned by TH_CR1 in step 1, AddNOC fails with NOCResponse with StatusCode field set to FabricConflict (9)"),
            TestStep(14, "TH_CR2 reads the CurrentFabricIndex attribute from the Operational Credentials cluster and saves as th2_idx, TH_CR1 sends the RemoveFabric command to the DUT with the FabricIndex set to th2_idx",
                     "TH_CR1 removes TH_CR2 fabric using th2_idx")
        ]

    def pics_TC_CADMIN_1_4(self) -> list[str]:
        return ["CADMIN.S"]

    def steps_TC_CADMIN_1_4(self) -> list[TestStep]:
        return [
            TestStep(1, "TH_CR1 starts a commissioning process with DUT_CE", is_commissioning=True),
            TestStep(2, "TH_CR1 reads the BasicCommissioningInfo attribute from the General Commissioning cluster and saves the MaxCumulativeFailsafeSeconds field as max_window_duration."),
            TestStep("3a", "TH_CR1 opens a commissioning window on DUT_CE using a commissioning timeout of max_window_duration using BCM",
                     "DUT_CE opens its Commissioning window to allow a second commissioning."),
            TestStep("3b", "DNS-SD records shows DUT_CE advertising", "Verify that the DNS-SD advertisement shows CM=1"),
            TestStep("3c", "TH_CR1 writes and reads the Basic Information Cluster’s NodeLabel mandatory attribute of DUT_CE",
                     "Verify DUT_CE responds to both write/read with a success"),
            TestStep(4, "TH creates a controller (TH_CR2) on a new fabric and commissions DUT_CE using that controller. TH_CR2 should commission the device using a different NodeID than TH_CR1.",
                     "Commissioning is successful"),
            TestStep(5, "TH_CR1 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                     "Verify that the RootPublicKey matches the root public key for TH_CR1 and the NodeID matches the node ID used when TH_CR1 commissioned the device."),
            TestStep(6, "TH_CR2 reads the Fabrics attribute from the Node Operational Credentials cluster using a fabric-filtered read",
                     "Verify that the RootPublicKey matches the root public key for TH_CR2 and the NodeID matches the node ID used when TH_CR2 commissioned the device."),
            TestStep(7, "TH_CR2 reads the CurrentFabricIndex attribute from the Operational Credentials cluster and saves as th2_idx, TH_CR1 sends the RemoveFabric command to the DUT with the FabricIndex set to th2_idx",
                     "TH_CR1 removes TH_CR2 fabric using th2_idx")
        ]

    @async_test_body
    async def test_TC_CADMIN_1_3(self):
        await self.combined_commission_val_steps(commission_type="ECM")

    @async_test_body
    async def test_TC_CADMIN_1_4(self):
        await self.combined_commission_val_steps(commission_type="BCM")


if __name__ == "__main__":
    default_matter_test_main()

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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import random

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.testing.matter_testing import (AttributeMatcher, ClusterAttributeChangeAccumulator, MatterBaseTest, TestStep,
                                         async_test_body, default_matter_test_main)
from mobly import asserts


class TC_CADMIN_1_25(MatterBaseTest):
    min_report_interval_sec = 0
    max_report_interval_sec = 30

    async def get_fabrics(self, th: ChipDeviceCtrl) -> int:
        OC_cluster = Clusters.OperationalCredentials
        fabrics = await self.read_single_attribute_check_success(dev_ctrl=th, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.Fabrics)
        return fabrics

    def steps_TC_CADMIN_1_25(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH_CR1 subscribes to WindowStatus attribute on DUT_CE",
                     "Verify TH_CR1 receives WindowStatus subscription notification"),
            TestStep(3, "TH_CR1 subscribes to AdminFabricIndex attribute on DUT_CE",
                     "Verify TH_CR1 receives AdminFabricIndex subscription notification"),
            TestStep(4, "TH_CR1 subscribes to AdminVendorId attribute on DUT_CE",
                     "Verify TH_CR1 receives AdminVendorId subscription notification"),
            TestStep(5, "TH_CR1 reads the BasicCommissioningInfo attribute from the General Commissioning cluster and saves the MaxCumulativeFailsafeSeconds field as `max_window_duration`."),
            TestStep(6, "TH_CR1 send an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of `max_window_duration`",
                     "{resDutSuccess}"),
            TestStep(7, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric"),
            TestStep(8, "TH_CR2 starts a commissioning process with DUT_CE", "DUT_CE is commissioned by TH_CR2"),
            TestStep(9, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null"),
            TestStep(10, "TH_CR2 subscribes to WindowStatus attribute on DUT_CE",
                     "Verify TH_CR2 receives WindowStatus subscription notification"),
            TestStep(11, "TH_CR2 subscribes to AdminFabricIndex attribute on DUT_CE",
                     "Verify TH_CR2 receives AdminFabricIndex subscription notification"),
            TestStep(12, "TH_CR2 subscribes to AdminVendorId attribute on DUT_CE",
                     "Verify TH_CR2 receives AdminVendorId subscription notification"),
            TestStep(13, "TH_CR1 sends an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of `max_window_duration`",
                     "{resDutSuccess}"),
            TestStep(14, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric"),
            TestStep(15, "Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric"),
            TestStep(16, "TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command",
                     "Verify DUT_CE closes its Commissioning window"),
            TestStep(17, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null"),
            TestStep(18, "Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null"),
            TestStep(19, "TH_CR2 opens a commissioning window on DUT_CE using ECM with commissioning timeout of `max_window_duration`",
                     "{resDutSuccess}"),
            TestStep(20, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric"),
            TestStep(21, "Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric"),
            TestStep(22, "TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command",
                     "Verify DUT_CE closes its Commissioning window"),
            TestStep(23, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null"),
            TestStep(24, "Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null"),
            TestStep(
                25, "TH_CR2 send an OpenCommissioningWindow command to DUT_CE using ECM with a commissioning timeout of `max_window_duration`", "{resDutSuccess}"),
            TestStep(26, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric"),
            TestStep(27, "Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric"),
            TestStep(28, "Before expiration of `max_window_duration` set in step 25, TH_CR1 sends RemoveFabric command to DUT_CE with FabricIndex set to the fabric index of TH_CR2's fabric",
                     "Verify DUT_CE responses with NOCResponse with a StatusCode OK (note that expecting OK should ONLY work if an administrator/commissioner on another fabric than the one being removed is invoking RemoveFabric)."),
            TestStep(29, "Verify TH_CR1 receives subscription notifications which show AdminFabricIndex value to be null"),
            TestStep(30, "TH_CR1 reads WindowStatus attribute from DUT_CE",
                     "verify the value to be 1 indicating the window is still open"),
            TestStep(31, "TH_CR1 reads AdminVendorID attribute from DUT_CE",
                     "verify the value to be the same as the Admin Vendor ID of the Fabrics attribute list entry corresponding to TH_CR2's fabric"),
            TestStep(32, "TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command",
                     "Verify DUT_CE closes its Commissioning window"),
            TestStep(33, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminVendorId to be null"),
        ]

    def pics_TC_CADMIN_1_25(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_25(self):
        self.step(1)
        # Establishing TH1 and TH2
        self.th1 = self.default_controller
        self.discriminator = random.randint(0, 4095)
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2, useTestCommissioner=True)

        self.step(2)
        # TH_CR1 subscribes to WindowStatus attribute on DUT_CE
        th1_window_status_accumulator = ClusterAttributeChangeAccumulator(
            Clusters.AdministratorCommissioning,
            Clusters.AdministratorCommissioning.Attributes.WindowStatus)
        await th1_window_status_accumulator.start(
            self.th1, self.dut_node_id, 0, fabric_filtered=True,
            min_interval_sec=self.min_report_interval_sec,
            max_interval_sec=self.max_report_interval_sec)

        self.step(3)
        # TH_CR1 subscribes to AdminFabricIndex attribute on DUT_CE
        th1_admin_fabric_index_accumulator = ClusterAttributeChangeAccumulator(
            Clusters.AdministratorCommissioning,
            Clusters.AdministratorCommissioning.Attributes.AdminFabricIndex)
        await th1_admin_fabric_index_accumulator.start(
            self.th1, self.dut_node_id, 0, fabric_filtered=True,
            min_interval_sec=self.min_report_interval_sec,
            max_interval_sec=self.max_report_interval_sec)

        self.step(4)
        # TH_CR1 subscribes to AdminVendorId attribute on DUT_CE
        th1_admin_vendor_id_accumulator = ClusterAttributeChangeAccumulator(
            Clusters.AdministratorCommissioning,
            Clusters.AdministratorCommissioning.Attributes.AdminVendorId)
        await th1_admin_vendor_id_accumulator.start(
            self.th1, self.dut_node_id, 0, fabric_filtered=True,
            min_interval_sec=self.min_report_interval_sec,
            max_interval_sec=self.max_report_interval_sec)

        self.step(5)
        # TH_CR1 reads the BasicCommissioningInfo attribute from the General Commissioning cluster
        # and saves the MaxCumulativeFailsafeSeconds field as `max_window_duration`."
        GC_cluster = Clusters.GeneralCommissioning
        attribute = GC_cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=GC_cluster, attribute=attribute)
        max_window_duration = duration.maxCumulativeFailsafeSeconds

        self.step(6)
        # TH_CR1 send an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of `max_window_duration`
        self.discriminator = random.randint(0, 4095)
        params = await self.th1.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=max_window_duration, iteration=10000,
            discriminator=self.discriminator, option=1)

        self.step(7)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1,
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric,
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric
        fabrics = await self.get_fabrics(self.th1)
        th1_admin_fabric_vendor_id = fabrics[0].vendorID
        th1_admin_fabric_index = fabrics[0].fabricIndex

        # Create attribute matchers
        window_status_match = AttributeMatcher.from_callable(
            "WindowStatus is 1",
            lambda report: report.value == 1)

        fabric_index_match = AttributeMatcher.from_callable(
            f"AdminFabricIndex is {th1_admin_fabric_index}",
            lambda report: report.value == th1_admin_fabric_index)

        vendor_id_match = AttributeMatcher.from_callable(
            f"AdminVendorId is {th1_admin_fabric_vendor_id}",
            lambda report: report.value == th1_admin_fabric_vendor_id)

        # Wait for attribute reports to match
        th1_window_status_accumulator.await_all_expected_report_matches([window_status_match], timeout_sec=10)
        th1_admin_fabric_index_accumulator.await_all_expected_report_matches([fabric_index_match], timeout_sec=10)
        th1_admin_vendor_id_accumulator.await_all_expected_report_matches([vendor_id_match], timeout_sec=10)

        # Reset TH1 accumulators
        th1_window_status_accumulator.reset()
        th1_admin_fabric_index_accumulator.reset()
        th1_admin_vendor_id_accumulator.reset()

        self.step(8)
        # TH_CR2 starts a commissioning process with DUT_CE
        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id,
            setupPinCode=params.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
            filter=self.discriminator
        )

        self.step(9)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0
        # AdminFabricIndex value to be null and AdminVendorId to be null
        window_status_0_match = AttributeMatcher.from_callable(
            "WindowStatus is 0",
            lambda report: report.value == 0)

        null_match = AttributeMatcher.from_callable(
            "Attribute is null",
            lambda report: str(type(report.value)).find('chip.clusters.Types.Nullable') >= 0)

        th1_window_status_accumulator.await_all_expected_report_matches([window_status_0_match], timeout_sec=10)
        th1_admin_fabric_index_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)
        th1_admin_vendor_id_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)

        # Reset TH1 accumulators
        th1_window_status_accumulator.reset()
        th1_admin_fabric_index_accumulator.reset()
        th1_admin_vendor_id_accumulator.reset()

        self.step(10)
        # TH_CR2 subscribes to WindowStatus attribute on DUT_CE
        th2_window_status_accumulator = ClusterAttributeChangeAccumulator(
            Clusters.AdministratorCommissioning,
            Clusters.AdministratorCommissioning.Attributes.WindowStatus)
        await th2_window_status_accumulator.start(
            self.th2, self.dut_node_id, 0, fabric_filtered=True,
            min_interval_sec=self.min_report_interval_sec,
            max_interval_sec=self.max_report_interval_sec)

        self.step(11)
        # TH_CR2 subscribes to AdminFabricIndex attribute on DUT_CE
        th2_admin_fabric_index_accumulator = ClusterAttributeChangeAccumulator(
            Clusters.AdministratorCommissioning,
            Clusters.AdministratorCommissioning.Attributes.AdminFabricIndex)
        await th2_admin_fabric_index_accumulator.start(
            self.th2, self.dut_node_id, 0, fabric_filtered=True,
            min_interval_sec=self.min_report_interval_sec,
            max_interval_sec=self.max_report_interval_sec)

        self.step(12)
        # TH_CR2 subscribes to AdminVendorId attribute on DUT_CE
        th2_admin_vendor_id_accumulator = ClusterAttributeChangeAccumulator(
            Clusters.AdministratorCommissioning,
            Clusters.AdministratorCommissioning.Attributes.AdminVendorId)
        await th2_admin_vendor_id_accumulator.start(
            self.th2, self.dut_node_id, 0, fabric_filtered=True,
            min_interval_sec=self.min_report_interval_sec,
            max_interval_sec=self.max_report_interval_sec)

        self.step(13)
        # TH_CR1 sends an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of `max_window_duration`
        await self.th1.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=max_window_duration, iteration=10000,
            discriminator=self.discriminator, option=1)

        self.step(14)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric
        th1_window_status_accumulator.await_all_expected_report_matches([window_status_match], timeout_sec=10)
        th1_admin_fabric_index_accumulator.await_all_expected_report_matches([fabric_index_match], timeout_sec=10)
        th1_admin_vendor_id_accumulator.await_all_expected_report_matches([vendor_id_match], timeout_sec=10)

        # Reset TH1 accumulators
        th1_window_status_accumulator.reset()
        th1_admin_fabric_index_accumulator.reset()
        th1_admin_vendor_id_accumulator.reset()

        self.step(15)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric
        th2_fabrics = await self.get_fabrics(self.th2)
        th2_admin_fabric_vendor_id = th2_fabrics[1].vendorID
        th2_admin_fabric_index = th2_fabrics[1].fabricIndex

        th2_fabric_index_match = AttributeMatcher.from_callable(
            f"AdminFabricIndex is {th1_admin_fabric_index}",
            lambda report: report.value == th1_admin_fabric_index)

        th2_vendor_id_match = AttributeMatcher.from_callable(
            f"AdminVendorId is {th1_admin_fabric_vendor_id}",
            lambda report: report.value == th1_admin_fabric_vendor_id)

        th2_window_status_accumulator.await_all_expected_report_matches([window_status_match], timeout_sec=10)
        th2_admin_fabric_index_accumulator.await_all_expected_report_matches([th2_fabric_index_match], timeout_sec=10)
        th2_admin_vendor_id_accumulator.await_all_expected_report_matches([th2_vendor_id_match], timeout_sec=10)

        # Reset TH2 accumulators
        th2_window_status_accumulator.reset()
        th2_admin_fabric_index_accumulator.reset()
        th2_admin_vendor_id_accumulator.reset()

        self.step(16)
        # TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)

        self.step(17)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null
        th1_window_status_accumulator.await_all_expected_report_matches([window_status_0_match], timeout_sec=10)
        th1_admin_fabric_index_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)
        th1_admin_vendor_id_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)

        # Reset TH1 accumulators
        th1_window_status_accumulator.reset()
        th1_admin_fabric_index_accumulator.reset()
        th1_admin_vendor_id_accumulator.reset()

        self.step(18)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null
        th2_window_status_accumulator.await_all_expected_report_matches([window_status_0_match], timeout_sec=10)
        th2_admin_fabric_index_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)
        th2_admin_vendor_id_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)

        # Reset TH2 accumulators
        th2_window_status_accumulator.reset()
        th2_admin_fabric_index_accumulator.reset()
        th2_admin_vendor_id_accumulator.reset()

        self.step(19)
        # TH_CR2 opens a commissioning window on DUT_CE using ECM with commissioning timeout of `max_window_duration`
        await self.th2.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=max_window_duration, iteration=10000,
            discriminator=self.discriminator, option=1)

        self.step(20)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric
        th2_fabric_admin_index_match = AttributeMatcher.from_callable(
            f"AdminFabricIndex is {th2_admin_fabric_index}",
            lambda report: report.value == th2_admin_fabric_index)

        th2_fabric_vendor_id_match = AttributeMatcher.from_callable(
            f"AdminVendorId is {th2_admin_fabric_vendor_id}",
            lambda report: report.value == th2_admin_fabric_vendor_id)

        th1_window_status_accumulator.await_all_expected_report_matches([window_status_match], timeout_sec=10)
        th1_admin_fabric_index_accumulator.await_all_expected_report_matches([th2_fabric_admin_index_match], timeout_sec=10)
        th1_admin_vendor_id_accumulator.await_all_expected_report_matches([th2_fabric_vendor_id_match], timeout_sec=10)

        # Reset TH1 accumulators
        th1_window_status_accumulator.reset()
        th1_admin_fabric_index_accumulator.reset()
        th1_admin_vendor_id_accumulator.reset()

        self.step(21)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric
        th2_window_status_accumulator.await_all_expected_report_matches([window_status_match], timeout_sec=10)
        th2_admin_fabric_index_accumulator.await_all_expected_report_matches([th2_fabric_admin_index_match], timeout_sec=10)
        th2_admin_vendor_id_accumulator.await_all_expected_report_matches([th2_fabric_vendor_id_match], timeout_sec=10)

        # Reset TH2 accumulators
        th2_window_status_accumulator.reset()
        th2_admin_fabric_index_accumulator.reset()
        th2_admin_vendor_id_accumulator.reset()

        self.step(22)
        # TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)

        self.step(23)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0
        # AdminFabricIndex value to be null andAdminVendorId to be null
        th1_window_status_accumulator.await_all_expected_report_matches([window_status_0_match], timeout_sec=10)
        th1_admin_fabric_index_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)
        th1_admin_vendor_id_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)

        # Reset TH1 accumulators
        th1_window_status_accumulator.reset()
        th1_admin_fabric_index_accumulator.reset()
        th1_admin_vendor_id_accumulator.reset()

        self.step(24)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 0
        # AdminFabricIndex value to be null and AdminVendorId to be null
        th2_window_status_accumulator.await_all_expected_report_matches([window_status_0_match], timeout_sec=10)
        th2_admin_fabric_index_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)
        th2_admin_vendor_id_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)

        # Reset TH2 accumulators
        th2_window_status_accumulator.reset()
        th2_admin_fabric_index_accumulator.reset()
        th2_admin_vendor_id_accumulator.reset()

        self.step(25)
        # TH_CR2 send an OpenCommissioningWindow command to DUT_CE using ECM with a commissioning timeout of `max_window_duration`
        await self.th2.OpenCommissioningWindow(
            nodeid=self.dut_node_id, timeout=max_window_duration, iteration=10000,
            discriminator=self.discriminator, option=1)

        self.step(26)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric
        th1_window_status_accumulator.await_all_expected_report_matches([window_status_match], timeout_sec=10)
        th1_admin_fabric_index_accumulator.await_all_expected_report_matches([th2_fabric_admin_index_match], timeout_sec=10)
        th1_admin_vendor_id_accumulator.await_all_expected_report_matches([th2_fabric_vendor_id_match], timeout_sec=10)

        # Reset TH1 accumulators
        th1_window_status_accumulator.reset()
        th1_admin_fabric_index_accumulator.reset()
        th1_admin_vendor_id_accumulator.reset()

        self.step(27)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric
        th2_window_status_accumulator.await_all_expected_report_matches([window_status_match], timeout_sec=10)
        th2_admin_fabric_index_accumulator.await_all_expected_report_matches([th2_fabric_admin_index_match], timeout_sec=10)
        th2_admin_vendor_id_accumulator.await_all_expected_report_matches([th2_fabric_vendor_id_match], timeout_sec=10)

        # Reset TH2 accumulators
        th2_window_status_accumulator.reset()
        th2_admin_fabric_index_accumulator.reset()
        th2_admin_vendor_id_accumulator.reset()

        self.step(28)
        # Before expiration of `max_window_duration` set in step 25,
        # TH_CR1 sends RemoveFabric command to DUT_CE with FabricIndex set to the fabric index of TH_CR2's fabric
        th2_idx = await self.th2.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(0, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)])
        outer_key = list(th2_idx.keys())[0]
        inner_key = list(th2_idx[outer_key].keys())[0]
        attribute_key = list(th2_idx[outer_key][inner_key].keys())[1]
        removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(th2_idx[outer_key][inner_key][attribute_key])
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)

        self.step(29)
        # Verify TH_CR1 receives subscription notifications which show AdminFabricIndex value to be null
        th1_admin_fabric_index_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)

        self.step(30)
        # TH_CR1 reads WindowStatus attribute from DUT_CE
        # verify the value to be 1 indicating the window is still open
        AC_cluster = Clusters.AdministratorCommissioning
        window_status = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, fabric_filtered=False, endpoint=0,
            cluster=AC_cluster, attribute=AC_cluster.Attributes.WindowStatus)
        asserts.assert_equal(
            window_status,
            Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kEnhancedWindowOpen,
            "Commissioning window is expected to be open, but was found to be closed")

        self.step(31)
        # TH_CR1 reads AdminVendorID attribute from DUT_CE
        admin_vendor_id = await self.read_single_attribute_check_success(
            dev_ctrl=self.th1, fabric_filtered=False, endpoint=0,
            cluster=AC_cluster, attribute=AC_cluster.Attributes.AdminVendorId)
        asserts.assert_equal(
            admin_vendor_id, th2_admin_fabric_vendor_id,
            "AdminVendorId is not the expected value")

        self.step(32)
        # TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)

        self.step(33)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminVendorId to be null
        th1_window_status_accumulator.await_all_expected_report_matches([window_status_0_match], timeout_sec=10)
        th1_admin_vendor_id_accumulator.await_all_expected_report_matches([null_match], timeout_sec=10)


if __name__ == "__main__":
    default_matter_test_main()

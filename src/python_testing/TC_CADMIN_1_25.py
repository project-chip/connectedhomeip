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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import random
import queue
from queue import Empty
import time
from time import sleep

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.exceptions import ChipStackError
from chip.native import PyChipError
from chip.clusters.Types import Nullable
from chip.testing.matter_testing import AttributeChangeCallback, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_SC_3_6 import AttributeChangeAccumulator


class TC_CADMIN_1_25(MatterBaseTest):
    min_report_interval_sec = 0
    max_report_interval_sec = 30

    async def subscribe_window_status(self, ctrl):
        sub = await ctrl.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(0, Clusters.AdministratorCommissioning.Attributes.WindowStatus)],
            reportInterval=(self.min_report_interval_sec, self.max_report_interval_sec),
            keepSubscriptions=True,
            fabricFiltered=True
        )
        return sub

    async def subscribe_admin_fabric_index(self, ctrl):
        sub = await ctrl.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(0, Clusters.AdministratorCommissioning.Attributes.AdminFabricIndex)],
            reportInterval=(self.min_report_interval_sec, self.max_report_interval_sec),
            keepSubscriptions=True,
            fabricFiltered=True
        )
        return sub

    async def subscribe_admin_vendor_id(self, ctrl):
        sub = await ctrl.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=[(0, Clusters.AdministratorCommissioning.Attributes.AdminVendorId)],
            reportInterval=(self.min_report_interval_sec, self.max_report_interval_sec),
            keepSubscriptions=True,
            fabricFiltered=True
        )
        return sub

    async def get_fabrics(self, th: ChipDeviceCtrl) -> int:
        OC_cluster = Clusters.OperationalCredentials
        fabrics = await self.read_single_attribute_check_success(dev_ctrl=th, fabric_filtered=False, endpoint=0, cluster=OC_cluster, attribute=OC_cluster.Attributes.Fabrics)
        return fabrics

    async def wait_for_subscription_value(self, queue_obj, expected_value=None, timeout=10):
        """
        Wait for a subscription notification with the specified value.
        For null checks, expected_value should be None.
        """
        start_time = time.time()

        # Helper function to check if a value is effectively null
        def is_nullable(value):
            if value is None:
                return True
            # Check for Nullable instance from chip.clusters.Types
            if str(type(value)).find('chip.clusters.Types.Nullable') >= 0:
                return True
            return False

        # Helper function to compare values, handling nullables
        def values_match(actual, expected):
            # If we're expecting null and actual is nullable
            if expected is None and is_nullable(actual):
                return True
            # If both values are null
            if is_nullable(expected) and is_nullable(actual):
                return True
            # Normal equality check for non-null values
            return actual == expected

        while time.time() - start_time < timeout:
            try:
                # Non-blocking check to see if there's data
                report = queue_obj.get(block=False)
                logging.info(f"Received subscription report: {report}")

                value = report.get("value")
                logging.info(f"Report value: {value}, type: {type(value)}")

                # Check if we have the expected value
                if values_match(value, expected_value):
                    return report

            except Empty:
                # Wait a bit before trying again
                await asyncio.sleep(0.5)

        # If we got here, we didn't find what we wanted within the timeout
        logging.error(f"Timed out waiting for subscription value {expected_value}")
        return None

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
            TestStep(
                6, "TH_CR1 send an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of `max_window_duration`", "{resDutSuccess}"),
            TestStep(7, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric"),
            TestStep(8, "TH_CR2 starts a commissioning process with DUT_CE", "DUT_CE is commissioned by TH_CR2"),
            TestStep(9, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null"),
            TestStep(10, "TH_CR2 subscribes to WindowStatus attribute on DUT_CE",
                     "Verify TH_CR2 receives WindowStatus subscription notification"),
            TestStep(11, "TH_CR2 subscribes to AdminFabricIndex attribute on DUT_CE",
                     "Verify TH_CR2 receives AdminFabricIndex subscription notification"),
            TestStep(12, "TH_CR2 subscribes to AdminVendorId attribute on DUT_CE",
                     "Verify TH_CR2 receives AdminVendorId subscription notification"),
            TestStep(
                13, "TH_CR1 sends an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of `max_window_duration`", "{resDutSuccess}"),
            TestStep(14, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric"),
            TestStep(15, "Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1, AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric, AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric"),
            TestStep(16, "TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command",
                     "Verify DUT_CE closes its Commissioning window"),
            TestStep(17, "Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null"),
            TestStep(18, "Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null"),
            TestStep(
                19, "TH_CR2 opens a commissioning window on DUT_CE using ECM with commissioning timeout of `max_window_duration`", "{resDutSuccess}"),
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
            TestStep(28, "Before expiration of `max_window_duration` set in step 25, TH_CR1 sends _RemoveFabric_ command to DUT_CE with FabricIndex set to the fabric index of TH_CR2's fabric",
                     "Verify DUT_CE responses with _NOCResponse_ with a StatusCode OK (note that expecting OK should ONLY work if an administrator/commissioner on another fabric than the one being removed is invoking RemoveFabric)."),
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
        th1_window_status_sub = await self.subscribe_window_status(self.th1)
        th1_window_status_queue = queue.Queue()
        th1_window_status_handler = AttributeChangeAccumulator(
            name=self.th1.name, expected_attribute=Clusters.AdministratorCommissioning.Attributes.WindowStatus, output=th1_window_status_queue)
        th1_window_status_sub.SetAttributeUpdateCallback(th1_window_status_handler)

        self.step(3)
        # TH_CR1 subscribes to AdminFabricIndex attribute on DUT_CE
        th1_admin_fabric_index_sub = await self.subscribe_admin_fabric_index(self.th1)
        th1_admin_fabric_index_queue = queue.Queue()
        th1_admin_fabric_index_handler = AttributeChangeAccumulator(
            name=self.th1.name, expected_attribute=Clusters.AdministratorCommissioning.Attributes.AdminFabricIndex, output=th1_admin_fabric_index_queue)
        th1_admin_fabric_index_sub.SetAttributeUpdateCallback(th1_admin_fabric_index_handler)

        self.step(4)
        # TH_CR1 subscribes to AdminVendorId attribute on DUT_CE
        th1_admin_vendor_id_sub = await self.subscribe_admin_vendor_id(self.th1)
        th1_admin_vendor_id_queue = queue.Queue()
        th1_admin_vendor_id_handler = AttributeChangeAccumulator(
            name=self.th1.name, expected_attribute=Clusters.AdministratorCommissioning.Attributes.AdminVendorId, output=th1_admin_vendor_id_queue)
        th1_admin_vendor_id_sub.SetAttributeUpdateCallback(th1_admin_vendor_id_handler)

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
        params = await self.th1.OpenCommissioningWindow(nodeid=self.dut_node_id, timeout=max_window_duration, iteration=10000,
                                                        discriminator=self.discriminator, option=1)

        self.step(7)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1,
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric,
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric
        fabrics = await self.get_fabrics(self.th1)
        th1_admin_fabric_vendor_id = fabrics[0].vendorID
        th1_admin_fabric_index = fabrics[0].fabricIndex

        th1_window_status_report = await self.wait_for_subscription_value(
            th1_window_status_queue, expected_value=1)
        asserts.assert_is_not_none(th1_window_status_report, "No WindowStatus report received")
        asserts.assert_equal(th1_window_status_report["value"], 1, "WindowStatus value is not 1")

        th1_admin_fabric_index_report = await self.wait_for_subscription_value(
            th1_admin_fabric_index_queue, expected_value=th1_admin_fabric_index)
        asserts.assert_is_not_none(th1_admin_fabric_index_report, "No AdminFabricIndex report received")
        asserts.assert_equal(th1_admin_fabric_index_report.get("value"), th1_admin_fabric_index,
                             "AdminFabricIndex value is not the same as the Fabric Index")

        th1_admin_vendor_id_report = await self.wait_for_subscription_value(
            th1_admin_vendor_id_queue, expected_value=th1_admin_fabric_vendor_id)
        asserts.assert_is_not_none(th1_admin_vendor_id_report, "No AdminVendorId report received")
        asserts.assert_equal(th1_admin_vendor_id_report.get("value"), th1_admin_fabric_vendor_id,
                             "AdminVendorId value is not the same as the Vendor ID")

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
        th1_window_status_report2 = await self.wait_for_subscription_value(
            th1_window_status_queue, expected_value=0)
        asserts.assert_is_not_none(th1_window_status_report2, "No WindowStatus report received")
        asserts.assert_equal(th1_window_status_report2["value"], 0, "WindowStatus value is not 0")

        th1_admin_fabric_index_report2 = await self.wait_for_subscription_value(
            th1_admin_fabric_index_queue, expected_value=None)
        value = th1_admin_fabric_index_report2.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminFabricIndex value is not null")

        th1_admin_vendor_id_report2 = await self.wait_for_subscription_value(
            th1_admin_vendor_id_queue, expected_value=None)
        value = th1_admin_vendor_id_report2.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminVendorId value is not null")

        self.step(10)
        # TH_CR2 subscribes to WindowStatus attribute on DUT_CE
        th2_window_status_sub = await self.subscribe_window_status(self.th2)
        th2_window_status_queue = queue.Queue()
        th2_window_status_handler = AttributeChangeAccumulator(
            name=self.th2.name, expected_attribute=Clusters.AdministratorCommissioning.Attributes.WindowStatus, output=th2_window_status_queue)
        th2_window_status_sub.SetAttributeUpdateCallback(th2_window_status_handler)

        self.step(11)
        # TH_CR2 subscribes to AdminFabricIndex attribute on DUT_CE
        th2_admin_fabric_index_sub = await self.subscribe_admin_fabric_index(self.th2)
        th2_admin_fabric_index_queue = queue.Queue()
        th2_admin_fabric_index_handler = AttributeChangeAccumulator(
            name=self.th2.name, expected_attribute=Clusters.AdministratorCommissioning.Attributes.AdminFabricIndex, output=th2_admin_fabric_index_queue)
        th2_admin_fabric_index_sub.SetAttributeUpdateCallback(th2_admin_fabric_index_handler)

        self.step(12)
        # TH_CR2 subscribes to AdminVendorId attribute on DUT_CE
        th2_admin_vendor_id_sub = await self.subscribe_admin_vendor_id(self.th2)
        th2_admin_vendor_id_queue = queue.Queue()
        th2_admin_vendor_id_handler = AttributeChangeAccumulator(
            name=self.th2.name, expected_attribute=Clusters.AdministratorCommissioning.Attributes.AdminVendorId, output=th2_admin_vendor_id_queue)
        th2_admin_vendor_id_sub.SetAttributeUpdateCallback(th2_admin_vendor_id_handler)

        self.step(13)
        # TH_CR1 sends an OpenCommissioningWindow command to DUT_CE using a commissioning timeout of `max_window_duration`
        await self.th1.OpenCommissioningWindow(nodeid=self.dut_node_id, timeout=max_window_duration, iteration=10000,
                                               discriminator=self.discriminator, option=1)

        self.step(14)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric
        th1_window_status_report3 = await self.wait_for_subscription_value(
            th1_window_status_queue, expected_value=1)
        asserts.assert_is_not_none(th1_window_status_report3, "No WindowStatus report received")
        asserts.assert_equal(th1_window_status_report3["value"], 1, "WindowStatus value is not 1")

        th1_admin_fabric_index_report2 = await self.wait_for_subscription_value(
            th1_admin_fabric_index_queue, expected_value=th1_admin_fabric_index)
        asserts.assert_is_not_none(th1_admin_fabric_index_report2, "No AdminFabricIndex report received")
        asserts.assert_equal(th1_admin_fabric_index_report2.get("value"), th1_admin_fabric_index,
                             "AdminFabricIndex value is not the same as the Fabric Index")

        th1_admin_vendor_id_report2 = await self.wait_for_subscription_value(
            th1_admin_vendor_id_queue, expected_value=th1_admin_fabric_vendor_id)
        asserts.assert_is_not_none(th1_admin_vendor_id_report2, "No AdminVendorId report received")
        asserts.assert_equal(th1_admin_vendor_id_report2.get("value"), th1_admin_fabric_vendor_id,
                             "AdminVendorId value is not the same as the Vendor ID")

        self.step(15)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR1's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR1's fabric
        th2_fabrics = await self.get_fabrics(self.th2)
        th2_admin_fabric_vendor_id = th2_fabrics[1].vendorID
        th2_admin_fabric_index = th2_fabrics[1].fabricIndex

        th2_window_status_report = await self.wait_for_subscription_value(
            th2_window_status_queue, expected_value=1)
        asserts.assert_is_not_none(th2_window_status_report, "No WindowStatus report received")
        asserts.assert_equal(th2_window_status_report["value"], 1, "WindowStatus value is not 1")

        th2_admin_fabric_index_report = await self.wait_for_subscription_value(
            th2_admin_fabric_index_queue, expected_value=th1_admin_fabric_index)
        asserts.assert_is_not_none(th2_admin_fabric_index_report, "No AdminFabricIndex report received")
        asserts.assert_equal(th2_admin_fabric_index_report.get("value"), th1_admin_fabric_index,
                             "AdminFabricIndex value is not the same as the Fabric Index")

        th2_admin_vendor_id_report = await self.wait_for_subscription_value(
            th2_admin_vendor_id_queue, expected_value=th1_admin_fabric_vendor_id)
        asserts.assert_is_not_none(th2_admin_vendor_id_report, "No AdminVendorId report received")
        asserts.assert_equal(th2_admin_vendor_id_report.get("value"), th1_admin_fabric_vendor_id,
                             "AdminVendorId value is not the same as the Vendor ID")

        self.step(16)
        # TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)

        self.step(17)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null
        th1_window_status_report4 = await self.wait_for_subscription_value(
            th1_window_status_queue, expected_value=0)
        asserts.assert_is_not_none(th1_window_status_report4, "No WindowStatus report received")
        asserts.assert_equal(th1_window_status_report4["value"], 0, "WindowStatus value is not 0")

        # Check if the value is a Nullable
        th1_admin_fabric_index_report3 = await self.wait_for_subscription_value(
            th1_admin_fabric_index_queue, expected_value=None)
        value = th1_admin_fabric_index_report3.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminFabricIndex value is not null")

        # Check if the value is a Nullable
        th1_admin_vendor_id_report3 = await self.wait_for_subscription_value(
            th1_admin_vendor_id_queue, expected_value=None)
        value = th1_admin_vendor_id_report3.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminVendorId value is not null")

        self.step(18)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 0, AdminFabricIndex value to be null, AdminVendorId to be null
        th2_window_status_report2 = await self.wait_for_subscription_value(
            th2_window_status_queue, expected_value=0)
        asserts.assert_is_not_none(th2_window_status_report2, "No WindowStatus report received")
        asserts.assert_equal(th2_window_status_report2["value"], 0, "WindowStatus value is not 0")

        th2_admin_fabric_index_report2 = await self.wait_for_subscription_value(
            th2_admin_fabric_index_queue, expected_value=None)
        value = th2_admin_fabric_index_report2.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminFabricIndex value is not null")

        th2_admin_vendor_id_report2 = await self.wait_for_subscription_value(
            th2_admin_vendor_id_queue, expected_value=None)
        value = th2_admin_vendor_id_report2.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminVendorId value is not null")

        self.step(19)
        # TH_CR2 opens a commissioning window on DUT_CE using ECM with commissioning timeout of `max_window_duration`
        await self.th2.OpenCommissioningWindow(nodeid=self.dut_node_id, timeout=max_window_duration, iteration=10000,
                                               discriminator=self.discriminator, option=1)

        self.step(20)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric
        th1_window_status_report5 = await self.wait_for_subscription_value(
            th1_window_status_queue, expected_value=1)
        asserts.assert_is_not_none(th1_window_status_report5, "No WindowStatus report received")
        asserts.assert_equal(th1_window_status_report5["value"], 1, "WindowStatus value is not 1")

        th1_admin_fabric_index_report4 = await self.wait_for_subscription_value(
            th1_admin_fabric_index_queue, expected_value=th2_admin_fabric_index)
        asserts.assert_is_not_none(th1_admin_fabric_index_report4, "No AdminFabricIndex report received")
        asserts.assert_equal(th1_admin_fabric_index_report4.get("value"), th2_admin_fabric_index,
                             "AdminFabricIndex value is not the same as the Fabric Index")

        th1_admin_vendor_id_report3 = await self.wait_for_subscription_value(
            th1_admin_vendor_id_queue, expected_value=th2_admin_fabric_vendor_id)
        asserts.assert_is_not_none(th1_admin_vendor_id_report3, "No AdminVendorId report received")
        asserts.assert_equal(th1_admin_vendor_id_report3.get("value"), th2_admin_fabric_vendor_id,
                             "AdminVendorId value is not the same as the Vendor ID")

        self.step(21)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric
        th2_window_status_report3 = await self.wait_for_subscription_value(
            th2_window_status_queue, expected_value=1)
        asserts.assert_is_not_none(th2_window_status_report3, "No WindowStatus report received")
        asserts.assert_equal(th2_window_status_report3["value"], 1, "WindowStatus value is not 1")

        th2_admin_fabric_index_report2 = await self.wait_for_subscription_value(
            th2_admin_fabric_index_queue, expected_value=th2_admin_fabric_index)
        asserts.assert_is_not_none(th2_admin_fabric_index_report2, "No AdminFabricIndex report received")
        asserts.assert_equal(th2_admin_fabric_index_report2["value"], th2_admin_fabric_index,
                             "AdminFabricIndex value is not the same as the Fabric Index")

        th2_admin_vendor_id_report2 = await self.wait_for_subscription_value(
            th2_admin_vendor_id_queue, expected_value=th2_admin_fabric_vendor_id)
        asserts.assert_is_not_none(th2_admin_vendor_id_report2, "No AdminVendorId report received")
        asserts.assert_equal(th2_admin_vendor_id_report2.get("value"), th2_admin_fabric_vendor_id,
                             "AdminVendorId value is not the same as the Vendor ID")

        self.step(22)
        # TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)

        self.step(23)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0
        # AdminFabricIndex value to be null andAdminVendorId to be null
        th1_window_status_report6 = await self.wait_for_subscription_value(
            th1_window_status_queue, expected_value=0)
        asserts.assert_is_not_none(th1_window_status_report6, "No WindowStatus report received")
        asserts.assert_equal(th1_window_status_report6["value"], 0, "WindowStatus value is not 0")

        th1_admin_fabric_index_report6 = await self.wait_for_subscription_value(
            th1_admin_fabric_index_queue, expected_value=None)
        value = th1_admin_fabric_index_report6.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminFabricIndex value is not null")

        th1_admin_vendor_id_report6 = await self.wait_for_subscription_value(
            th1_admin_vendor_id_queue, expected_value=None)
        value = th1_admin_vendor_id_report6.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminVendorId value is not null")

        self.step(24)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 0
        # AdminFabricIndex value to be null and AdminVendorId to be null
        th2_window_status_report4 = await self.wait_for_subscription_value(
            th2_window_status_queue, expected_value=0)
        asserts.assert_is_not_none(th2_window_status_report4, "No WindowStatus report received")
        asserts.assert_equal(th2_window_status_report4["value"], 0, "WindowStatus value is not 0")

        th2_admin_fabric_index_report3 = await self.wait_for_subscription_value(
            th2_admin_fabric_index_queue, expected_value=None)
        value = th2_admin_fabric_index_report3.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminFabricIndex value is not null")

        th2_admin_vendor_id_report3 = await self.wait_for_subscription_value(
            th2_admin_vendor_id_queue, expected_value=None)
        value = th2_admin_vendor_id_report3.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminVendorId value is not null")

        asserts.assert_equal(th2_window_status_report4["value"], Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kWindowNotOpen,
                             "Commissioning window is expected to be closed, but was found to be open")

        self.step(25)
        # TH_CR2 send an OpenCommissioningWindow command to DUT_CE using ECM with a commissioning timeout of `max_window_duration`
        await self.th2.OpenCommissioningWindow(nodeid=self.dut_node_id, timeout=max_window_duration, iteration=10000,
                                               discriminator=self.discriminator, option=1)

        self.step(26)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric
        th1_window_status_report7 = await self.wait_for_subscription_value(
            th1_window_status_queue, expected_value=1)
        asserts.assert_is_not_none(th1_window_status_report7, "No WindowStatus report received")
        asserts.assert_equal(th1_window_status_report7["value"], 1, "WindowStatus value is not 1")

        th1_admin_fabric_index_report7 = await self.wait_for_subscription_value(
            th1_admin_fabric_index_queue, expected_value=th2_admin_fabric_index)
        asserts.assert_is_not_none(th1_admin_fabric_index_report7, "No AdminFabricIndex report received")
        asserts.assert_equal(th1_admin_fabric_index_report7.get("value"), th2_admin_fabric_index,
                             "AdminFabricIndex value is not the same as the Fabric Index")

        th1_admin_vendor_id_report4 = th1_admin_vendor_id_queue.get(block=True, timeout=10)
        asserts.assert_equal(th1_admin_vendor_id_report4["value"], th2_admin_fabric_vendor_id,
                             "AdminVendorId value is not the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric")

        self.step(27)
        # Verify TH_CR2 receives subscription notifications which show WindowStatus value to be 1
        # AdminFabricIndex value to be the same as the Fabric Index of the Fabrics attribute list entry corresponding to TH_CR2's fabric
        # AdminVendorId to be the same as the Vendor ID field of Fabrics attribute list entry corresponding to TH_CR2's fabric
        th2_window_status_report5 = await self.wait_for_subscription_value(
            th2_window_status_queue, expected_value=1)
        asserts.assert_is_not_none(th2_window_status_report5, "No WindowStatus report received")
        asserts.assert_equal(th2_window_status_report5["value"], 1, "WindowStatus value is not 1")

        th2_admin_fabric_index_report3 = await self.wait_for_subscription_value(
            th2_admin_fabric_index_queue, expected_value=th2_admin_fabric_index)
        asserts.assert_is_not_none(th2_admin_fabric_index_report3, "No AdminFabricIndex report received")
        asserts.assert_equal(th2_admin_fabric_index_report3.get("value"), th2_admin_fabric_index,
                             "AdminFabricIndex value is not the same as the Fabric Index")

        th2_admin_vendor_id_report3 = await self.wait_for_subscription_value(
            th2_admin_vendor_id_queue, expected_value=th2_admin_fabric_vendor_id)
        asserts.assert_is_not_none(th2_admin_vendor_id_report3, "No AdminVendorId report received")
        asserts.assert_equal(th2_admin_vendor_id_report3.get("value"), th2_admin_fabric_vendor_id,
                             "AdminVendorId value is not the same as the Vendor ID")

        self.step(28)
        # Before expiration of `max_window_duration` set in step 24,
        # TH_CR1 sends _RemoveFabric_ command to DUT_CE with FabricIndex set to the fabric index of TH_CR2's fabric
        th2_idx = await self.th2.ReadAttribute(nodeid=self.dut_node_id, attributes=[(0, Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)])
        outer_key = list(th2_idx.keys())[0]
        inner_key = list(th2_idx[outer_key].keys())[0]
        attribute_key = list(th2_idx[outer_key][inner_key].keys())[1]
        removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(th2_idx[outer_key][inner_key][attribute_key])
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)

        self.step(29)
        # Verify TH_CR1 receives subscription notifications which show AdminFabricIndex value to be null
        th1_admin_fabric_index_report8 = await self.wait_for_subscription_value(
            th1_admin_fabric_index_queue, expected_value=None)
        value = th1_admin_fabric_index_report8.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminFabricIndex value is not null")

        self.step(30)
        # TH_CR1 reads WindowStatus attribute from DUT_CE
        # verify the value to be 1 indicating the window is still open
        AC_cluster = Clusters.AdministratorCommissioning
        window_status = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=AC_cluster, attribute=AC_cluster.Attributes.WindowStatus)
        asserts.assert_equal(window_status, Clusters.AdministratorCommissioning.Enums.CommissioningWindowStatusEnum.kEnhancedWindowOpen,
                             "Commissioning window is expected to be closed, but was found to be open")

        self.step(31)
        # TH_CR1 reads AdminVendorID attribute from DUT_CE
        admin_vendor_id = await self.read_single_attribute_check_success(dev_ctrl=self.th1, fabric_filtered=False, endpoint=0, cluster=AC_cluster, attribute=AC_cluster.Attributes.AdminVendorId)
        self.print_step("AdminVendorId", admin_vendor_id)
        asserts.assert_equal(admin_vendor_id, th2_admin_fabric_vendor_id,
                             "Commissioning window is expected to be closed, but was found to be open")

        self.step(32)
        # TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)

        self.step(33)
        # Verify TH_CR1 receives subscription notifications which show WindowStatus value to be 0, AdminVendorId to be null
        th1_window_status_report9 = await self.wait_for_subscription_value(
            th1_window_status_queue, expected_value=0)
        asserts.assert_is_not_none(th1_window_status_report9, "No WindowStatus report received")
        asserts.assert_equal(th1_window_status_report9.get("value"), 0, "WindowStatus value is not 0")

        th1_admin_vendor_id_report6 = await self.wait_for_subscription_value(
            th1_admin_vendor_id_queue, expected_value=None)
        value = th1_admin_vendor_id_report6.get("value")
        is_null = (value is None or str(type(value)).find('chip.clusters.Types.Nullable') >= 0)
        asserts.assert_true(is_null, "AdminVendorId value is not null")


if __name__ == "__main__":
    default_matter_test_main()

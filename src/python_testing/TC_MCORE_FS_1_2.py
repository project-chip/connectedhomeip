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

# TODO: add to CI. See https://github.com/project-chip/connectedhomeip/issues/34676
# for details about the block below.
#

import base64
import logging
import queue
import time

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from TC_SC_3_6 import AttributeChangeAccumulator


class TC_MCORE_FS_1_2(MatterBaseTest):
    def steps_TC_MCORE_FS_1_2(self) -> list[TestStep]:
        steps = [TestStep(1, "TH_FSA subscribes to all the Bridged Device Basic Information clusters provided by DUT_FSA to identify the presence of a Bridged Node endpoint with a UniqueID matching the UniqueID provided by the BasicInformationCluster of the TH_SED_DUT."),
                 TestStep(2, "TH_FSA initiates commissioning of TH_SED_DUT by sending the OpenCommissioningWindow command to the Administrator Commissioning Cluster on the endpoint with the uniqueID matching that of TH_SED_DUT."),
                 TestStep(3, "TH_FSA completes commissioning of TH_SED_DUT using the Enhanced Commissioning Method."),
                 TestStep(4, "Commission TH_SED_L onto DUT_FSA’s fabric using the manufacturer specified mechanism."),
                 TestStep(5, "TH_FSA waits for subscription report from a the Bridged Device Basic Information clusters provided by DUT_FSA to identify the presence of a Bridged Node endpoint with a UniqueID matching the UniqueID provided by the BasicInformationCluster of the TH_SED_L."),
                 TestStep(6, "TH_FSA initiates commissions of TH_SED_L by sending the OpenCommissioningWindow command to the Administrator Commissioning Cluster on the endpoint with the uniqueID matching that of TH_SED_L."),
                 TestStep(7, "TH_FSA completes commissioning of TH_SED_L using the Enhanced Commissioning Method.")]
        return steps

    @property
    def default_timeout(self) -> int:
        return self.user_params.get("report_waiting_timeout_delay_sec", 10)*2 + 60

    @async_test_body
    async def test_TC_MCORE_FS_1_2(self):
        self.is_ci = self.check_pics('PICS_SDK_CI_ONLY')
        min_report_interval_sec = self.user_params.get("min_report_interval_sec", 0)
        max_report_interval_sec = self.user_params.get("max_report_interval_sec", 2)
        report_waiting_timeout_delay_sec = self.user_params.get("report_waiting_timeout_delay_sec", 10)

        self.step(1)

        # Subscribe to the UniqueIDs
        unique_id_queue = queue.Queue()
        subscription_contents = [
            (Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID)  # On all endpoints
        ]
        sub = await self.default_controller.ReadAttribute(
            nodeid=self.dut_node_id,
            attributes=subscription_contents,
            reportInterval=(min_report_interval_sec, max_report_interval_sec),
            keepSubscriptions=False
        )
        attribute_handler = AttributeChangeAccumulator(
            name=self.default_controller.name, expected_attribute=Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID, output=unique_id_queue)
        sub.SetAttributeUpdateCallback(attribute_handler)

        logging.info("Waiting for First BridgedDeviceBasicInformation.")
        start_time = time.time()
        elapsed = 0
        time_remaining = report_waiting_timeout_delay_sec

        th_sed_dut_bdbi_endpoint = -1
        th_sed_dut_unique_id = -1

        while time_remaining > 0 and th_sed_dut_bdbi_endpoint < 0:
            try:
                item = unique_id_queue.get(block=True, timeout=time_remaining)
                endpoint, attribute, value = item['endpoint'], item['attribute'], item['value']

                # Record arrival of an expected subscription change when seen
                if attribute == Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID:
                    th_sed_dut_bdbi_endpoint = endpoint
                    th_sed_dut_unique_id = value

            except queue.Empty:
                # No error, we update timeouts and keep going
                pass

            elapsed = time.time() - start_time
            time_remaining = report_waiting_timeout_delay_sec - elapsed

        asserts.assert_greater(th_sed_dut_bdbi_endpoint, 0, "Failed to find any BDBI instances with UniqueID present.")
        logging.info("Found BDBI with UniqueID (%d) on endpoint %d." % th_sed_dut_unique_id, th_sed_dut_bdbi_endpoint)

        self.step(2)

        self.sync_passcode = 20202024
        self.th_sed_dut_discriminator = 2222
        cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(commissioningTimeout=3*60,
                                                                                   PAKEPasscodeVerifier=b"+w1qZQR05Zn0bc2LDyNaDAhsrhDS5iRHPTN10+EmNx8E2OpIPC4SjWRDQVOgqcbnXdYMlpiZ168xLBqn1fx9659gGK/7f9Yc6GxpoJH8kwAUYAYyLGsYeEBt1kL6kpXjgA==",
                                                                                   discriminator=self.th_sed_dut_discriminator,
                                                                                   iterations=10000, salt=base64.b64encode(bytes('SaltyMcSalterson', 'utf-8')))
        await self.send_single_cmd(cmd, endpoint=th_sed_dut_bdbi_endpoint, timedRequestTimeoutMs=5000)

        logging.info("Commissioning Window open for TH_SED_DUT.")

        self.step(3)

        self.th_sed_dut_nodeid = 1111
        await self.TH_server_controller.CommissionOnNetwork(nodeId=self.th_sed_dut_nodeid, setupPinCode=self.sync_passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.th_sed_dut_discriminator)
        logging.info("Commissioning TH_SED_DUT complete")

        self.step(4)
        if not self.is_ci:
            self.wait_for_use_input(
                "Commission TH_SED_DUT onto DUT_FSA’s fabric using the manufacturer specified mechanism. (ensure Synchronization is enabled.)")
        else:
            logging.info("Stopping after step 3 while running in CI to avoid manual steps.")
            return

        self.step(5)

        th_sed_later_bdbi_endpoint = -1
        th_sed_later_unique_id = -1
        logging.info("Waiting for Second BridgedDeviceBasicInformation.")
        start_time = time.time()
        elapsed = 0
        time_remaining = report_waiting_timeout_delay_sec

        while time_remaining > 0 and th_sed_later_bdbi_endpoint < 0:
            try:
                item = unique_id_queue.get(block=True, timeout=time_remaining)
                endpoint, attribute, value = item['endpoint'], item['attribute'], item['value']

                # Record arrival of an expected subscription change when seen
                if attribute == Clusters.BridgedDeviceBasicInformation.Attributes.UniqueID and endpoint != th_sed_dut_bdbi_endpoint and th_sed_later_unique_id != th_sed_dut_unique_id:
                    th_sed_later_bdbi_endpoint = endpoint
                    th_sed_later_unique_id = value

            except queue.Empty:
                # No error, we update timeouts and keep going
                pass

            elapsed = time.time() - start_time
            time_remaining = report_waiting_timeout_delay_sec - elapsed

        asserts.assert_greater(th_sed_later_bdbi_endpoint, 0, "Failed to find any BDBI instances with UniqueID present.")
        logging.info("Found another BDBI with UniqueID (%d) on endpoint %d." % th_sed_later_unique_id, th_sed_later_bdbi_endpoint)

        self.step(6)

        self.th_sed_later_discriminator = 3333
        # min commissioning timeout is 3*60 seconds, so use that even though the command said 30.
        cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(commissioningTimeout=3*60,
                                                                                   PAKEPasscodeVerifier=b"+w1qZQR05Zn0bc2LDyNaDAhsrhDS5iRHPTN10+EmNx8E2OpIPC4SjWRDQVOgqcbnXdYMlpiZ168xLBqn1fx9659gGK/7f9Yc6GxpoJH8kwAUYAYyLGsYeEBt1kL6kpXjgA==",
                                                                                   discriminator=self.th_sed_later_discriminator,
                                                                                   iterations=10000, salt=base64.b64encode(bytes('SaltyMcSalterson', 'utf-8')))
        await self.send_single_cmd(cmd, endpoint=th_sed_later_bdbi_endpoint, timedRequestTimeoutMs=5000)

        logging.info("Commissioning Window open for TH_SED_L.")

        self.step(7)

        self.th_sed_later_nodeid = 2222
        await self.TH_server_controller.CommissionOnNetwork(nodeId=self.th_sed_later_nodeid, setupPinCode=self.sync_passcode, filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=self.th_sed_later_discriminator)
        logging.info("Commissioning TH_SED_L complete")


if __name__ == "__main__":
    default_matter_test_main()

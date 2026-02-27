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

import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class TC_TBRM_2_6(MatterBaseTest):

    def desc_TC_TBRM_2_6(self) -> str:
        """Returns a description of this test"""
        return "This test case verifies that the Network Commissioning Cluster will reflect the same underlying " \
               "network configuration when Thread Border Router endpoint that supports also Secondary Network " \
               "Interface device type is used to configure Thread network."

    def steps_TC_TBRM_2_6(self) -> list[TestStep]:
        return [
            TestStep(1,
                     "TH commissions DUT, putting device on operational network using Network Commissioning cluster "
                     "on primary network interface.",
                     is_commissioning=False),
            TestStep(2, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 120",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(3,
                     "In a CASE session, TH sends command SetActiveDatasetRequest to DUT with ActiveDataset: "
                     "PIXIT.TBRM.THREAD_ACTIVE_DATASET",
                     "Verify DUT responds w/ status SUCCESS(0x00)."),
            TestStep(4, "TH read InterfaceEnabled attribute from DUT on Thread Border Router Endpoint.",
                     "Verify that InterfaceEnabled attribute value is True"),
            TestStep(5, "TH sends CommissioningComplete command to the DUT",
                     "Verify that DUT sends CommissioningCompleteResponse command to the TH with errorCode: 0 ("
                     "Success status)"),
            TestStep(6, "TH read LastNetworkID attribute from DUT on Thread Border Router Endpoint.",
                     "LastNetworkID received from DUT has value PIXIT.TBRM.THREAD_EXTENDED_PANID."),
            TestStep(7, "TH read MaxNetworks attribute from DUT on Thread Border Router Endpoint.",
                     "Verify that MaxNetwork attribute value is equal to 1")
        ]

    def pics_TC_TBRM_2_6(self) -> list[str]:
        """ This function returns a list of PICS for this test case that must be True for the test to be run"""
        return [
            "TBRM.S",
        ]

    @staticmethod
    def extract_active_dataset_tlvs(b_active_dataset: bytes):
        nwk_dataset_tlvs = {
            # Active Timestamp TLV
            14: {
                "name": "timestamp",
                "value": b""
            },
            # Channel Mask TLV
            53: {
                "name": "channel_mask",
                "value": b""
            },
            # Mesh Prefix TLV
            7: {
                "name": "mesh_prefix",
                "value": b""
            },
            # PSKc TLV
            4: {
                "name": "pskc",
                "value": b""
            },
            # Security Policy TLV
            12: {
                "name": "security_policy",
                "value": b""
            },
            # Channel TLV
            0: {
                "name": "channel",
                "value": b""
            },
            # PAN ID TLV
            1: {
                "name": "panid",
                "value": b""
            },
            # Extended PAN ID TLV
            2: {
                "name": "expanid",
                "value": b""
            },
            # NWK Name TLV
            3: {
                "name": "nwkname",
                "value": b""
            },
            # NWK KEY TLV
            5: {
                "name": "nwkkey",
                "value": b""
            },
        }
        _index = 0
        while _index < len(b_active_dataset):
            tlv_type = b_active_dataset[_index]
            tlv_length = b_active_dataset[_index+1]
            nwk_dataset_tlvs[tlv_type]["value"] = b_active_dataset[_index+2:_index+2+tlv_length]
            _index += tlv_length + 2
        return nwk_dataset_tlvs

    @async_test_body
    async def test_TC_TBRM_2_6(self):

        self.step(1)

        test_harness = self.default_controller
        tbrm_endpoint = self.matter_test_config.global_test_params['PIXIT.TBRM.ENDPOINT']
        # Check if DUT has Secondary Network Interface and Thread Border Router Management Support on the same endpoint
        rsp = await test_harness.ReadAttribute(self.dut_node_id, [(tbrm_endpoint, Clusters.NetworkCommissioning.Attributes.MaxNetworks)])
        logging.info(f" Response: {rsp}")
        if self.matter_test_config.commissioning_method == "ble-thread" or rsp[tbrm_endpoint][Clusters.NetworkCommissioning][Clusters.NetworkCommissioning.Attributes.MaxNetworks] != 1:
            logging.info("Device doesn't support SNI and TBRM on PIXIT.TBRM.ENDPOINT or ble-thread commissioning was used. SKIP remaining steps!")
            self.skip_all_remaining_steps(starting_step_number=2)
        else:
            logging.info("Device support SNI and TBRM on PIXIT.TBRM.ENDPOINT. Contiune test ...")

        self.step(2)

        # ArmFailSafe in order to set Active Dataset and connect to Thread network
        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=120, breadcrumb=1)
        rsp = await test_harness.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
        logging.info(f" Response: {rsp}")

        self.step(3)

        thread_active_dataset = self.matter_test_config.global_test_params['PIXIT.TBRM.THREAD_ACTIVE_DATASET']
        cmd = Clusters.ThreadBorderRouterManagement.Commands.SetActiveDatasetRequest(
            activeDataset=thread_active_dataset, breadcrumb=1)
        await test_harness.SendCommand(nodeid=self.dut_node_id, endpoint=tbrm_endpoint, payload=cmd)

        self.step(4)

        rsp = await test_harness.ReadAttribute(self.dut_node_id, [(tbrm_endpoint, Clusters.NetworkCommissioning.Attributes.InterfaceEnabled)])
        logging.info(f" Response: {rsp}")
        asserts.assert_equal(
            True, rsp[tbrm_endpoint][Clusters.NetworkCommissioning][Clusters.NetworkCommissioning.Attributes.InterfaceEnabled],
            "NetworkCommissioning.Attributes.InterfaceEnabled != True")

        self.step(5)

        cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
        rsp = await test_harness.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
        logging.info(f" Response: {rsp}")

        self.step(6)

        nwk_dataset = self.extract_active_dataset_tlvs(b_active_dataset=thread_active_dataset)
        rsp = await test_harness.ReadAttribute(self.dut_node_id, [(tbrm_endpoint, Clusters.NetworkCommissioning.Attributes.LastNetworkID)])
        logging.info(f" Response: {rsp}")
        asserts.assert_equal(
            nwk_dataset[2]["value"], rsp[tbrm_endpoint][Clusters.NetworkCommissioning][Clusters.NetworkCommissioning.Attributes.LastNetworkID],
            "NetworkCommissioning Cluster LastNetworkID Attribute dose not match TBRM Cluster Extended PanID Attribute(extracted from dataset)")

        self.step(7)

        rsp = await test_harness.ReadAttribute(self.dut_node_id, [(tbrm_endpoint, Clusters.NetworkCommissioning.Attributes.MaxNetworks)])
        logging.info(f" Response: {rsp}")
        asserts.assert_equal(
            1, rsp[tbrm_endpoint][Clusters.NetworkCommissioning][Clusters.NetworkCommissioning.Attributes.MaxNetworks],
            "NetworkCommissioning.Attributes.MaxNetworks Attribute != 1")


if __name__ == "__main__":
    default_matter_test_main()

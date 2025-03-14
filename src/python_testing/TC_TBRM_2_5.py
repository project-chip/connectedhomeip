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


class TC_TBRM_2_5(MatterBaseTest):

    def desc_TC_TBRM_2_5(self) -> str:
        """Returns a description of this test"""
        return "This test case verifies that the Thread Border Router endpoint that supports also Secondary Network " \
               "Interface device type will reflect the same underlying network configuration when Network " \
               "Commissioning Cluster is used to configure Thread network."

    def steps_TC_TBRM_2_5(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning", is_commissioning=False),
            TestStep(2, "TH use Network Commissioning Cluster to configure dataset to PIXIT.TBRM.THREAD_ACTIVE_DATASET",
                     "DUT connect to the Thread network using the dataset provided in the Network Commissioning "
                     "Cluster."),
            TestStep(3,
                     "In a CASE session, TH sends command GetActiveDatasetRequest to DUT on Thread Border Router "
                     "Endpoint",
                     "Verify DUT responds w/ status SUCCESS(0x00) and the Active Dataset received from DUT has value "
                     "equal to PIXIT.TBRM.THREAD_ACTIVE_DATASET"),
            TestStep(4, "TH read MaxNetworks attribute from DUT on Thread Border Router Endpoint.",
                     "Verify that MaxNetwork attribute value is equal to 1"),
            TestStep(5, "TH read InterfaceEnabled attribute from DUT on Thread Border Router Endpoint",
                     "Verify that InterfaceEnabled attribute value is True")
        ]

    def pics_TC_TBRM_2_5(self) -> list[str]:
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
            tlv_lenght = b_active_dataset[_index+1]
            nwk_dataset_tlvs[tlv_type]["value"] = b_active_dataset[_index+2:_index+2+tlv_lenght]
            _index += tlv_lenght + 2
        return nwk_dataset_tlvs

    @async_test_body
    async def test_TC_TBRM_2_5(self):

        self.step(1)

        test_harness = self.default_controller
        tbrm_endpoint = self.matter_test_config.global_test_params['PIXIT.TBRM.ENDPOINT']
        # Check if DUT has Secondary Network Interface and Thread Border Router Management Support on the same endpoint
        rsp = await test_harness.ReadAttribute(self.dut_node_id, [(tbrm_endpoint, Clusters.NetworkCommissioning.Attributes.MaxNetworks)])
        logging.info(f" Response: {rsp}")
        if rsp[tbrm_endpoint][Clusters.NetworkCommissioning][Clusters.NetworkCommissioning.Attributes.MaxNetworks] > 1:
            logging.info("Device doesn't support SNI and TBRM on PIXIT.TBRM.ENDPOINT. SKIP remaining steps!")
            self.skip_all_remaining_steps(starting_step_number=2)
        else:
            logging.info("Device support SNI and TBRM on PIXIT.TBRM.ENDPOINT. Contiune test ...")

        try:
            thread_active_dataset = self.matter_test_config.global_test_params['PIXIT.TBRM.THREAD_ACTIVE_DATASET']
        except KeyError:
            thread_active_dataset = self.matter_test_config.global_test_params['meta_config']['thread_operational_dataset']
        assert thread_active_dataset is not None

        self.step(2)

        if self.matter_test_config.commissioning_method != "ble-thread":
            logging.info("Use NetworkCommissioning to configure Thread network")
            # ArmFailSafe in order to set Active Dataset and connect to Thread network
            cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=120, breadcrumb=1)
            rsp = await test_harness.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
            logging.info(f" Response: {rsp}")
            # Use NetworkCommissioning cluster to write the Thread Operational Dataset
            cmd = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
                operationalDataset=thread_active_dataset, breadcrumb=1)
            rsp = await test_harness.SendCommand(nodeid=self.dut_node_id, endpoint=tbrm_endpoint, payload=cmd)
            logging.info(f" Response: {rsp}")
            # Connect to Thread Network
            nwk_dataset = self.extract_active_dataset_tlvs(b_active_dataset=thread_active_dataset)
            cmd = Clusters.NetworkCommissioning.Commands.ConnectNetwork(networkID=nwk_dataset[2]["value"], breadcrumb=1)
            rsp = await test_harness.SendCommand(nodeid=self.dut_node_id, endpoint=tbrm_endpoint, payload=cmd)
            logging.info(f" Response: {rsp}")
            # Send Commissioning Complete command
            cmd = Clusters.GeneralCommissioning.Commands.CommissioningComplete()
            rsp = await test_harness.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd)
            logging.info(f" Response: {rsp}")
        else:
            logging.info("SKIP Step 2 - Thread network was already configured.")

        self.step(3)

        cmd = Clusters.ThreadBorderRouterManagement.Commands.GetActiveDatasetRequest()
        rsp = await test_harness.SendCommand(nodeid=self.dut_node_id, endpoint=tbrm_endpoint, payload=cmd)
        logging.info(f" Response: {rsp}")
        asserts.assert_equal(thread_active_dataset, rsp.dataset,
                             "Dataset configured through NetwrokCommissioning != Dataset extracted from TBRM")

        self.step(4)

        rsp = await test_harness.ReadAttribute(self.dut_node_id, [(tbrm_endpoint, Clusters.NetworkCommissioning.Attributes.MaxNetworks)])
        logging.info(f" Response: {rsp}")
        asserts.assert_equal(
            1, rsp[tbrm_endpoint][Clusters.NetworkCommissioning][Clusters.NetworkCommissioning.Attributes.MaxNetworks],
            "NetworkCommissioning.Attributes.MaxNetworks Attribute != 1")

        self.step(5)

        rsp = await test_harness.ReadAttribute(self.dut_node_id, [(tbrm_endpoint, Clusters.NetworkCommissioning.Attributes.InterfaceEnabled)])
        logging.info(f" Response: {rsp}")
        asserts.assert_equal(
            True, rsp[tbrm_endpoint][Clusters.NetworkCommissioning][Clusters.NetworkCommissioning.Attributes.InterfaceEnabled],
            "NClusters.NetworkCommissioning.Attributes.InterfaceEnabled != True")


if __name__ == "__main__":
    default_matter_test_main()

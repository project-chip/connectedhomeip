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
#   run:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 3840 --passcode 20202021 --KVS kvs1
#     script-args: >
#       --commissioning-method ble-thread
#       --discriminator 3840
#       --passcode 20202021
#       --thread-dataset-hex <DATASET_HEX>
#       --int-arg PIXIT.CNET.ENDPOINT_THREAD:0
#       --storage-path admin_storage.json
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: false
#     quiet: true
# === END CI TEST ARGUMENTS ===


import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
from matter_testing_infrastructure.chip.testing.matter_asserts import assert_valid_uint8

import logging
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

PIXIT_CNET_THREAD_2ND_OPERATIONALDATASET = bytes.fromhex("1111111122222222")


class TC_CNET_4_16(MatterBaseTest):

    def steps_TC_CNET_4_16(self):
        return [TestStep("precondition", "TH is commissioned", is_commissioning=True),
                TestStep(1, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
                TestStep(2, 'TH sends RemoveNetwork Command to the DUT with NetworkID field set to the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET, which does not match the commissioned network, and Breadcrumb field set to 1'),
                TestStep(3, 'TH sends ConnectNetwork Command to the DUT with NetworkID value as the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET, which does not match the commissioned network, and Breadcrumb field set to 1'),
                ]

    def desc_TC_CNET_4_16(self):
        return '[TC-CNET-4.16] [Thread] NetworkIDNotFound returned in LastNetworkingStatus field validation [DUT-Server]'

    @async_test_body
    async def test_TC_CNET_4_16(self):

        asserts.assert_true("PIXIT.CNET.ENDPOINT_THREAD" in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.ENDPOINT_THREAD must be included in the command line with "
                            "the --int-arg flag as PIXIT.CNET.ENDPOINT_THREAD:<endpoint>")
        endpoint = self.matter_test_config.global_test_params["PIXIT.CNET.ENDPOINT_THREAD"]
        threadFirstOperationalDataset = self.matter_test_config.thread_operational_dataset

        cnet = Clusters.NetworkCommissioning
        attr = cnet.Attributes

        # Commissioning is already done
        self.step("precondition")

        # Precondition 1: DUT has a Network Commissioning cluster on endpoint PIXIT.CNET.ENDPOINT_THREAD with FeatureMap attribute of 2
        feature_map = await self.read_single_attribute_check_success(cluster=cnet, endpoint=endpoint, attribute=attr.FeatureMap)
        if not (feature_map & cnet.Bitmaps.Feature.kThreadNetworkInterface):
            logging.info('Device does not support Thread on endpoint, skipping remaining steps')
            self.skip_all_remaining_steps(1)
            return

        # Precondition 2: DUT is commissioned on PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET
        # Precondition 3: TH can communicate with the DUT on PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET
        networkID = await self.read_single_attribute_check_success(cluster=cnet, endpoint=endpoint, attribute=attr.LastNetworkID)
        logger.info(f" --- NetworkID: {networkID.hex()}")
        asserts.assert_in(networkID.hex(), threadFirstOperationalDataset.hex(),
                          f"NetworkID: {networkID.hex()} not in {threadFirstOperationalDataset.hex()}")

        # Precondition 4: DUT MaxNetworks attribute value is at least 1 and is saved as 'MaxNetworksValue' for future use
        maxNetworksValue = 0
        maxNetworksValue = await self.read_single_attribute_check_success(cluster=cnet, endpoint=endpoint, attribute=attr.MaxNetworks)
        logger.info(f" --- maxNetworksValue: {maxNetworksValue}")
        assert_valid_uint8(maxNetworksValue, "MaxNetworksValue range")
        asserts.assert_greater_equal(maxNetworksValue, 1, "MaxNetworksValue not greater or equal to 1")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)

        cmd = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        res = await self.send_single_cmd(cmd=cmd)
        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_equal(
            res.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "ArmFailSafeResponse error code is not OK.",
        )

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET,
        # which does not match the commissioned network, and Breadcrumb field set to 1
        self.step(2)

        networkList = await self.read_single_attribute_check_success(cluster=cnet, endpoint=endpoint, attribute=attr.Networks)
        if len(networkList) != 0:
            cmd = cnet.Commands.RemoveNetwork(networkID=PIXIT_CNET_THREAD_2ND_OPERATIONALDATASET, breadcrumb=1)
            res = await self.send_single_cmd(cmd=cmd)
            logger.info(f" --- NetworkingStatus on RemoveNetwork: {res.networkingStatus}")
            # Verify that DUT sends NetworkConfigResponse command to the TH1 with NetworkingStatus field set to NetworkIDNotFound
            assert isinstance(res, cnet.Commands.NetworkConfigResponse)
            asserts.assert_equal(res.networkingStatus,
                                 cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
                                 f"Expected kNetworkIDNotFound but got: {res.networkingStatus}")
        else:
            asserts.fail(f"NetworkList is Empty")

        # TH sends ConnectNetwork Command to the DUT with NetworkID value as the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET,
        # which does not match the commissioned network, and Breadcrumb field set to 1
        self.step(3)

        cmd = cnet.Commands.ConnectNetwork(networkID=PIXIT_CNET_THREAD_2ND_OPERATIONALDATASET, breadcrumb=1)
        res = await self.send_single_cmd(cmd=cmd)
        logger.info(f" --- NetworkingStatus on ConnectNetwork: {res.networkingStatus}")
        # Verify that DUT sends ConnectNetworkResponse command to the TH1 with NetworkingStatus field set to NetworkIDNotFound
        assert isinstance(res, cnet.Commands.ConnectNetworkResponse)
        asserts.assert_equal(res.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
                             f"Expected NetworkIDNotFound but got {res.networkingStatus}")


if __name__ == "__main__":
    default_matter_test_main()

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
#   run:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 3840 --passcode 20202021 --KVS kvs1
#     script-args: >
#       --commissioning-method ble-thread
#       --discriminator 3840
#       --passcode 20202021
#       --thread-dataset-hex <DATASET_HEX>
#       --string-arg PIXIT_CNET_THREAD_2ND_OPERATIONALDATASET:"1111111122222222"
#       -- endpoint <ENDPOINT>
#       --storage-path admin_storage.json
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: false
#     quiet: true

import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches
from matter_testing_infrastructure.chip.testing.matter_asserts import assert_valid_uint8
from mobly import asserts

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)


class TC_CNET_4_16(MatterBaseTest):

    def steps_TC_CNET_4_16(self):
        return [TestStep("precondition", "TH is commissioned", is_commissioning=True),
                TestStep(1, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
                TestStep(2, 'TH sends RemoveNetwork Command to the DUT with NetworkID field set to the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET, which does not match the commissioned network, and Breadcrumb field set to 1'),
                TestStep(3, 'TH sends ConnectNetwork Command to the DUT with NetworkID value as the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET, which does not match the commissioned network, and Breadcrumb field set to 1'),
                ]

    def desc_TC_CNET_4_16(self):
        return '[TC-CNET-4.16] [Thread] NetworkIDNotFound returned in LastNetworkingStatus field validation [DUT-Server]'

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface))
    async def test_TC_CNET_4_16(self):

        asserts.assert_true("PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET" in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET must be included in the command line with "
                            "the --string-arg flag as PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET:<operational_dataset>")

        cnet = Clusters.NetworkCommissioning
        attr = cnet.Attributes
        thread_1st = self.matter_test_config.thread_operational_dataset
        thread_2nd_operational_dataset = self.matter_test_config.global_test_params[
            "PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET"]
        thread_2nd = bytes.fromhex(thread_2nd_operational_dataset)

        # Commissioning is already done
        self.step("precondition")

        # Precondition 1: DUT is commissioned on PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET
        # Precondition 2: TH can communicate with the DUT on PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET
        networkID = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.LastNetworkID)
        logger.info(f" --- NetworkID: {networkID.hex()}")
        asserts.assert_in(networkID.hex(), thread_1st.hex(),
                          f"NetworkID: {networkID.hex()} not in {thread_1st.hex()}")

        # Precondition 3: DUT MaxNetworks attribute value is at least 1 and is saved as 'MaxNetworksValue' for future use
        maxNetworksValue = 0
        maxNetworksValue = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.MaxNetworks)
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

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to
        # the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET,
        # which does not match the commissioned network, and Breadcrumb field set to 1
        self.step(2)

        networkList = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.Networks)
        if len(networkList) != 0:
            cmd = cnet.Commands.RemoveNetwork(networkID=thread_2nd, breadcrumb=1)
            res = await self.send_single_cmd(cmd=cmd)
            logger.info(f" --- NetworkingStatus on RemoveNetwork: {res.networkingStatus}")
            # Verify that DUT sends NetworkConfigResponse command to the TH1 with NetworkingStatus field set to NetworkIDNotFound
            asserts.assert_true(isinstance(res, cnet.Commands.NetworkConfigResponse),
                                f"{res} must be of type NetworkConfigResponse")
            asserts.assert_equal(res.networkingStatus,
                                 cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
                                 f"Expected kNetworkIDNotFound but got: {res.networkingStatus}")
        else:
            asserts.fail(f"NetworkList is Empty")

        # TH sends ConnectNetwork Command to the DUT with NetworkID value as the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET,
        # which does not match the commissioned network, and Breadcrumb field set to 1
        self.step(3)

        cmd = cnet.Commands.ConnectNetwork(networkID=thread_2nd, breadcrumb=1)
        res = await self.send_single_cmd(cmd=cmd)
        logger.info(f" --- NetworkingStatus on ConnectNetwork: {res.networkingStatus}")
        # Verify that DUT sends ConnectNetworkResponse command to the TH1 with NetworkingStatus field set to NetworkIDNotFound
        asserts.assert_true(isinstance(res, cnet.Commands.ConnectNetworkResponse), f"{res} must be of type ConnectNetworkResponse")
        asserts.assert_equal(res.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
                             f"Expected NetworkIDNotFound but got {res.networkingStatus}")


if __name__ == "__main__":
    default_matter_test_main()

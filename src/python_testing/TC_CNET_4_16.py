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
#   run:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --passcode 20202021 --KVS kvs1
#     script-args: >
#       --storage-path admin_storage.json
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --int-arg: PIXIT.CNET.ENDPOINT_THREAD:0
#       --commissioning-method ble-thread
#       --thread-dataset-hex <DATASET_HEX>
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: false
#     quiet: true

import logging

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches, run_if_endpoint_matches, has_feature
from mobly import asserts

from chip import ChipDeviceCtrl
from matter_testing_infrastructure.chip.testing.matter_asserts import assert_valid_uint8

logger = logging.getLogger('NetworkCommissioning')
logger.setLevel(logging.INFO)

# To be replaced with --thread-dataset-hex <DATASET_HEX> for the CLI
TEST_THREAD_NETWORK_DATASET_TLVS = [bytes.fromhex("0e080000000000010000" +
                                                  "000300000c" +
                                                  "35060004001fffe0" +
                                                  "0208fedcba9876543210" +
                                                  "0708fd00000000001234" +
                                                  "0510ffeeddccbbaa99887766554433221100" +
                                                  "030e54657374696e674e6574776f726b" +
                                                  "0102d252" +
                                                  "041081cb3b2efa781cc778397497ff520fa50c0302a0ff"),
                                    # End of first TLV
                                    bytes.fromhex("0e080000000000010000" +
                                                  "000300000c" +
                                                  "35060004001fffe0" +
                                                  "02081111111122222222" +
                                                  "0708fd00000000001234" +
                                                  "0510ffeeddccbbaa99887766554433221100" +
                                                  "030e54657374696e674e6574776f726b" +
                                                  "0102d252" +
                                                  "041081cb3b2efa781cc778397497ff520fa50c0302a0ff"),
                                    # End of second TLV
                                    ]
# Network id, for the thread network, current a const value, will be changed to XPANID of the thread network.
# TEST_THREAD_NETWORK_IDS = [
#     bytes.fromhex("fedcba9876543210"),
# ]
# THREAD_NETWORK_FEATURE_MAP = 2
PIXIT_CNET_THREAD_1ST_OPERATIONALDATASET = bytes.fromhex("fedcba9876543210")
PIXIT_CNET_THREAD_2ND_OPERATIONALDATASET = bytes.fromhex("1111111122222222")


class TC_CNET_4_16(MatterBaseTest):
    def steps_TC_CNET_4_16(self):
        return [TestStep("precondition", "TH is commissioned", is_commissioning=True),
                TestStep(1, 'TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900'),
                TestStep(2, 'TH sends RemoveNetwork Command to the DUT with NetworkID field set to the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET, which does not match the commissioned network, and Breadcrumb field set to 1'),
                TestStep(3, 'TH sends ConnectNetwork Command to the DUT with NetworkID value as the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET, which does not match the commissioned network, and Breadcrumb field set to 1'),
                ]

    def desc_TC_CNET_4_16(self):
        return '[TC-CNET-4.16] [Thread] NetworkIDNotFound value as LastNetworkingStatus argument validation [DUT-Server]'

    def pics_TC_CNET_4_16(self):
        return ['CNET.S']

    # @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kThreadNetworkInterface))
    @async_test_body
    async def test_TC_CNET_4_16(self):

        asserts.assert_true('PIXIT.CNET.ENDPOINT_THREAD' in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.ENDPOINT_THREAD must be included on the command line in "
                            "the --int-arg flag as PIXIT.CNET.ENDPOINT_THREAD:<endpoint>")
        endpoint = self.matter_test_config.global_test_params['PIXIT.CNET.ENDPOINT_THREAD']
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller
        TH1_nodeid = self.matter_test_config.controller_node_id
        TH2_nodeid = self.matter_test_config.controller_node_id + 1

        cnet = Clusters.NetworkCommissioning
        attr = cnet.Attributes

        # Commissioning is already done
        self.step("precondition")

        # Precondition 1: TH asses DUT supports CNET.S.F01(TH)
        # Precondition 2: DUT has a Network Commissioning cluster on endpoint PIXIT.CNET.ENDPOINT_THREAD with FeatureMap attribute of 2
        feature_map = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.FeatureMap)
        if not (self.check_pics("CNET.S.F01") or (feature_map & cnet.Bitmaps.Feature.kThreadNetworkInterface)):
            logging.info('Device does not support Thread on endpoint, skipping remaining steps')
            self.skip_all_remaining_steps(1)
            return

        # Precondition 3: DUT is factory reset (not needed)

        # Precondition 4: DUT is commissioned on PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET
        logger.info("Adding first test network")
        req = Clusters.NetworkCommissioning.Commands.AddOrUpdateThreadNetwork(
            operationalDataset=TEST_THREAD_NETWORK_DATASET_TLVS[0], breadcrumb=1)
        res = await commissioner.SendCommand(nodeid=TH1_nodeid, endpoint=endpoint, payload=req)
        logger.info(f"Received response: {res}")

        # Precondition 5: TH can communicate with the DUT on PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET
        # Precondition 6: DUT MaxNetworks attribute value is at least 1 and is saved as 'MaxNetworksValue' for future use
        logger.info("Check MaxNetworkValue")
        maxNetworksValue = 0
        if self.check_pics("CNET.S.A0000"):
            res = await commissioner.ReadAttribute(nodeid=TH1_nodeid, attributes=[(endpoint, attr.MaxNetworks.value)], returnClusterObject=True)
            maxNetworksValue = res[endpoint][cnet].maxNetworks
        assert_valid_uint8(maxNetworksValue, "MaxNetworksValue range")
        asserts.assert_greater_equal(maxNetworksValue, 1, "MaxNetworksValue not greater or equal to 1")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)

        logger.info("Arming the failsafe")
        req = Clusters.GeneralCommissioning.Commands.ArmFailSafe(expiryLengthSeconds=900)
        res = await commissioner.SendCommand(nodeid=TH1_nodeid, endpoint=endpoint, payload=req)
        logger.info(f"Received response: {res}")
        # Verify that DUT sends ArmFailSafeResponse command to the TH
        asserts.assert_equal(
            res.errorCode,
            Clusters.GeneralCommissioning.Enums.CommissioningErrorEnum.kOk,
            "ArmFailSafeResponse error code is not OK.",
        )

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET,
        # which does not match the commissioned network, and Breadcrumb field set to 1
        self.step(2)

        if self.check_pics("CNET.S.C04.Rsp"):
            if self.check_pics("CNET.S.C05.Tx"):
                logger.info("Check network list")
                res = await commissioner.ReadAttribute(nodeid=TH1_nodeid, attributes=[(endpoint, cnet.Attributes.Networks)], returnClusterObject=True)
                networkList = res[endpoint][cnet].networks
                logger.info(f"Got network list: {networkList}")
                if len(networkList) != 0:
                    logger.info("Removing existing network with extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET")
                    req = cnet.Commands.RemoveNetwork(networkID=PIXIT_CNET_THREAD_2ND_OPERATIONALDATASET, breadcrumb=1)
                    res = await commissioner.SendCommand(nodeid=TH1_nodeid, endpoint=endpoint, payload=req)
                    logger.info(f"Received response: {res}")

                    # Verify that DUT sends NetworkConfigResponse command to the TH1 with NetworkingStatus field set to NetworkIDNotFound
                    asserts.assert_is_instance(res, cnet.Commands.NetworkConfigResponse.response_type)
                    asserts.assert_equal(res.networkingStatus,
                                         cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
                                         f"Expected kNetworkIDNotFound but got: {res.networkingStatus}")    # LastNetworkingStatus?
                else:
                    asserts.fail(f"NetworkList is Empty")
            else:
                asserts.fail("DUT does not invoke/generate the NetworkConfigResponse command")
        else:
            asserts.fail("DUT does not implement receiving the RemoveNetwork command")

        # TH sends ConnectNetwork Command to the DUT with NetworkID value as the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET,
        # which does not match the commissioned network, and Breadcrumb field set to 1
        self.step(3)

        logger.info("Connect to PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET")
        networkID = await self.read_single_attribute(
            dev_ctrl=commissioner, node_id=TH2_nodeid, endpoint=endpoint, attribute=cnet.Structs.ThreadInterfaceScanResultStruct.extendedPanId)
        req = cnet.Commands.ConnectNetwork(networkID=networkID, breadcrumb=1)
        interactionTimeoutMs = commissioner.ComputeRoundTripTimeout(nodeid=TH2_nodeid, upperLayerProcessingTimeoutMs=30000)
        res = await commissioner.SendCommand(nodeid=TH1_nodeid, endpoint=endpoint, payload=req, interactionTimeoutMs=interactionTimeoutMs)
        logger.info(f"Got response: {res}")

        # Verify that DUT sends ConnectNetworkResponse command to the TH1 with NetworkingStatus field set to NetworkIDNotFound
        asserts.assert_is_instance(res, cnet.Commands.ConnectNetworkResponse.response_type)
        asserts.assert_equal(res.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kNetworkIDNotFound,
                             f"Expected NetworkIDNotFound but got {res.networkingStatus}")  # LastNetworkingStatus?


if __name__ == "__main__":
    default_matter_test_main()

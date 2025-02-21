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
#       --commissioning-method on-network
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: false
#     quiet: true

import logging
import random
import string
from typing import Optional

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main, type_matches, run_if_endpoint_matches, has_feature
from mobly import asserts

from controller.python.chip import ChipDeviceCtrl
from python_testing.matter_testing_infrastructure.chip.testing.matter_asserts import assert_valid_uint8

logger = logging.getLogger('NetworkCommissioning')
logger.setLevel(logging.INFO)

# TEST_THREAD_NETWORK_DATASET_TLVS = [bytes.fromhex("0e080000000000010000" +
#                                                   "000300000c" +
#                                                   "35060004001fffe0" +
#                                                   "0208fedcba9876543210" +
#                                                   "0708fd00000000001234" +
#                                                   "0510ffeeddccbbaa99887766554433221100" +
#                                                   "030e54657374696e674e6574776f726b" +
#                                                   "0102d252" +
#                                                   "041081cb3b2efa781cc778397497ff520fa50c0302a0ff"),
#                                     # End of first TLV
#                                     ]
# # Network id, for the thread network, current a const value, will be changed to XPANID of the thread network.
# TEST_THREAD_NETWORK_IDS = [
#     bytes.fromhex("fedcba9876543210"),
# ]
# THREAD_NETWORK_FEATURE_MAP = 2
PIXIT_CNET_THREAD_1ST_OPERATIONALDATASET = 0x1111111111111111
PIXIT_CNET_THREAD_2ND_OPERATIONALDATASET = 0x1111111122222222


class TC_CNET_4_16(MatterBaseTest):
    def steps_TC_CNET_4_16(self):
        return [TestStep("precondition", "TH is commissioned", is_commissioning=True),
                TestStep("precondition 1", "TH asses DUT supports CNET.S.F01 and reads FeatureMap attribute from the DUT."),
                # TestStep("precondition 2", "DUT is factory reset"),
                TestStep("precondition 3", "TH can communicate with the DUT on PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET"),
                TestStep(
                    "precondition 4", 'TH reads DUT MaxNetworks attribute value and asserts it is at least 1 and is saved as maxNetworksValue for future use'),
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
        self.endpoint = self.matter_test_config.global_test_params['PIXIT.CNET.ENDPOINT_THREAD']
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller

        # Commissioning is already done
        self.step("precondition")

        cnet = Clusters.NetworkCommissioning
        attr = cnet.Attributes

        # TH asses DUT supports CNET.S.F01 and reads FeatureMap attribute from the DUT.
        self.step("precondition 1")
        feature_map = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.FeatureMap)
        if not (self.check_pics("CNET.S.F01") or (feature_map & cnet.Bitmaps.Feature.kThreadNetworkInterface)):
            logging.info('Device does not support Thread on endpoint, skipping remaining steps')
            self.skip_all_remaining_steps(2)
            return

        # DUT is factory reset
        # self.step("precondition 2")

        # TH can communicate with the DUT on PIXIT.CNET.THREAD_1ST_OPERATIONALDATASET
        self.step("precondition 3")

        # DUT MaxNetworks attribute value is at least 1 and is saved as 'MaxNetworksValue' for future use
        self.step("precondition 4")
        maxNetworksValue = 1
        if self.check_pics("CNET.S.A0000"):
            maxNetworksValue = await self.read_single_attribute_check_success(cluster=cnet, attribute=attr.MaxNetworks)
        assert_valid_uint8(maxNetworksValue, "MaxNetworksValue range")
        asserts.assert_greater_equal(maxNetworksValue, 1, "MaxNetworksValue not greater or equal to 1")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET,
        # which does not match the commissioned network, and Breadcrumb field set to 1
        self.step(2)

        # TH sends ConnectNetwork Command to the DUT with NetworkID value as the extended PAN ID of PIXIT.CNET.THREAD_2ND_OPERATIONALDATASET,
        # which does not match the commissioned network, and Breadcrumb field set to 1
        self.step(3)


if __name__ == "__main__":
    default_matter_test_main()

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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --PICS src/app/tests/suites/certification/ci-pics-values --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
# === END CI TEST ARGUMENTS ===

import logging

import chip.clusters as Clusters
from matter_testing_support import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

kRootEndpointId = 0
kSecondaryNetworkInterfaceDeviceTypeId = 0x0019


class TC_CNET_1_4(MatterBaseTest):
    def steps_TC_CNET_1_4(self):
        return [TestStep(1, "TH is commissioned", is_commissioning=True),
                TestStep(2, 'TH performs a wildcard read of the FeatureMap attribute on Network Commissioning clusters across all endpoints, and saves the response as `NetworkCommissioningResponse`'),
                TestStep(3, 'If `NetworkCommissioningResponse` does not contain any entries for Network Commissioning cluster, skip remaining steps and end test case'),
                TestStep(4, 'If `NetworkCommissioningResponse` contains only a single entry for Network Commissioning cluster on Endpoint 0, skip remaining steps and end test case. Verify `NetworkCommissioningResponse` contains an entry for Network Commissioning cluster on Endpoint 0'),
                TestStep(5, 'TH reads from the DUT the Descriptor Cluster DeviceTypeList attribute on each endpoint from the `NetworkCommissioningResponse` (except for Endpoint 0), verify that the Secondary Network Interface device type id (0x0019) is listed in the DeviceTypeList'),
                TestStep(6, 'TH reads from the DUT the General Commissioning Cluster SupportsConcurrentConnection attribute, verify that it is true')]

    def def_TC_CNET_1_4(self):
        return '[TC-CNET-1.4] Verification for Secondary Network Interface [DUT-Server]'

    def pics_TC_CNET_1_4(self):
        return ['CNET.S']

    # Override default timeout.
    @property
    def default_timeout(self) -> int:
        return 200

    @async_test_body
    async def test_TC_CNET_1_4(self):
        # Commissioning is already done
        self.step(1)

        self.step(2)
        # Read FeatureMap attribute with wildcard endpoint
        NetworkCommissioningResponse = await self.default_controller.ReadAttribute(self.dut_node_id, [(Clusters.NetworkCommissioning.Attributes.FeatureMap)], fabricFiltered=True)

        self.step(3)
        NumNetworkCommissioning = len(NetworkCommissioningResponse)
        if NumNetworkCommissioning == 0:
            logging.info('No endpoint has Network Commissioning Cluster, skipping remaining steps')
            self.skip_all_remaining_steps(4)
            return

        self.step(4)
        endpoints = []
        for endpoint, _ in NetworkCommissioningResponse.items():
            endpoints.append(endpoint)
        if kRootEndpointId not in endpoints:
            asserts.assert_true(False, "There is no Network Commissioning Cluster on endpoint 0")

        if NumNetworkCommissioning == 1:
            logging.info('Only endpoint 0 has Network Commissioning Cluster, skipping remaining steps')
            self.skip_all_remaining_steps(5)
            return

        self.step(5)
        for endpoint in endpoints:
            if endpoint == kRootEndpointId:
                continue
            device_type_list = await self.read_single_attribute_check_success(cluster=Clusters.Descriptor,
                                                                              attribute=Clusters.Descriptor.Attributes.DeviceTypeList, endpoint=endpoint)
            required_device_types = {kSecondaryNetworkInterfaceDeviceTypeId}
            found_device_types = {device.deviceType for device in device_type_list}
            asserts.assert_true(required_device_types.intersection(found_device_types),
                                "Network Commissioning Cluster is not on Root Node or Secondary Network Interface")

        self.step(6)
        concurrent_connection = await self.read_single_attribute_check_success(cluster=Clusters.GeneralCommissioning,
                                                                               attribute=Clusters.GeneralCommissioning.Attributes.SupportsConcurrentConnection,
                                                                               endpoint=kRootEndpointId)
        asserts.assert_true(concurrent_connection, "The device does not support concurrent connection commissioning")


if __name__ == "__main__":
    default_matter_test_main()

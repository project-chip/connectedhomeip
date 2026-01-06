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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
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
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.testing.event_attribute_reporting import AttributeSubscriptionHandler
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)
cluster = Clusters.Objects.GeneralDiagnostics


class TC_DGGEN_2_5(MatterBaseTest):
    def desc_TC_DGGEN_2_5(self) -> str:
        return "[TC_DGGEN_2_5] DeviceLoadStatus Attribute Tests"

    def pics_TC_DGGEN_2_5(self) -> list[str]:
        return ["DGGEN.S"]

    def steps_TC_DGGEN_2_5(self):
        return [TestStep(1, "Ensure DUT is commissioned in TH1’s fabric.", "TH1 is able to communicate with DUT."),
                TestStep(2, "TH1 reads DUT’s ClusterRevision global attribute in General Diagnostics cluster.",
                         "If the ClusterRevision is < 3, then skip the rest of the test steps."),
                TestStep(3, "Ensure DUT is commissioned in TH2’s fabric.", "TH2 is able to communicate with DUT."),
                TestStep(4, "TH1 reads the DUT’s DeviceLoadStatus attribute in General Diagnostics cluster.\n"
                         "Save the TotalInteractionModelMessagesSent field of the DeviceLoadStatus attribute as 'initialTotalInteractionModelMessageSent'.\n"
                         "Save the TotalInteractionModelMessagesReceived field of the DeviceLoadStatus attribute as 'initialTotalInteractionModelMessagesReceived'.\n"
                         "Save the CurrentSubscriptions field of the DeviceLoadStatus attribute as 'initialCurrentSubscriptions'.\n"
                         "Save the CurrentSubscriptionsForFabric field of the DeviceLoadStatus attribute as 'initialCurrentSubscriptionsForFabric'.\n"
                         "Save the TotalSubscriptionsEstablished field of the DeviceLoadStatus attribute as 'initialTotalSubscriptionsEstablished'.",
                         "Verify that initialCurrentSubscriptionsForFabric <= initialCurrentSubscriptions.\n"
                         "Verify that initialTotalSubscriptionsEstablished >= initialCurrentSubscriptions."),
                TestStep(5, "TH1 subscribes to the General Diagnostics cluster’s RebootCount attribute, min interval 1 second, max interval 30 seconds, KeepSubscriptions false.",
                         "Subscription succeeds."),
                TestStep(6, "TH1 subscribes to the General Diagnostics cluster’s RebootCount attribute, min interval 1 second, max interval 30 seconds, KeepSubscriptions true.",
                         "Subscription succeeds."),
                TestStep(7, "TH2 subscribes to the General Diagnostics cluster’s RebootCount attribute, min interval 1 second, max interval 30 seconds, KeepSubscriptions false.",
                         "Subscription succeeds."),
                TestStep(8, "TH1 reads the DUT’s DeviceLoadStatus attribute in General Diagnostics cluster.",
                         "Verify that the TotalInteractionModelMessagesSent field of the DeviceLoadStatus attribute is higher than the saved initialTotalInteractionModelMessageSent by at least 3 messages.\n"
                         "Verify that the TotalInteractionModelMessagesReceived field of the DeviceLoadStatus attribute is higher than the saved initialTotalInteractionModelMessagesReceived by at least 6 messages.\n"
                         "Verify that the CurrentSubscriptions field of the DeviceLoadStatus attribute is higher than the saved initialCurrentSubscriptions by exactly 3 subscriptions.\n"
                         "Verify that the CurrentSubscriptionsForFabric field of the DeviceLoadStatus attribute is higher than the saved initialCurrentSubscriptionsForFabric by exactly 2 subscriptions.\n"
                         "Verify that the TotalSubscriptionsEstablished field of the DeviceLoadStatus attribute is higher than the saved initialTotalSubscriptionsEstablished by exactly 3 subscriptions.")
                ]

    @run_if_endpoint_matches(has_cluster(cluster))
    async def test_TC_DGGEN_2_5(self):
        endpoint = self.get_endpoint()

        # Ensure DUT is commissioned in TH1’s fabric.
        self.step(1)
        self.th1 = self.default_controller

        # TH1 reads DUT’s ClusterRevision global attribute in General Diagnostics cluster.
        self.step(2)
        clusterRevision = await self.read_single_attribute_check_success(
            cluster=cluster, attribute=cluster.Attributes.ClusterRevision, dev_ctrl=self.th1, endpoint=endpoint)

        logger.info(f"ClusterRevision Attribute: {clusterRevision}")

        # If the ClusterRevision is < 3, then skip the rest of the test steps.
        if clusterRevision < 3:
            self.skip_step(3)
            self.skip_step(4)
            self.skip_step(5)
            self.skip_step(6)
            self.skip_step(7)
            self.skip_step(8)
        else:
            # Ensure DUT is commissioned in TH2’s fabric.
            self.step(3)
            logger.info("Setting up TH2")
            th2CertificateAuth = self.certificate_authority_manager.NewCertificateAuthority()
            th2FabricAdmin = th2CertificateAuth.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
            self.th2 = th2FabricAdmin.NewController(nodeId=2, useTestCommissioner=True)

            logger.info("Opening commissioning window on DUT")
            params = await self.open_commissioning_window()
            await self.th2.CommissionOnNetwork(
                nodeId=self.dut_node_id,
                setupPinCode=params.commissioningParameters.setupPinCode,
                filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR,
                filter=params.randomDiscriminator
            )

            # TH1 reads the DUT’s DeviceLoadStatus attribute in General Diagnostics cluster.
            self.step(4)
            if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DeviceLoadStatus):
                deviceLoadStatus = await self.read_single_attribute_check_success(
                    cluster=cluster, attribute=cluster.Attributes.DeviceLoadStatus, dev_ctrl=self.th1, endpoint=endpoint)

                logger.info(f"DeviceLoadStatus Attribute: {deviceLoadStatus}")

                initialTotalInteractionModelMessageSent = deviceLoadStatus.totalInteractionModelMessagesSent
                initialTotalInteractionModelMessagesReceived = deviceLoadStatus.totalInteractionModelMessagesReceived
                initialCurrentSubscriptions = deviceLoadStatus.currentSubscriptions
                initialCurrentSubscriptionsForFabric = deviceLoadStatus.currentSubscriptionsForFabric
                initialTotalSubscriptionsEstablished = deviceLoadStatus.totalSubscriptionsEstablished

                # Verify that initialCurrentSubscriptionsForFabric <= initialCurrentSubscriptions.
                asserts.assert_less_equal(initialCurrentSubscriptionsForFabric, initialCurrentSubscriptions,
                                          f"initialCurrentSubscriptionsForFabric: {initialCurrentSubscriptionsForFabric} is not less or equal than initialCurrentSubscriptions: {initialCurrentSubscriptions}")

                # Verify that initialTotalSubscriptionsEstablished >= initialCurrentSubscriptions.
                asserts.assert_greater_equal(initialTotalSubscriptionsEstablished, initialCurrentSubscriptions,
                                             f"initialTotalSubscriptionsEstablished: {initialTotalSubscriptionsEstablished} is not greater or equal than initialCurrentSubscriptions: {initialCurrentSubscriptions}")

            # TH1 subscribes to the General Diagnostics cluster’s RebootCount attribute, min interval 1 second, max interval 30 seconds, KeepSubscriptions false.
            self.step(5)
            rebootCountCallBack = AttributeSubscriptionHandler(
                expected_cluster=cluster, expected_attribute=cluster.Attributes.RebootCount)
            await rebootCountCallBack.start(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=endpoint, fabric_filtered=False, min_interval_sec=1, max_interval_sec=30, keepSubscriptions=False)

            # TH1 subscribes to the General Diagnostics cluster’s RebootCount attribute, min interval 1 second, max interval 30 seconds, KeepSubscriptions true.
            self.step(6)
            rebootCountCallBack2 = AttributeSubscriptionHandler(
                expected_cluster=cluster, expected_attribute=cluster.Attributes.RebootCount)
            await rebootCountCallBack2.start(dev_ctrl=self.th1, node_id=self.dut_node_id, endpoint=endpoint, fabric_filtered=False, min_interval_sec=1, max_interval_sec=30, keepSubscriptions=True)

            # TH2 subscribes to the General Diagnostics cluster’s RebootCount attribute, min interval 1 second, max interval 30 seconds, KeepSubscriptions false.
            self.step(7)
            rebootCountCallBack3 = AttributeSubscriptionHandler(
                expected_cluster=cluster, expected_attribute=cluster.Attributes.RebootCount)
            await rebootCountCallBack3.start(dev_ctrl=self.th2, node_id=self.dut_node_id, endpoint=endpoint, fabric_filtered=False, min_interval_sec=1, max_interval_sec=30, keepSubscriptions=False)

            # TH1 reads the DUT’s DeviceLoadStatus attribute in General Diagnostics cluster.
            self.step(8)
            if await self.attribute_guard(endpoint=endpoint, attribute=cluster.Attributes.DeviceLoadStatus):
                deviceLoadStatus2 = await self.read_single_attribute_check_success(
                    cluster=cluster, attribute=cluster.Attributes.DeviceLoadStatus, dev_ctrl=self.th1, endpoint=endpoint)

                logger.info(f"DeviceLoadStatus Attribute: {deviceLoadStatus2}")

                # Verify that the TotalInteractionModelMessagesSent field of the DeviceLoadStatus attribute is higher than the saved initialTotalInteractionModelMessageSent by at least 3 messages
                finalTotalInteractionModelMessageSent = deviceLoadStatus2.totalInteractionModelMessagesSent
                expectedIncreaseInteractionModeMessagesSent = 3
                asserts.assert_greater_equal(finalTotalInteractionModelMessageSent, initialTotalInteractionModelMessageSent + expectedIncreaseInteractionModeMessagesSent,
                                             f"finalTotalInteractionModelMessageSent: {finalTotalInteractionModelMessageSent} is not greater by at least {expectedIncreaseInteractionModeMessagesSent} messages than initialTotalInteractionModelMessageSent: {initialTotalInteractionModelMessageSent}")

                # Verify that the TotalInteractionModelMessagesReceived field of the DeviceLoadStatus attribute is higher than the saved initialTotalInteractionModelMessagesReceived by at least 6 messages.
                finalTotalInteractionModelMessagesReceived = deviceLoadStatus2.totalInteractionModelMessagesReceived
                expectedIncreaseInteractionModeMessagesReceived = 6
                asserts.assert_greater_equal(finalTotalInteractionModelMessagesReceived, initialTotalInteractionModelMessagesReceived + expectedIncreaseInteractionModeMessagesReceived,
                                             f"finalTotalInteractionModelMessagesReceived: {finalTotalInteractionModelMessagesReceived} is not greater by at least {expectedIncreaseInteractionModeMessagesReceived} messages than initialTotalInteractionModelMessagesReceived: {initialTotalInteractionModelMessagesReceived}")

                # Verify that the CurrentSubscriptions field of the DeviceLoadStatus attribute is higher than the saved initialCurrentSubscriptions by exactly 3 subscriptions.
                finalCurrentSubscriptions = deviceLoadStatus2.currentSubscriptions
                expectedIncreaseCurrentSubscriptions = 3
                asserts.assert_equal(finalCurrentSubscriptions, initialCurrentSubscriptions + expectedIncreaseCurrentSubscriptions,
                                     f"finalCurrentSubscriptions: {finalCurrentSubscriptions} is not greater by exactly {expectedIncreaseCurrentSubscriptions} subscriptions to initialCurrentSubscriptions: {initialCurrentSubscriptions}")

                # Verify that the CurrentSubscriptionsForFabric field of the DeviceLoadStatus attribute is higher than the saved initialCurrentSubscriptionsForFabric by exactly 2 subscriptions.
                finalCurrentSubscriptionsForFabric = deviceLoadStatus2.currentSubscriptionsForFabric
                expectedIncreaseSubscriptionsForFabric = 2
                asserts.assert_equal(finalCurrentSubscriptionsForFabric, initialCurrentSubscriptionsForFabric + expectedIncreaseSubscriptionsForFabric,
                                     f"finalCurrentSubscriptionsForFabric: {finalCurrentSubscriptionsForFabric} is not greater by exactly {expectedIncreaseSubscriptionsForFabric} subscriptions to initialCurrentSubscriptionsForFabric: {initialCurrentSubscriptionsForFabric}")

                # Verify that the TotalSubscriptionsEstablished field of the DeviceLoadStatus attribute is higher than the saved initialTotalSubscriptionsEstablished by exactly 3 subscriptions.
                finalTotalSubscriptionsEstablished = deviceLoadStatus2.totalSubscriptionsEstablished
                expectedIncreaseSubscriptionsEstablished = 3
                asserts.assert_equal(finalTotalSubscriptionsEstablished, initialTotalSubscriptionsEstablished + expectedIncreaseSubscriptionsEstablished,
                                     f"finalTotalSubscriptionsEstablished: {finalTotalSubscriptionsEstablished} is not greater by exactly {expectedIncreaseSubscriptionsEstablished} subscriptions to initialTotalSubscriptionsEstablished:{initialTotalSubscriptionsEstablished}")

            rebootCountCallBack.cancel()
            rebootCountCallBack2.cancel()
            rebootCountCallBack3.cancel()


if __name__ == "__main__":
    default_matter_test_main()

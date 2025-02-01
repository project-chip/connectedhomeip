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
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===
#

import chip.clusters as Clusters
from chip.testing import matter_asserts
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


class TC_THREADND_2_2(MatterBaseTest):
    """
    [TC-THREADND-2.2] Thread Network Diagnostics Cluster - Attribute Read Verification for Tx Attributes

    This test case verifies the behavior of the Tx-related attributes of the Thread Network Diagnostics 
    cluster server (Server as DUT). The test case steps are derived from the provided 
    test plan specification.
    """

    async def read_thread_diagnostics_attribute_expect_success(self, endpoint, attribute):
        """
        Convenience method to read a single ThreadNetworkDiagnostics attribute,
        ensuring success.
        """
        cluster = Clusters.Objects.ThreadNetworkDiagnostics
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    #
    # --- Test Description, PICS, and Steps ---
    #
    def desc_TC_THREADND_2_2(self) -> str:
        return "[TC-THREADND-2.2] Thread Network Diagnostics Tx Attributes with Server as DUT"

    def pics_TC_THREADND_2_2(self) -> list[str]:
        return ["THREADND.S", "THREADND.SF.MACCNT"]  # PICS identifiers for the test case

    def steps_TC_THREADND_2_2(self) -> list[TestStep]:
        """
        Lists the test steps from the specification in an easy-to-read format.
        """
        return [
            TestStep(1, "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep(2, "Read TxTotalCount attribute"),
            TestStep(3, "Read TxUnicastCount attribute"),
            TestStep(4, "Read TxBroadcastCount attribute"),
            TestStep(5, "Read TxAckRequestedCount attribute"),
            TestStep(6, "Read TxAckedCount attribute"),
            TestStep(7, "Read TxNoAckRequestedCount attribute"),
            TestStep(8, "Read TxDataCount attribute"),
            TestStep(9, "Read TxDataPollCount attribute"),
            TestStep(10, "Read TxBeaconCount attribute"),
            TestStep(11, "Read TxBeaconRequestCount attribute"),
            TestStep(12, "Read TxOtherCount attribute"),
            TestStep(13, "Read TxRetryCount attribute"),
            TestStep(14, "Read TxDirectMaxRetryExpiryCount attribute"),
            TestStep(15, "Read TxIndirectMaxRetryExpiryCount attribute"),
            TestStep(16, "Read TxErrCcaCount attribute"),
            TestStep(17, "Read TxErrAbortCount attribute"),
            TestStep(18, "Read TxErrBusyChannelCount attribute"),
        ]

    #
    # --- Main Test Routine ---
    #
    @async_test_body
    async def test_TC_THREADND_2_2(self):
        endpoint = self.get_endpoint(default=0)
        attributes = Clusters.ThreadNetworkDiagnostics.Attributes

        #
        # STEP 1: Commissioning (assumed done)
        #
        self.step(1)
        # Normally performed by harness; no explicit code needed if already commissioned.

        #
        # STEP 2: Read TxTotalCount
        #
        self.step(2)
        tx_total_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxTotalCount)
        if tx_total_count is not None:
            matter_asserts.assert_valid_uint32(tx_total_count, "TxTotalCount")

        #
        # STEP 3: Read TxUnicastCount
        #
        self.step(3)
        tx_unicast_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxUnicastCount)
        if tx_unicast_count is not None:
            matter_asserts.assert_valid_uint32(tx_unicast_count, "TxUnicastCount")

        #
        # STEP 4: Read TxBroadcastCount
        #
        self.step(4)
        tx_broadcast_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxBroadcastCount)
        if tx_broadcast_count is not None:
            matter_asserts.assert_valid_uint32(tx_broadcast_count, "TxBroadcastCount")

        #
        # STEP 5: Read TxAckRequestedCount
        #
        self.step(5)
        tx_ack_requested_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxAckRequestedCount)
        if tx_ack_requested_count is not None:
            matter_asserts.assert_valid_uint32(tx_ack_requested_count, "TxAckRequestedCount")

        #
        # STEP 6: Read TxAckedCount
        #
        self.step(6)
        tx_acked_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxAckedCount)
        if tx_acked_count is not None:
            matter_asserts.assert_valid_uint32(tx_acked_count, "TxAckedCount")

        #
        # STEP 7: Read TxNoAckRequestedCount
        #
        self.step(7)
        tx_no_ack_requested_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxNoAckRequestedCount)
        if tx_no_ack_requested_count is not None:
            matter_asserts.assert_valid_uint32(tx_no_ack_requested_count, "TxNoAckRequestedCount")

        #
        # STEP 8: Read TxDataCount
        #
        self.step(8)
        tx_data_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxDataCount)
        if tx_data_count is not None:
            matter_asserts.assert_valid_uint32(tx_data_count, "TxDataCount")

        #
        # STEP 9: Read TxDataPollCount
        #
        self.step(9)
        tx_data_poll_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxDataPollCount)
        if tx_data_poll_count is not None:
            matter_asserts.assert_valid_uint32(tx_data_poll_count, "TxDataPollCount")

        #
        # STEP 10: Read TxBeaconCount
        #
        self.step(10)
        tx_beacon_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxBeaconCount)
        if tx_beacon_count is not None:
            matter_asserts.assert_valid_uint32(tx_beacon_count, "TxBeaconCount")

        #
        # STEP 11: Read TxBeaconRequestCount
        #
        self.step(11)
        tx_beacon_request_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxBeaconRequestCount)
        if tx_beacon_request_count is not None:
            matter_asserts.assert_valid_uint32(tx_beacon_request_count, "TxBeaconRequestCount")

        #
        # STEP 12: Read TxOtherCount
        #
        self.step(12)
        tx_other_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxOtherCount)
        if tx_other_count is not None:
            matter_asserts.assert_valid_uint32(tx_other_count, "TxOtherCount")

        #
        # STEP 13: Read TxRetryCount
        #
        self.step(13)
        tx_retry_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxRetryCount)
        if tx_retry_count is not None:
            matter_asserts.assert_valid_uint32(tx_retry_count, "TxRetryCount")

        #
        # STEP 14: Read TxDirectMaxRetryExpiryCount
        #
        self.step(14)
        tx_direct_max_retry_expiry_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxDirectMaxRetryExpiryCount)
        if tx_direct_max_retry_expiry_count is not None:
            matter_asserts.assert_valid_uint32(tx_direct_max_retry_expiry_count, "TxDirectMaxRetryExpiryCount")

        #
        # STEP 15: Read TxIndirectMaxRetryExpiryCount
        #
        self.step(15)
        tx_indirect_max_retry_expiry_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxIndirectMaxRetryExpiryCount)
        if tx_indirect_max_retry_expiry_count is not None:
            matter_asserts.assert_valid_uint32(tx_indirect_max_retry_expiry_count, "TxIndirectMaxRetryExpiryCount")

        #
        # STEP 16: Read TxErrCcaCount
        #
        self.step(16)
        tx_err_cca_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxErrCcaCount)
        if tx_err_cca_count is not None:
            matter_asserts.assert_valid_uint32(tx_err_cca_count, "TxErrCcaCount")

        #
        # STEP 17: Read TxErrAbortCount
        #
        self.step(17)
        tx_err_abort_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxErrAbortCount)
        if tx_err_abort_count is not None:
            matter_asserts.assert_valid_uint32(tx_err_abort_count, "TxErrAbortCount")

        #
        # STEP 18: Read TxErrBusyChannelCount
        #
        self.step(18)
        tx_err_busy_channel_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.TxErrBusyChannelCount)
        if tx_err_busy_channel_count is not None:
            matter_asserts.assert_valid_uint32(tx_err_busy_channel_count, "TxErrBusyChannelCount")


if __name__ == "__main__":
    default_matter_test_main()

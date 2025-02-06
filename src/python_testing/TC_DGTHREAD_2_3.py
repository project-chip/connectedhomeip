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


class TC_THREADND_2_3(MatterBaseTest):
    """
    [TC-THREADND-2.3] Thread Network Diagnostics Cluster - Rx Attribute Read Verification

    This test case verifies the behavior of the Rx-related attributes of the Thread Network Diagnostics 
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
    def desc_TC_THREADND_2_3(self) -> str:
        return "[TC-THREADND-2.3] Thread Network Diagnostics Rx Attributes with Server as DUT"

    def pics_TC_THREADND_2_3(self) -> list[str]:
        return ["THREADND.S", "THREADND.SF.MACCNT"]  # PICS identifiers for the test case

    def steps_TC_THREADND_2_3(self) -> list[TestStep]:
        """
        Lists the test steps from the specification in an easy-to-read format.
        """
        return [
            TestStep(1, "Commission DUT to TH (already done)", is_commissioning=True),
            TestStep(2, "Read RxTotalCount attribute"),
            TestStep(3, "Read RxUnicastCount attribute"),
            TestStep(4, "Read RxBroadcastCount attribute"),
            TestStep(5, "Read RxDataCount attribute"),
            TestStep(6, "Read RxDataPollCount attribute"),
            TestStep(7, "Read RxBeaconCount attribute"),
            TestStep(8, "Read RxBeaconRequestCount attribute"),
            TestStep(9, "Read RxOtherCount attribute"),
            TestStep(10, "Read RxAddressFilteredCount attribute"),
            TestStep(11, "Read RxDestAddrFilteredCount attribute"),
            TestStep(12, "Read RxDuplicatedCount attribute"),
            TestStep(13, "Read RxErrNoFrameCount attribute"),
            TestStep(14, "Read RxErrUnknownNeighborCount attribute"),
            TestStep(15, "Read RxErrInvalidSrcAddrCount attribute"),
            TestStep(16, "Read RxErrSecCount attribute"),
            TestStep(17, "Read RxErrFcsCount attribute"),
            TestStep(18, "Read RxErrOtherCount attribute"),
        ]

    #
    # --- Main Test Routine ---
    #
    @async_test_body
    async def test_TC_THREADND_2_3(self):
        endpoint = self.get_endpoint(default=0)
        attributes = Clusters.ThreadNetworkDiagnostics.Attributes

        #
        # STEP 1: Commissioning (assumed done)
        #
        self.step(1)
        # Normally performed by harness; no explicit code needed if already commissioned.

        #
        # STEP 2: Read RxTotalCount
        #
        self.step(2)
        rx_total_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxTotalCount)
        if rx_total_count is not None:
            matter_asserts.assert_valid_uint32(rx_total_count, "RxTotalCount")

        #
        # STEP 3: Read RxUnicastCount
        #
        self.step(3)
        rx_unicast_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxUnicastCount)
        if rx_unicast_count is not None:
            matter_asserts.assert_valid_uint32(rx_unicast_count, "RxUnicastCount")

        #
        # STEP 4: Read RxBroadcastCount
        #
        self.step(4)
        rx_broadcast_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxBroadcastCount)
        if rx_broadcast_count is not None:
            matter_asserts.assert_valid_uint32(rx_broadcast_count, "RxBroadcastCount")

        #
        # STEP 5: Read RxDataCount
        #
        self.step(5)
        rx_data_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxDataCount)
        if rx_data_count is not None:
            matter_asserts.assert_valid_uint32(rx_data_count, "RxDataCount")

        #
        # STEP 6: Read RxDataPollCount
        #
        self.step(6)
        rx_data_poll_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxDataPollCount)
        if rx_data_poll_count is not None:
            matter_asserts.assert_valid_uint32(rx_data_poll_count, "RxDataPollCount")

        #
        # STEP 7: Read RxBeaconCount
        #
        self.step(7)
        rx_beacon_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxBeaconCount)
        if rx_beacon_count is not None:
            matter_asserts.assert_valid_uint32(rx_beacon_count, "RxBeaconCount")

        #
        # STEP 8: Read RxBeaconRequestCount
        #
        self.step(8)
        rx_beacon_request_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxBeaconRequestCount)
        if rx_beacon_request_count is not None:
            matter_asserts.assert_valid_uint32(rx_beacon_request_count, "RxBeaconRequestCount")

        #
        # STEP 9: Read RxOtherCount
        #
        self.step(9)
        rx_other_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxOtherCount)
        if rx_other_count is not None:
            matter_asserts.assert_valid_uint32(rx_other_count, "RxOtherCount")

        #
        # STEP 10: Read RxAddressFilteredCount
        #
        self.step(10)
        rx_address_filtered_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxAddressFilteredCount)
        if rx_address_filtered_count is not None:
            matter_asserts.assert_valid_uint32(rx_address_filtered_count, "RxAddressFilteredCount")

        #
        # STEP 11: Read RxDestAddrFilteredCount
        #
        self.step(11)
        rx_dest_addr_filtered_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxDestAddrFilteredCount)
        if rx_dest_addr_filtered_count is not None:
            matter_asserts.assert_valid_uint32(rx_dest_addr_filtered_count, "RxDestAddrFilteredCount")

        #
        # STEP 12: Read RxDuplicatedCount
        #
        self.step(12)
        rx_duplicated_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxDuplicatedCount)
        if rx_duplicated_count is not None:
            matter_asserts.assert_valid_uint32(rx_duplicated_count, "RxDuplicatedCount")

        #
        # STEP 13: Read RxErrNoFrameCount
        #
        self.step(13)
        rx_err_no_frame_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxErrNoFrameCount)
        if rx_err_no_frame_count is not None:
            matter_asserts.assert_valid_uint32(rx_err_no_frame_count, "RxErrNoFrameCount")

        #
        # STEP 14: Read RxErrUnknownNeighborCount
        #
        self.step(14)
        rx_err_unknown_neighbor_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxErrUnknownNeighborCount)
        if rx_err_unknown_neighbor_count is not None:
            matter_asserts.assert_valid_uint32(rx_err_unknown_neighbor_count, "RxErrUnknownNeighborCount")

        #
        # STEP 15: Read RxErrInvalidSrcAddrCount
        #
        self.step(15)
        rx_err_invalid_src_addr_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxErrInvalidSrcAddrCount)
        if rx_err_invalid_src_addr_count is not None:
            matter_asserts.assert_valid_uint32(rx_err_invalid_src_addr_count, "RxErrInvalidSrcAddrCount")

        #
        # STEP 16: Read RxErrSecCount
        #
        self.step(16)
        rx_err_sec_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxErrSecCount)
        if rx_err_sec_count is not None:
            matter_asserts.assert_valid_uint32(rx_err_sec_count, "RxErrSecCount")

        #
        # STEP 17: Read RxErrFcsCount
        #
        self.step(17)
        rx_err_fcs_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxErrFcsCount)
        if rx_err_fcs_count is not None:
            matter_asserts.assert_valid_uint32(rx_err_fcs_count, "RxErrFcsCount")

        #
        # STEP 18: Read RxErrOtherCount
        #
        self.step(18)
        rx_err_other_count = await self.read_thread_diagnostics_attribute_expect_success(endpoint, attributes.RxErrOtherCount)
        if rx_err_other_count is not None:
            matter_asserts.assert_valid_uint32(rx_err_other_count, "RxErrOtherCount")


if __name__ == "__main__":
    default_matter_test_main()

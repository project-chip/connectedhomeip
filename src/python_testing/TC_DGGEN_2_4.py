#
#    Copyright (c) 2023 Project CHIP Authors
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
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from chip.interaction_model import InteractionModelError
from chip.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main
from chip.testing.timeoperations import utc_datetime_from_matter_epoch_us, utc_datetime_from_posix_time_ms, utc_time_in_matter_epoch
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_DGGEN_2_4(MatterBaseTest):
    async def read_diags_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.GeneralDiagnostics
        return await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=attribute)

    async def read_timesync_attribute_expect_success(self, attribute):
        cluster = Clusters.Objects.TimeSynchronization
        return await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=attribute)

    async def set_time_in_timesync(self, current_time: int):
        endpoint = 0
        time_cluster = Clusters.Objects.TimeSynchronization
        code = 0

        try:
            await self.send_single_cmd(cmd=time_cluster.Commands.SetUTCTime(UTCTime=current_time, granularity=time_cluster.Enums.GranularityEnum.kMillisecondsGranularity), endpoint=endpoint)
        except InteractionModelError as e:
            # The python layer discards the cluster specific portion of the status IB, so for now we just expect a generic FAILURE error
            # see #26521
            code = e.status

        asserts.assert_true(code in [0, 1], "Unexpected error while trying to set the UTCTime")

    async def send_time_snapshot_expect_success(self):
        endpoint = 0
        diags_cluster = Clusters.Objects.GeneralDiagnostics
        code = 0

        try:
            response = await self.send_single_cmd(cmd=diags_cluster.Commands.TimeSnapshot(), endpoint=endpoint)
        except InteractionModelError as e:
            code = e.status

        asserts.assert_equal(code, 0, "Expected success of TimeSnapshot.")
        return response

    @async_test_body
    async def test_TC_DGGEN_2_4(self):
        self.print_step("1a", "Detect Time Synchronization UTCTime attribute presence")
        root_descriptor = await self.default_controller.ReadAttribute(self.dut_node_id, [(0, Clusters.Descriptor)])
        root_server_list = root_descriptor[0][Clusters.Descriptor][Clusters.Descriptor.Attributes.ServerList]
        has_timesync = (Clusters.TimeSynchronization.id in root_server_list)

        testvar_TimeSyncSupported = False

        if has_timesync:
            ts_attributes = await self.read_single_attribute(self.default_controller, node_id=self.dut_node_id, endpoint=0, attribute=Clusters.TimeSynchronization.Attributes.AttributeList)
            has_utc_time = (Clusters.TimeSynchronization.Attributes.UTCTime.attribute_id in ts_attributes)

            if has_utc_time:
                testvar_TimeSyncSupported = True
            else:
                asserts.fail("Time Synchronization cluster present but UTCTime missing. This should not happen!")

            self.print_step("1b", "Write current time to DUT")
            # Get current time in the correct format to set via command.
            th_utc = utc_time_in_matter_epoch(desired_datetime=None)

            await self.set_time_in_timesync(th_utc)

            self.print_step("1c", "Read current time from DUT")
            testvar_UTCTime1 = await self.read_timesync_attribute_expect_success(Clusters.TimeSynchronization.Attributes.UTCTime)
            asserts.assert_true(testvar_UTCTime1 != NullValue,
                                "UTCTime1 readback must not be null after SetUTCTime (per Time Synchronization cluster spec)")

            testvar_TimeSyncSupported = True

        self.print_step(2, "Read UpTime attribute, save as UpTime1")
        testvar_UpTime1 = await self.read_diags_attribute_expect_success(Clusters.GeneralDiagnostics.Attributes.UpTime)
        asserts.assert_greater(testvar_UpTime1, 0, "UpTime1 must be > 0")

        # Step 3 (Time Sync supported)
        if testvar_TimeSyncSupported:
            self.print_step(3, "Functional verifications when Time Synchronization is supported")

            self.print_step("3a", "Read UTCTime if TimeSyncSupported is true (already done as part of 1c, not redoing)")

            self.print_step("3b", "Wait for 1 second")
            await asyncio.sleep(1)

            self.print_step("3c", "Send a first TimeSnapshot command and verify")
            response = await self.send_time_snapshot_expect_success()
            logging.info(f"Step 3c: {response}")

            # Verify that the DUT sends a TimeSnapshotResponse with the following conditions met:
            #   - Value of PosixTimeMs field is not null.
            #   - Value of (SystemTimeMs field / 1000) is greater than or equal to UpTime1
            #   - PosixTimeMs field converted to a UTC timestamp is greater than or equal to UTCTime1 converted to a UTC timestamp.
            #
            # On success of prior verifications:
            #   - Save the value of the SystemTimeMs field as SystemTimeMs1.
            #   - Save the value of the PosixTimeMs field as PosixTimeMs1.
            asserts.assert_true(response.posixTimeMs != NullValue, "PosixTimeMs field of TimeSnapshotResponse must not be null")
            asserts.assert_greater_equal(response.systemTimeMs // 1000, testvar_UpTime1,
                                         "System time in milliseconds must be >= UpTime1")

            utc_from_posix = utc_datetime_from_posix_time_ms(posix_time_ms=response.posixTimeMs)
            utc_from_utctime1 = utc_datetime_from_matter_epoch_us(testvar_UTCTime1)

            asserts.assert_greater_equal(
                utc_from_posix, utc_from_utctime1, "PosixTimeMs field converted to a UTC timestamp must be >= than UTCTime1 converted to a UTC timestamp")

            testvar_SystemTimeMs1 = response.systemTimeMs
            testvar_PosixTimeMs1 = response.posixTimeMs

            self.print_step("3d", "Wait for 1 second")
            await asyncio.sleep(1)

            self.print_step("3e", "Send a second TimeSnapshot command and verify")

            response = await self.send_time_snapshot_expect_success()
            logging.info(f"Step 3e: {response}")

            # Verify that the DUT sends a TimeSnapshotResponse with the following fields:
            #   - Value of PosixTimeMs field is not null and greater than PosixTimeMs1.
            #   - Value of SystemTimeMs field is greater than SystemTimeMs1.

            asserts.assert_true(response.posixTimeMs != NullValue, "PosixTimeMs field of TimeSnapshotResponse must not be null")
            asserts.assert_greater(response.posixTimeMs, testvar_PosixTimeMs1,
                                   "POSIX time in milliseconds must be > PosixTimeMs1")
            asserts.assert_greater(response.systemTimeMs, testvar_SystemTimeMs1,
                                   "System time in milliseconds must be > SystemTimeMs1")

            self.print_step(4, "Skipped: Functional verifications for case when Time Synchronization is NOT supported")

        # Step 4 (Time Sync not supported)
        else:  # if not testvar_TimeSyncSupported:
            self.print_step(3, "Skipped: Functional verifications for case when Time Synchronization is supported")

            self.print_step(4, "Functional verifications when Time Synchronization is NOT supported")

            self.print_step("4a", "Send a first TimeSnapshot command and verify")
            response = await self.send_time_snapshot_expect_success()
            logging.info(f"Step 4a: {response}")

            # Verify that the DUT sends a TimeSnapshotResponse with the following fields:
            #   - Value of PosixTimeMs field is null.
            #   - Value of (SystemTimeMs field / 1000) is greater than UpTime1.
            #
            # On success of prior verifications, save the value of SystemTimeMs field as SystemTimeMs1.

            asserts.assert_true(response.posixTimeMs == NullValue, "PosixTimeMs field of TimeSnapshotResponse must be null")
            asserts.assert_greater_equal(response.systemTimeMs // 1000, testvar_UpTime1,
                                         "System time in milliseconds must be >= UpTime1")

            testvar_SystemTimeMs1 = response.systemTimeMs

            self.print_step("4b", "Wait for 1 second")
            await asyncio.sleep(1)

            self.print_step("4c", "Send a second TimeSnapshot command and verify")

            response = await self.send_time_snapshot_expect_success()
            logging.info(f"Step 4c: {response}")

            # Verify that the DUT sends a TimeSnapshotResponse with the following fields:
            #   - Value of PosixTimeMs field is null.
            #   - Value of SystemTimeMs field is greater than SystemTimeMs1.
            #
            # On success of prior verifications, save the value of SystemTimeMs field as SystemTimeMs1.

            asserts.assert_true(response.posixTimeMs == NullValue, "PosixTimeMs field of TimeSnapshotResponse must be null")
            asserts.assert_greater(response.systemTimeMs, testvar_SystemTimeMs1,
                                   "System time in milliseconds must be > SystemTimeMs1")


if __name__ == "__main__":
    default_matter_test_main()

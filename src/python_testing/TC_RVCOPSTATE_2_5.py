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

import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.event_attribute_reporting import ClusterAttributeChangeAccumulator
from chip.testing.matter_testing import AttributeMatcher, MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts


class RvcStatusEnum(enum.IntEnum):
    # TODO remove this class once InvalidInMode response code is implemented in python SDK
    Success = 0x0
    UnsupportedMode = 0x1
    GenericFailure = 0x2
    InvalidInMode = 0x3

def error_enum_to_text(error_enum):
    try:
        return f'{Clusters.RvcRunMode.Enums.ModeTag(error_enum).name} 0x{error_enum:02x}'
    except AttributeError:
        if error_enum == RvcStatusEnum.Success:
            return "Success(0x00)"
        elif error_enum == RvcStatusEnum.UnsupportedMode:
            return "UnsupportedMode(0x01)"
        elif error_enum == RvcStatusEnum.GenericFailure:
            return "GenericFailure(0x02)"
        elif error_enum == RvcStatusEnum.InvalidInMode:
            return "InvalidInMode(0x03)"

    raise AttributeError("Unknown Enum value")


class TC_RVCOPSTATE_2_5(MatterBaseTest):
    
    min_report_interval_sec = 0
    max_report_interval_sec = 30

    def desc_TC_RVCOPSTATE_2_5(self) -> str:
        return "[TC-RVCOPSTATE-2.5] Attributes with DUT as Server"

    def steps_TC_RVCOPSTATE_2_5(self) -> list[TestStep]:
        steps = [
            TestStep("1", "Commissioning, already done", is_commissioning=True),
            TestStep("2", "TH reads the SupportedModes attribute of the RVC Run Mode cluster",
            TestStep("3", "TH establishes a subscription to the CurrentMode attribute of the RVC Run Mode cluster of the DUT"),
            TestStep("4", "TH sends a RVC Run Mode cluster ChangeToMode command to the DUT with NewMode set to PIXIT.CLEANMODE"),
            TestStep("5", "Wait for DUT to leave dock and begin cleaning activities"),
            TestStep("6", "TH reads CurrentMode attribute of the RVC Run Mode cluster"),
            TestStep("7", "TH sends GoHome command to the DUT"),
            TestStep("8", "Wait for DUT to return to dock and complete docking-related activities"),
            TestStep("9", "TH reads CurrentMode attribute of the RVC Run Mode cluster"),
        ]
        return steps

    def pics_TC_RVCOPSTATE_2_5(self) -> list[str]:
        pics = [
            "RVCOPSTATE.S",
        ]
        return pics

    async def read_rvcrunm_attribute_expect_success(self, endpoint, attribute):
        cluster = Clusters.Objects.RvcRunMode
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute=attribute)

    async def send_change_to_mode_with_check(self, new_mode, expected_error):
        response = await self.send_change_to_mode_cmd(new_mode)
        asserts.assert_true(response.status == expected_error,
                            "Expected a ChangeToMode response status of %s, got %s" %
                            (error_enum_to_text(expected_error), error_enum_to_text(response.status)))

    @async_test_body
    async def test_TC_RVCOPSTATE_2_5(self):

        self.endpoint = self.get_endpoint()
        self.mode_clean = self.matter_test_config.global_test_params['PIXIT.RVCRUNM.CLEANMODE']

        # Commission DUT to TH
        self.step("1")

        self.print_step(1, "Commissioning, already done")

        # TH reads the SupportedModes attribute of the RVC Run Mode cluster
        self.step("2")
        supported_run_modes_dut = await self.read_rvcrunm_attribute_expect_success(endpoint=self.endpoint, attribute=attributes.SupportedModes)
        # Logging the SupportedModes Attribute output responses from the DUT:
        logging.info("SupportedModes: %s" % (supported_run_modes_dut))
        # As per SPEC, the SupportedModes must have at least two entries
        asserts.assert_greater_equal(len(supported_run_modes_dut), 2, "SupportedModes must have at least two entries!")

        # TH establishes a subscription to the CurrentMode attribute of the RVC Run Mode cluster of the DUT
        self.step("3")
        current_mode_accumulator = ClusterAttributeChangeAccumulator(
            Clusters.RvcRunMode,
            Clusters.RvcRunMode.Attributes.CurrentMode)
        await current_mode_accumulator.start(
            self.dut_node_id, endpoint=self.endpoint,
            min_interval_sec=self.min_report_interval_sec,
            max_interval_sec=self.max_report_interval_sec)

        # TH sends a RVC Run Mode cluster ChangeToMode command to the DUT with NewMode set to PIXIT.CLEANMODE
        self.step("4")
        await self.send_change_to_mode_with_check(self.mode_clean, RvcStatusEnum.Success)
        # This step is not described in the test plan, but it ought to be
        await self.read_current_mode_with_check(self.mode_clean)












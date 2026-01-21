#
#    Copyright (c) 2026 Project CHIP Authors
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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 0
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

"""
[TC-GRPKEY-2.2] Primary functionality with DUT as Server

This test case verifies the primary functionality of the Group Key Management cluster server.
The test case verifies for the DUT response when the below commands are sent with different
EpochKey, EpochKeyStartTime and GroupKeySetID values covering negative checks:
- KeySetWrite
- KeySetRead
- KeySetRemove
- KeySetReadAllIndices
"""

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_GRPKEY_2_2(MatterBaseTest):
    """
    [TC-GRPKEY-2.2] Primary functionality with DUT as Server

    Purpose:
        This test case verifies the primary functionality of the Group Key Management cluster server.
        The test case verifies for the DUT response when the below commands are sent with different
        EpochKey, EpochKeyStartTime and GroupKeySetID values covering negative checks.

    PICS:
        GRPKEY.S
    """

    # Test constants for GroupKeySet creation
    # Standard EpochKey values (16 bytes each)
    EPOCH_KEY_0 = bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf")
    EPOCH_KEY_1 = bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf")
    EPOCH_KEY_2 = bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf")

    # Alternative EpochKey values for step 17 (16 bytes each)
    EPOCH_KEY_0_ALT = bytes.fromhex("d3d1d2d3d4d5d6d7d8d9dadbdcdddedf")
    EPOCH_KEY_1_ALT = bytes.fromhex("d4d1d2d3d4d5d6d7d8d9dadbdcdddedf")
    EPOCH_KEY_2_ALT = bytes.fromhex("d5d1d2d3d4d5d6d7d8d9dadbdcdddedf")

    # Standard EpochStartTime values
    EPOCH_START_TIME_0 = 1
    EPOCH_START_TIME_1 = 18446744073709551613
    EPOCH_START_TIME_2 = 18446744073709551614

    # Alternative EpochStartTime values for step 17
    EPOCH_START_TIME_1_ALT = 17446744073709551613
    EPOCH_START_TIME_2_ALT = 17446744073709551614

    # Invalid EpochKey sizes for constraint testing
    EPOCH_KEY_INVALID_1_BYTE = bytes.fromhex("d0")
    EPOCH_KEY_INVALID_15_BYTES = bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcddde")
    EPOCH_KEY_INVALID_17_BYTES = bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedfd0")

    # GroupKeySecurityPolicy constant
    GROUP_KEY_SECURITY_POLICY = Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst

    # GroupKeySetID constants
    GROUP_KEY_SET_ID_PRIMARY = 0x01a
    GROUP_KEY_SET_ID_NON_EXISTENT = 0x01b
    GROUP_KEY_SET_ID_RESERVED_IPK = 0x0

    def desc_TC_GRPKEY_2_2(self):
        return "Primary functionality with DUT as Server"

    def steps_TC_GRPKEY_2_2(self):
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads MaxGroupKeysPerFabric attribute from GroupKeyManagement cluster on DUT. "
                        "Save the value as Max_GrpKey for future use."),
            TestStep(2, "TH sends KeySetWrite command in the GroupKeyManagement cluster to DUT on EP0 "
                        "with GroupKeySet fields with all 3 epoch keys"),
            TestStep(3, "TH sends KeySetRead command to GroupKeyManagement cluster with GroupKeySetID as 0x01a"),
            TestStep(4, "TH sends KeySetWrite command with EpochKey1 and EpochKey2 as null"),
            TestStep(5, "TH sends KeySetWrite command with only EpochKey2 as null"),
            TestStep(6, "TH sends KeySetWrite command with EpochKey0 as null (should fail)"),
            TestStep(7, "TH sends KeySetWrite command with EpochStartTime0 as null (should fail)"),
            TestStep(8, "TH sends KeySetWrite command with EpochStartTime0 set to 0 (should fail)"),
            TestStep(9, "TH sends KeySetWrite command with EpochKey1 null but EpochStartTime1 not null "
                        "(should fail)"),
            TestStep(10, "TH sends KeySetWrite command with EpochKey1 not null but EpochStartTime1 null "
                         "(should fail)"),
            TestStep(11, "TH sends KeySetWrite command with EpochStartTime1 earlier than EpochStartTime0 "
                         "(should fail)"),
            TestStep(12, "TH sends KeySetWrite command with EpochKey1 and EpochStartTime1 null when "
                         "EpochKey2 is not null (should fail)"),
            TestStep(13, "TH sends KeySetWrite command with EpochKey2 null but EpochStartTime2 not null "
                         "(should fail)"),
            TestStep(14, "TH sends KeySetWrite command with EpochKey2 not null but EpochStartTime2 null "
                         "(should fail)"),
            TestStep(15, "TH sends KeySetWrite command with EpochStartTime2 earlier than EpochStartTime1 "
                         "(should fail)"),
            TestStep(16, "TH sends KeySetWrite command with EpochKey0 having 1 byte value (< 16 bytes) "
                         "(should fail)"),
            TestStep("16a", "TH sends KeySetWrite command with EpochKeys having 15 bytes value "
                            "(< 16 bytes) (should fail)"),
            TestStep("16b", "TH sends KeySetWrite command with EpochKeys having 17 bytes value "
                            "(> 16 bytes) (should fail)"),
            TestStep(17, "TH sends KeySetWrite command with different EpochKeys and EpochStartTime values"),
            TestStep(18, "TH sends KeySetRead command to verify the updated values"),
            TestStep(19, "TH sends KeySetRead command with GroupKeySetID that does not exist (should fail)"),
            TestStep(20, "TH removes the Group key set by sending KeySetRemove command with "
                         "GroupKeySetID 0x01a"),
            TestStep(21, "TH sends KeySetWrite command to DUT until size of list contains Max_GrpKey entries"),
            TestStep(22, "TH again sends KeySetWrite command with any other GroupKeySetID "
                         "(should fail with RESOURCE_EXHAUSTED)"),
            TestStep(23, "TH sends KeySetReadAllIndices command to DUT"),
            TestStep(24, "TH removes Group key set with GroupKeySetID 0x0 (should fail)"),
            TestStep(25, "TH removes Group key set with GroupKeySetID that does not exist (should fail)"),
            TestStep(26, "TH removes all the existing GroupKeySetID that were added in Step 21"),
        ]

    def pics_TC_GRPKEY_2_2(self) -> list[str]:
        return ["GRPKEY.S"]

    @run_if_endpoint_matches(has_cluster(Clusters.GroupKeyManagement))
    async def test_TC_GRPKEY_2_2(self):
        """Test TC-GRPKEY-2.2"""

        # Pre-Conditions: Commissioning
        self.step(0)
        endpoint = self.get_endpoint()

        self.step(1)
        # Read MaxGroupKeysPerFabric attribute
        max_group_keys = await self.read_single_attribute_check_success(
            cluster=Clusters.GroupKeyManagement,
            attribute=Clusters.GroupKeyManagement.Attributes.MaxGroupKeysPerFabric,
            endpoint=endpoint,
        )

        max_grp_key = max_group_keys
        logger.info(f"MaxGroupKeysPerFabric: {max_grp_key}")

        self.step(2)
        # Send KeySetWrite command with all 3 epoch keys
        group_key_set_id = self.GROUP_KEY_SET_ID_PRIMARY
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
        )

        self.step(3)
        # Send KeySetRead command
        response = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=group_key_set_id),
        )
        asserts.assert_equal(
            response.groupKeySet.groupKeySetID,
            group_key_set_id,
            f"KeySetRead response should contain the requested GroupKeySetID {group_key_set_id}"
        )
        asserts.assert_equal(
            response.groupKeySet.groupKeySecurityPolicy,
            self.GROUP_KEY_SECURITY_POLICY,
            "KeySetRead response should preserve the GroupKeySecurityPolicy as kTrustFirst",
        )
        asserts.assert_equal(
            response.groupKeySet.epochKey0,
            NullValue,
            "KeySetRead response should return null for EpochKey0 as keys are not returned for security reasons"
        )
        asserts.assert_equal(
            response.groupKeySet.epochStartTime0,
            self.EPOCH_START_TIME_0,
            "KeySetRead response should return the correct EpochStartTime0 value that was written"
        )
        asserts.assert_equal(
            response.groupKeySet.epochKey1,
            NullValue,
            "KeySetRead response should return null for EpochKey1 as keys are not returned for security reasons"
        )
        asserts.assert_equal(
            response.groupKeySet.epochStartTime1,
            self.EPOCH_START_TIME_1,
            "KeySetRead response should return the correct EpochStartTime1 value that was written"
        )
        asserts.assert_equal(
            response.groupKeySet.epochKey2,
            NullValue,
            "KeySetRead response should return null for EpochKey2 as keys are not returned for security reasons"
        )
        asserts.assert_equal(
            response.groupKeySet.epochStartTime2,
            self.EPOCH_START_TIME_2,
            "KeySetRead response should return the correct EpochStartTime2 value that was written"
        )

        self.step(4)
        # Send KeySetWrite with EpochKey1 and EpochKey2 as null
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=NullValue,
            epochStartTime1=NullValue,
            epochKey2=NullValue,
            epochStartTime2=NullValue,
        )
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
        )
        self.step(5)
        # Send KeySetWrite with only EpochKey2 as null
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=NullValue,
            epochStartTime2=NullValue,
        )
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
        )
        self.step(6)
        # Send KeySetWrite with EpochKey0 as null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=NullValue,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(7)
        # Send KeySetWrite with EpochStartTime0 as null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=NullValue,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(8)
        # Send KeySetWrite with EpochStartTime0 set to 0 (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(9)
        # Send KeySetWrite with EpochKey1 null but EpochStartTime1 not null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=NullValue,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(10)
        # Send KeySetWrite with EpochKey1 not null but EpochStartTime1 null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=NullValue,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(11)
        # Send KeySetWrite with EpochStartTime1 earlier than EpochStartTime0 (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_1,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_0,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(12)
        # Send KeySetWrite with EpochKey1 and EpochStartTime1 null when EpochKey2 is not null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=NullValue,
            epochStartTime1=NullValue,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(13)
        # Send KeySetWrite with EpochKey2 null but EpochStartTime2 not null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=NullValue,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(14)
        # Send KeySetWrite with EpochKey2 not null but EpochStartTime2 null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=NullValue,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(15)
        # Send KeySetWrite with EpochStartTime2 earlier than EpochStartTime1 (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_0,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetWrite command with invalid parameters should be rejected with INVALID_COMMAND status"
        )

        self.step(16)
        # Send KeySetWrite with EpochKey0 having 1 byte value (< 16 bytes) (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_INVALID_1_BYTE,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ConstraintError,
            "KeySetWrite command with EpochKey size constraint violation should be rejected with CONSTRAINT_ERROR"
        )

        # Repeat for EpochKey1
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_INVALID_1_BYTE,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ConstraintError,
            "KeySetWrite command with EpochKey size constraint violation should be rejected with CONSTRAINT_ERROR"
        )

        # Repeat for EpochKey2
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_INVALID_1_BYTE,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ConstraintError,
            "KeySetWrite command with EpochKey size constraint violation should be rejected with CONSTRAINT_ERROR"
        )

        self.step("16a")
        # Send KeySetWrite with EpochKeys having 15 bytes value (< 16 bytes) (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_INVALID_15_BYTES,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ConstraintError,
            "KeySetWrite command with EpochKey size constraint violation should be rejected with CONSTRAINT_ERROR"
        )

        # Repeat for EpochKey1
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_INVALID_15_BYTES,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ConstraintError,
            "KeySetWrite command with EpochKey size constraint violation should be rejected with CONSTRAINT_ERROR"
        )

        # Repeat for EpochKey2
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_INVALID_15_BYTES,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ConstraintError,
            "KeySetWrite command with EpochKey size constraint violation should be rejected with CONSTRAINT_ERROR"
        )

        self.step("16b")
        # Send KeySetWrite with EpochKeys having 17 bytes value (> 16 bytes) (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_INVALID_17_BYTES,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ConstraintError,
            "KeySetWrite command with EpochKey size constraint violation should be rejected with CONSTRAINT_ERROR"
        )

        # Repeat for EpochKey1
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_INVALID_17_BYTES,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ConstraintError,
            "KeySetWrite command with EpochKey size constraint violation should be rejected with CONSTRAINT_ERROR"
        )

        # Repeat for EpochKey2
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_INVALID_17_BYTES,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ConstraintError,
            "KeySetWrite command with EpochKey size constraint violation should be rejected with CONSTRAINT_ERROR"
        )

        self.step(17)
        # Send KeySetWrite with different EpochKeys and EpochStartTime values
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0_ALT,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1_ALT,
            epochStartTime1=self.EPOCH_START_TIME_1_ALT,
            epochKey2=self.EPOCH_KEY_2_ALT,
            epochStartTime2=self.EPOCH_START_TIME_2_ALT,
        )
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
        )
        self.step(18)
        # Send KeySetRead command to verify the updated values
        response = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=group_key_set_id),
        )
        asserts.assert_equal(
            response.groupKeySet.groupKeySetID,
            group_key_set_id,
            f"KeySetRead response should contain the requested GroupKeySetID {group_key_set_id}"
        )
        asserts.assert_equal(
            response.groupKeySet.groupKeySecurityPolicy,
            self.GROUP_KEY_SECURITY_POLICY,
            "KeySetRead response should preserve the GroupKeySecurityPolicy as kTrustFirst",
        )
        asserts.assert_equal(
            response.groupKeySet.epochKey0,
            NullValue,
            "KeySetRead response should return null for EpochKey0 as keys are not returned for security reasons"
        )
        asserts.assert_equal(
            response.groupKeySet.epochStartTime0,
            self.EPOCH_START_TIME_0,
            "KeySetRead response should return the correct EpochStartTime0 value that was written"
        )
        asserts.assert_equal(
            response.groupKeySet.epochKey1,
            NullValue,
            "KeySetRead response should return null for EpochKey1 as keys are not returned for security reasons"
        )
        asserts.assert_equal(
            response.groupKeySet.epochStartTime1,
            self.EPOCH_START_TIME_1_ALT,
            "KeySetRead response should return the updated EpochStartTime1 value after KeySetWrite modification"
        )
        asserts.assert_equal(
            response.groupKeySet.epochKey2,
            NullValue,
            "KeySetRead response should return null for EpochKey2 as keys are not returned for security reasons"
        )
        asserts.assert_equal(
            response.groupKeySet.epochStartTime2,
            self.EPOCH_START_TIME_2_ALT,
            "KeySetRead response should return the updated EpochStartTime2 value after KeySetWrite modification"
        )

        self.step(19)
        # Send KeySetRead command with GroupKeySetID that does not exist (should fail)
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=self.GROUP_KEY_SET_ID_NON_EXISTENT),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.NotFound,
            "KeySetRead/KeySetRemove command with non-existent GroupKeySetID should be rejected with NOT_FOUND status"
        )

        self.step(20)
        # Remove the Group key set by sending KeySetRemove command
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=group_key_set_id),
        )
        self.step(21)
        # Send KeySetWrite command to DUT until size of list contains Max_GrpKey entries
        # The Group Key Set ID of 0 SHALL be reserved for managing the Identity Protection Key (IPK) on a given Fabric.
        # It SHALL NOT be possible to remove the IPK Key Set if it exists.

        # Use KeySetReadAllIndices to get the list of existing key set IDs
        key_set_indices_response = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetReadAllIndices(),
        )
        # The response is expected to have a 'groupKeySetIDs' field containing the list of used IDs
        existing_key_set_ids = set(getattr(key_set_indices_response, "groupKeySetIDs", []))
        # The Group Key Set ID of 0 SHALL be reserved for managing the IPK and must not be used for new key sets
        for i in range(1, max_grp_key):
            if i in existing_key_set_ids:
                continue
            group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
                groupKeySetID=i,
                groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
                epochKey0=self.EPOCH_KEY_0,
                epochStartTime0=self.EPOCH_START_TIME_0,
                epochKey1=self.EPOCH_KEY_1,
                epochStartTime1=self.EPOCH_START_TIME_1,
                epochKey2=self.EPOCH_KEY_2,
                epochStartTime2=self.EPOCH_START_TIME_2,
            )
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        self.step(22)
        # Send KeySetWrite command with any other GroupKeySetID (should fail with RESOURCE_EXHAUSTED)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=max_grp_key + 1,
            groupKeySecurityPolicy=self.GROUP_KEY_SECURITY_POLICY,
            epochKey0=self.EPOCH_KEY_0,
            epochStartTime0=self.EPOCH_START_TIME_0,
            epochKey1=self.EPOCH_KEY_1,
            epochStartTime1=self.EPOCH_START_TIME_1,
            epochKey2=self.EPOCH_KEY_2,
            epochStartTime2=self.EPOCH_START_TIME_2,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.ResourceExhausted,
            "KeySetWrite command when MaxGroupKeysPerFabric limit is reached should be rejected with RESOURCE_EXHAUSTED"
        )

        self.step(23)
        # Send KeySetReadAllIndices command
        response = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetReadAllIndices(),
        )
        asserts.assert_equal(
            len(response.groupKeySetIDs),
            max_grp_key,
            f"KeySetReadAllIndices should return exactly {max_grp_key} GroupKeySetIDs matching MaxGroupKeysPerFabric,"
            f"but got {len(response.groupKeySetIDs)}"
        )

        self.step(24)
        # Remove Group key set with GroupKeySetID 0x0 (should fail)
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=self.GROUP_KEY_SET_ID_RESERVED_IPK),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.InvalidCommand,
            "KeySetRemove command with reserved GroupKeySetID 0x0 should be rejected with INVALID_COMMAND status"
        )

        self.step(25)
        # Remove Group key set with GroupKeySetID that does not exist (should fail)
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=self.GROUP_KEY_SET_ID_NON_EXISTENT),
            )
        asserts.assert_equal(
            context.exception.status,
            Status.NotFound,
            "KeySetRead/KeySetRemove command with non-existent GroupKeySetID should be rejected with NOT_FOUND status"
        )

        self.step(26)
        # Remove all the existing GroupKeySetID that were added in Step 21
        # The Group Key Set ID of 0 SHALL be reserved for managing the Identity Protection Key (IPK) on a given Fabric.
        # It SHALL NOT be possible to remove the IPK Key Set if it exists.
        for i in range(1, max_grp_key):
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=i),
            )


if __name__ == "__main__":
    default_matter_test_main()

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
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

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

    def desc_TC_GRPKEY_2_2(self):
        return "Primary functionality with DUT as Server"

    def steps_TC_GRPKEY_2_2(self):
        return [
            TestStep(0, "Comissioning, already done", is_commissioning=True),
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

    def test_TC_GRPKEY_2_2(self) -> list[str]:
        return ["GRPKEY.S"]

    @async_test_body
    async def test_TC_GRPKEY_2_2(self):
        """Test TC-GRPKEY-2.2"""

        # Pre-Conditions: Comissioning
        self.step(0)
        if self.matter_test_config.endpoint is None:
            self.matter_test_config.endpoint = 0

        self.step(1)
        # Read MaxGroupKeysPerFabric attribute
        max_group_keys = await self.read_single_attribute_check_success(
            cluster=Clusters.GroupKeyManagement,
            attribute=Clusters.GroupKeyManagement.Attributes.MaxGroupKeysPerFabric,
            endpoint=self.matter_test_config.endpoint,
        )

        max_grp_key = max_group_keys
        logger.info(f"MaxGroupKeysPerFabric: {max_grp_key}")

        self.step(2)
        # Send KeySetWrite command with all 3 epoch keys
        group_key_set_id = 0x01a
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.matter_test_config.endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
        )

        self.step(3)
        # Send KeySetRead command
        response = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.matter_test_config.endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=group_key_set_id),
        )
        asserts.assert_equal(response.groupKeySet.groupKeySetID, group_key_set_id, "GroupKeySetID mismatch")
        asserts.assert_equal(
            response.groupKeySet.groupKeySecurityPolicy,
            Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            "GroupKeySecurityPolicy mismatch",
        )
        asserts.assert_equal(response.groupKeySet.epochKey0, NullValue, "EpochKey0 should be null")
        asserts.assert_equal(response.groupKeySet.epochStartTime0, 1, "EpochStartTime0 mismatch")
        asserts.assert_equal(response.groupKeySet.epochKey1, NullValue, "EpochKey1 should be null")
        asserts.assert_equal(response.groupKeySet.epochStartTime1, 18446744073709551613, "EpochStartTime1 mismatch")
        asserts.assert_equal(response.groupKeySet.epochKey2, NullValue, "EpochKey2 should be null")
        asserts.assert_equal(response.groupKeySet.epochStartTime2, 18446744073709551614, "EpochStartTime2 mismatch")

        self.step(4)
        # Send KeySetWrite with EpochKey1 and EpochKey2 as null
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=NullValue,
            epochStartTime1=NullValue,
            epochKey2=NullValue,
            epochStartTime2=NullValue,
        )
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.matter_test_config.endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
        )

        self.step(5)
        # Send KeySetWrite with only EpochKey2 as null
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=NullValue,
            epochStartTime2=NullValue,
        )
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.matter_test_config.endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
        )

        self.step(6)
        # Send KeySetWrite with EpochKey0 as null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=NullValue,
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(7)
        # Send KeySetWrite with EpochStartTime0 as null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=NullValue,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(8)
        # Send KeySetWrite with EpochStartTime0 set to 0 (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=0,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(9)
        # Send KeySetWrite with EpochKey1 null but EpochStartTime1 not null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=NullValue,
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(10)
        # Send KeySetWrite with EpochKey1 not null but EpochStartTime1 null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=NullValue,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(11)
        # Send KeySetWrite with EpochStartTime1 earlier than EpochStartTime0 (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=18446744073709551613,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=1,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(12)
        # Send KeySetWrite with EpochKey1 and EpochStartTime1 null when EpochKey2 is not null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=NullValue,
            epochStartTime1=NullValue,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(13)
        # Send KeySetWrite with EpochKey2 null but EpochStartTime2 not null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=NullValue,
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(14)
        # Send KeySetWrite with EpochKey2 not null but EpochStartTime2 null (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=NullValue,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(15)
        # Send KeySetWrite with EpochStartTime2 earlier than EpochStartTime1 (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=1,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(16)
        # Send KeySetWrite with EpochKey0 having 1 byte value (< 16 bytes) (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR status")

        # Repeat for EpochKey1
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR status")

        # Repeat for EpochKey2
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR status")

        self.step("16a")
        # Send KeySetWrite with EpochKeys having 15 bytes value (< 16 bytes) (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcddde"),  # 15 bytes
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR status")

        # Repeat for EpochKey1
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcddde"),  # 15 bytes
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR status")

        # Repeat for EpochKey2
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcddde"),  # 15 bytes
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR status")

        self.step("16b")
        # Send KeySetWrite with EpochKeys having 17 bytes value (> 16 bytes) (should fail)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedfd0"),  # 17 bytes
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR status")

        # Repeat for EpochKey1
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedfd0"),  # 17 bytes
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR status")

        # Repeat for EpochKey2
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedfd0"),  # 17 bytes
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ConstraintError, "Expected CONSTRAINT_ERROR status")

        self.step(17)
        # Send KeySetWrite with different EpochKeys and EpochStartTime values
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=group_key_set_id,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d3d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d4d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=17446744073709551613,
            epochKey2=bytes.fromhex("d5d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=17446744073709551614,
        )
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.matter_test_config.endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
        )

        self.step(18)
        # Send KeySetRead command to verify the updated values
        response = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.matter_test_config.endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=group_key_set_id),
        )
        asserts.assert_equal(response.groupKeySet.groupKeySetID, group_key_set_id, "GroupKeySetID mismatch")
        asserts.assert_equal(
            response.groupKeySet.groupKeySecurityPolicy,
            Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            "GroupKeySecurityPolicy mismatch",
        )
        asserts.assert_equal(response.groupKeySet.epochKey0, NullValue, "EpochKey0 should be null")
        asserts.assert_equal(response.groupKeySet.epochStartTime0, 1, "EpochStartTime0 mismatch")
        asserts.assert_equal(response.groupKeySet.epochKey1, NullValue, "EpochKey1 should be null")
        asserts.assert_equal(response.groupKeySet.epochStartTime1, 17446744073709551613, "EpochStartTime1 mismatch")
        asserts.assert_equal(response.groupKeySet.epochKey2, NullValue, "EpochKey2 should be null")
        asserts.assert_equal(response.groupKeySet.epochStartTime2, 17446744073709551614, "EpochStartTime2 mismatch")

        self.step(19)
        # Send KeySetRead command with GroupKeySetID that does not exist (should fail)
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetRead(groupKeySetID=0x01b),
            )
        asserts.assert_equal(context.exception.status, Status.NotFound, "Expected NOT_FOUND status")

        self.step(20)
        # Remove the Group key set by sending KeySetRemove command
        await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.matter_test_config.endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=group_key_set_id),
        )

        self.step(21)
        # Send KeySetWrite command to DUT until size of list contains Max_GrpKey entries
        # The Group Key Set ID of 0 SHALL be reserved for managing the Identity Protection Key (IPK) on a given Fabric.
        # It SHALL NOT be possible to remove the IPK Key Set if it exists.
        for i in range(1, max_grp_key):
            group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
                groupKeySetID=i,
                groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
                epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
                epochStartTime0=1,
                epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
                epochStartTime1=18446744073709551613,
                epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
                epochStartTime2=18446744073709551614,
            )
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )

        self.step(22)
        # Send KeySetWrite command with any other GroupKeySetID (should fail with RESOURCE_EXHAUSTED)
        group_key_set = Clusters.GroupKeyManagement.Structs.GroupKeySetStruct(
            groupKeySetID=max_grp_key + 1,
            groupKeySecurityPolicy=Clusters.GroupKeyManagement.Enums.GroupKeySecurityPolicyEnum.kTrustFirst,
            epochKey0=bytes.fromhex("d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime0=1,
            epochKey1=bytes.fromhex("d1d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime1=18446744073709551613,
            epochKey2=bytes.fromhex("d2d1d2d3d4d5d6d7d8d9dadbdcdddedf"),
            epochStartTime2=18446744073709551614,
        )
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetWrite(groupKeySet=group_key_set),
            )
        asserts.assert_equal(context.exception.status, Status.ResourceExhausted, "Expected RESOURCE_EXHAUSTED status")

        self.step(23)
        # Send KeySetReadAllIndices command
        response = await self.send_single_cmd(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=self.matter_test_config.endpoint,
            cmd=Clusters.GroupKeyManagement.Commands.KeySetReadAllIndices(),
        )
        asserts.assert_equal(
            len(response.groupKeySetIDs),
            max_grp_key,
            f"Expected {max_grp_key} GroupKeySetIDs, got {len(response.groupKeySetIDs)}",
        )

        self.step(24)
        # Remove Group key set with GroupKeySetID 0x0 (should fail)
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=0x0),
            )
        asserts.assert_equal(context.exception.status, Status.InvalidCommand, "Expected INVALID_COMMAND status")

        self.step(25)
        # Remove Group key set with GroupKeySetID that does not exist (should fail)
        with asserts.assert_raises(InteractionModelError) as context:
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=0x01b),
            )
        asserts.assert_equal(context.exception.status, Status.NotFound, "Expected NOT_FOUND status")

        self.step(26)
        # Remove all the existing GroupKeySetID that were added in Step 21
        # The Group Key Set ID of 0 SHALL be reserved for managing the Identity Protection Key (IPK) on a given Fabric.
        # It SHALL NOT be possible to remove the IPK Key Set if it exists.
        for i in range(1, max_grp_key):
            await self.send_single_cmd(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=self.matter_test_config.endpoint,
                cmd=Clusters.GroupKeyManagement.Commands.KeySetRemove(groupKeySetID=i),
            )


if __name__ == "__main__":
    default_matter_test_main()

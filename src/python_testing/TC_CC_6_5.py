import asyncio
from typing import List
import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_infrastructure.chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts
kCCAttributeValueIDs = [0x0001, 0x0003, 0x0004, 0x0007, 0x4000, 0x4001, 0x4002, 0x4003, 0x4004]
class TC_CC_6_5(MatterBaseTest):
    def desc_TC_CC_6_5(self) -> str:
        """Returns a description of this test"""
        return "4.2.18.[TC_CC_6_5] This test case verifies Color Temperature StartUpColorTemperatureMireds functionality of the Color Control Cluster server."
    def steps_TC_CC_6_5(self) -> list[TestStep]:
        steps = [
            TestStep("0", "Commissioning, already done", is_commissioning=True),
            TestStep("0a", "TH writes 0x00 to the Options attribute"),
            TestStep("0b", "TH sends On command to DUT"),
            TestStep("0c", "TH reads ColorTemperatureMireds attribute from DUT."),
            TestStep("0d", "TH reads ColorTempPhysicalMinMireds attribute from DUT."),
            TestStep("0e", "TH reads ColorTempPhysicalMaxMireds attribute from DUT."),
            TestStep("1", "TH reads from the DUT the StartUpColorTemperatureMireds attribute"),
            TestStep("2a", "TH writes to StartUpColorTemperatureMireds attribute with value StartUpColorTemperatureMireds"),
            TestStep("2b", "TH reads from the DUT the StartUpColorTemperatureMireds attribute"),
            TestStep("3a", "Power off DUT"),
            TestStep("3b", "Power on DUT"),
            TestStep("4a", "TH reads from the DUT the StartUpColorTemperatureMireds attribute"),
            TestStep("4b", "TH reads from the DUT the ColorTemperatureMireds attribute from DUT."),
            TestStep("5a", "TH reads ColorMode attribute from DUT."),
            TestStep("5b", "TH reads EnhancedColorMode attribute from DUT.")
        ]
        return steps
    @async_test_body
    async def setup_test(self):
        super().setup_test()
        # Pre-Condition: Commissioning
        self.step("0")
        self.TH1 = self.default_controller

    @async_test_body
    async def test_TC_CC_6_5(self):
        cluster = Clusters.Objects.ColorControl
        attributes = cluster.Attributes
        # Step 0a: Write 0x00 to Options attribute
        self.step("0a")
        await self.TH1.WriteAttribute(self.dut_node_id, [(attributes.Options, 0x00)])

        # Step 0b: Send On command
        self.step("0b")
        response = await self.TH1.WriteAttribute(self.dut_node_id, [(Clusters.Objects.OnOff.Attributes.OnOff, True)])

        # Verify DUT responds with a successful (value 0x00) status response
        asserts.assert_equal(response.status, Status.Success, "DUT did not respond with a successful status response")

        # Step 0c: Read ColorTemperatureMireds
        self.step("0c")
        color_temp = await self.TH1.ReadAttribute(self.dut_node_id, [(attributes.ColorTemperatureMireds)])

        # Verify that the DUT response contains with ColorTemperatureMireds attribute value.
        asserts.assert_equal(color_temp.value, attributes.colorTemperatureMireds,
                              "DUT responded with expected ColorTemperatureMireds attribute value")

        # Step 0d: Read ColorTempPhysicalMinMireds
        self.step("0d")
        min_mireds = await self.TH1.ReadAttribute(self.dut_node_id, [(attributes.ColorTempPhysicalMinMireds)])

        # Verify that the DUT response contains with ColorTempPhysicalMinMireds attribute value.
        asserts.assert_equal(min_mireds.value, attributes.ColorTempPhysicalMinMireds,
                              "DUT responded with expected ColorTempPhysicalMinMireds attribute value")

        # Step 0e: Read ColorTempPhysicalMaxMireds
        self.step("0e")
        max_mireds = await self.TH1.ReadAttribute(self.dut_node_id, [(attributes.ColorTempPhysicalMaxMireds)])

        # Verify that the DUT response contains with ColorTempPhysicalMaxMireds attribute value.
        asserts.assert_equal(max_mireds.value, attributes.ColorTempPhysicalMaxMireds,
                              "DUT responded with expected ColorTempPhysicalMaxMireds attribute value")

        # Step 1: Read StartUpColorTemperatureMireds
        self.step("1")
        startup_color_temp = await self.TH1.ReadAttribute(self.dut_node_id, [(attributes.StartUpColorTemperatureMireds)])
        
        # Verify that the DUT response contains an uint16 [Min:1 Max:0xfeff or null]
        asserts.assert_true(startup_color_temp.value is None or isinstance(startup_color_temp.value, int),
                            "StartUpColorTemperatureMireds attribute value is not a valid uint16")
        asserts.assert_in_range(startup_color_temp.value, 1, 0xfeff, 
                                "StartUpColorTemperatureMireds attribute value is out of range")

        # Step 2a: Write StartUpColorTemperatureMireds
        self.step("2a")
        test_value = min_mireds + ((max_mireds - min_mireds) // 2)  # Use midpoint value
        response = await self.TH1.WriteAttribute(self.dut_node_id, [(attributes.StartUpColorTemperatureMireds, test_value)])

        # Verify DUT responds with a successful (value 0x00) status response.
        asserts.assert_equal(response.status, Status.Success, "DUT respondedwith a successful status response")


        # Step 2b: Read StartUpColorTemperatureMireds to verify write
        self.step("2b")
        verify_startup = await self.TH1.ReadAttribute(self.dut_node_id, [(attributes.StartUpColorTemperatureMireds)])
        
        # Verify that the DUT response contains StartUpColorTemperatureMireds that matches the StartUpColorTemperatureMireds 
        # set in Step 2a
        asserts.assert_equal(verify_startup, test_value, "StartUpColorTemperatureMireds write verification failed")

        # Step 3a & 3b: Power cycle handled by test harness
        self.step("3a")
        await self.TH1.PowerCycle()

        # Step 3b: Wait for DUT to boot up  
        self.step("3b")
        await asyncio.sleep(10)

        # Step 4a: Read StartUpColorTemperatureMireds after power cycle
        self.step("4a")
        post_cycle_startup = await self.TH1.ReadAttribute(self.dut_node_id, [(attributes.StartUpColorTemperatureMireds)])

        # Verify that the DUT response indicates that the StartUpColorTemperatureMireds
        #  attribute matches the StartUpColorTemperatureMireds set in Step 2a
        asserts.assert_equal(post_cycle_startup, test_value, 
                             "StartUpColorTemperatureMireds matches StartUpColorTemperatureMireds after power cycle")

        # Step 4b: TH reads from the DUT the ColorTemperatureMireds attribute from DUT.
        self.step("4b")
        post_cycle_color = await self.TH1.ReadAttribute(self.dut_node_id, [(attributes.ColorTemperatureMireds)])

        # Verify that the DUT response indicates that the ColorTemperatureMireds 
        # attribute is StartUpColorTemperatureMireds
        asserts.assert_equal(post_cycle_color, test_value, 
                             "ColorTemperatureMireds matches StartUpColorTemperatureMireds after power cycle")

        # Step 5a: Read ColorMode
        self.step("5a")
        color_mode = await self.TH1.ReadAttribute(self.dut_node_id, [(attributes.ColorMode)])

        # Verify that the DUT response indicates that the ColorTemperatureMireds 
        # attribute is StartUpColorTemperatureMireds
        asserts.assert_equal(color_mode, attributes.ColorMode.StartUpColorTemperatureMireds,
                             "ColorMode attribute is StartUpColorTemperatureMireds")

        # Step 5b: Read EnhancedColorMode
        self.step("5b")
        enhanced_mode = await self.TH1.ReadAttribute(self.dut_node_id, [(attributes.EnhancedColorMode)])

        # Verify that the DUT response indicates that the EnhancedColorMode attribute
        #  has the expected value 2 (ColorTemperatureMireds).
        asserts.assert_equal(enhanced_mode, attributes.EnhancedColorMode.ColorTemperatureMireds,
                             "EnhancedColorMode attribute is ColorTemperatureMireds")

if __name__ == "__main__":
    default_matter_test_main()
    
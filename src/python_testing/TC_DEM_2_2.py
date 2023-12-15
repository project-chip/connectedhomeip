import logging
import datetime
import pytz
import time

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)
    
class TC_DEM_2_2(MatterBaseTest):
    
    async def read_dem_attribute_exepct_success(self, endpoint, attribute):
        full_attr= getattr(Clusters.DeviceEnergyManagement.Attributes, attribute)
        cluster = Clusters.Objects.DeviceEnergyManagement
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute= full_attr)
    
   
    async def send_power_adj_request_command(self, endpoint: int = 0, power: int = None, duration: int = 0 timedRequestTimeoutMs: int = 60000, expected_status: Status=Status.Success):
        try:
            await self.send_single_cmd(cmd=Clusters.DeviceEnergyMangement.Commands.PowerAdjustRequest(power=power, duration=duration), endpoint=1, timedRequestTimeoutMs=timedRequestTimeoutMs)
        
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
            pass
        
    
    async def send_cncl_power_adj_request_command(self, endpoint: int = 0,  timedRequestTimeoutMs: int = 60000, expected_status: Status=Status.Success):
    try:
        await self.send_single_cmd(cmd=Clusters.DeviceEnergyMangement.Commands.CanceelPowerAdjustRequest(), endpoint=1, timedRequestTimeoutMs=timedRequestTimeoutMs)
    
    except InteractionModelError as e:
        asserts.assert_equal(e.status, expected_status, "Unexpected error returned")
        pass
        

           
    @async_test_body 
    async def test_TC_DEM_2_2(self):
        
        # Part 1
        self.print_step(1, "Commissioning, already done")
        
        #Part 2 TODO Requires TestEventTriggers
        
        #Part 3 TODO Requires TestEventTriggers
        """These tests will not work without the test event triggers."""
        # self.print_step('3a', "TH reads from the DUT ESAType")
        # esa_type = await self.read_dem_attribute_exepct_success(endpoint = 1, attribute='ESAType')
        # asserts.assert_equal(esa_type, 5, f'ESAType should be 5, but is actually {esa_type}')
        
        # self.print_step('3b', "TH reads from the DUT ESACanGenerate")
        # esa_can_generate = await self.read_dem_attribute_exepct_success(endpoint = 1, attribute='ESACanGenerate')
        # asserts.assert_equal(esa_can_generate, True, f'ESACanGenerate should be 5, but is actually {esa_can_generate}')
        
        # self.print_step('3c', "TH reads from the DUT ESAState")
        # esa_state = await self.read_dem_attribute_exepct_success(endpoint = 1, attribute='ESAState')
        # asserts.assert_equal(esa_state, 1, f'ESAState should be 1, but is actually {esa_state}')
        
        # self.print_step('3d', "TH reads from the DUT AbsMinPower")
        # abs_min_power = await self.read_dem_attribute_exepct_success(endpoint = 1, attribute='AbsMinPower')
        # asserts.assert_equal(abs_min_power, -2000000, f'AbsMinPower should be -2000000, but is actually {abs_min_power}')
        
        # self.print_step('3e', "TH reads from the DUT AbsMaxPower")
        # abs_max_power = await self.read_dem_attribute_exepct_success(endpoint = 1, attribute='AbsMaxPower')
        # asserts.assert_equal(abs_max_power, 3000000, f'AbsMaxPowershould be 3000000, but is actually {abs_max_power}')
        
        # self.print_step('3f', "TH reads from the DUT PowerAdjustmentCapability")
        # power_adj_cap = await self.read_dem_attribute_exepct_success(endpoint = 1, attribute='PowerAdjustmentCapability')
        # asserts.assert_equal(power_adj_cap, 3000000, f'PowerAdjustmentCapability should be 3000000, but is actually {abs_max_power}')
        
        #Part 4
        self.print_step(4, "TH sends command PowerAdjustRequest with power= -2000000 and duration =120s")
        await self.send_power_adj_request_command(endpoint = 1, power = -2000000, duration = 120)
        
        self.print_step('4a', "TH reads from the DUT ESAState")
        esa_state = await self.read_dem_attribute_exepct_success(endpoint = 1, attribute='ESAState')
        asserts.assert_equal(esa_state, 4, f'ESAState should be 4, but is actually {esa_state}')
        
        #Part 5
        self.print_step(5, "Wait 2 minutes")
        time.sleep(120)
        
        #Part 6
        self.print_step(6, "TH sends command PowerAdjustRequest with power= 3000000 and duration =120s")
        await self.send_power_adj_request_command(endpoint = 1, power = -2000000, duration = 120)
        
        #Part 7
        self.print_step(7, "Wait 1 minute")
        time.sleep(60)
        
        #Part 8
        self.print_step(8, "TH sends command CancelPowerAdjustRequest")
        await self.send_cncl_power_adj_request_command(endpoint = 1)
        
        #Part 9
        self.print_step(9, "TH sends command CancelPowerAdjustRequest")
        cncl_power_adj = await self.send_cncl_power_adj_request_command(endpoint = 1)
        asserts.assert_equal(cncl_power_adj, Status.Failure, f'CancelPowerAdjustment command should fail, but does not')
        
        #Part 10
        self.print_step(10, "TH sends command PowerAdjustRequest with power = 5000000 and duration = 120")
        power_adj = await self.send_power_adj_request_command(endpoint = 1, power = 5000000, duration = 120)
        asserts.assert_equal(power_adj, Status.Failure, f'PowerAdjustment command should fail, but does not')
        
        #Part 11
        self.print_step(11, "TH sends command PowerAdjustRequest with power = 3000000 and duration = 10000")
        power_adj = await self.send_power_adj_request_command(endpoint = 1, power = 3000000, duration = 10000)
        asserts.assert_equal(power_adj, Status.Failure, f'PowerAdjustment command should fail, but does not')
        
        #Part 12
        self.print_step(12, "TH sends command PowerAdjustRequest with power = 3000000 and duration = 120")
        power_adj = await self.send_power_adj_request_command(endpoint = 1, power = 3000000, duration = 120)
        asserts.assert_equal(power_adj, Status.Success, f'PowerAdjustment command should Succeed but does not')
        
        #Part 13
        self.print_step(5, "Wait 1 minute")
        time.sleep(60)
        
        #Part 14
        self.print_step(12, "TH sends command PowerAdjustRequest with power = 3000000 and duration = 120")
        power_adj = await self.send_power_adj_request_command(endpoint = 1, power = 3000000, duration = 120)
        
        #Part 15 TODO RequiresTestEventTriggers
        
        #Part 16 TODO Requires TestEventTriggers
        
        #Part 17 TODO Requires TestEventTriggers
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        #Part 5 TODO Requires TestEventTriggers
        

if __name__ == "__main__":
    default_matter_test_main()
        
        
        
        
        
   
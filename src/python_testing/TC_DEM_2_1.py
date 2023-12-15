
import logging

import chip.clusters as Clusters
from chip.interaction_model import Status
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts

#List of attributes tests in the following format: ['Attribute name',read/write, Value to Write, ExpectedValue, Test step] 
Attribute_names = [
    ['ESAType', 'r', None, None, ('print_step', (2, 'TH reads from DUT the ESAType attribute'))],
    ['ESACanGenerate','r', None, None, ('print_step',(3, 'Th reads from the DUT the ESACanGenerate attribute'))],
    ['ESAState','r', None, None, ('print_step', (4, 'Th reads from the DUT the ESAState attribute'))],
    ['AbsMinPower', 'r', None, None, ('print_step', (5, 'Th reads from the DUT the AbsMinPower attribute'))],
    ['AbsMaxPower','r', None, None, ('print_step', (6, 'Th reads from the DUT the AbsMaxPower attribute'))],
    ['PowerAdjustmentCapability','r', None, None, ('print_step', (7, 'Th reads from the DUT the PowerAdjustmentCapability attribute'))],
    ['Forecast','r', None, None, ('print_step', (8, 'Th reads from the DUT the Forecast attribute'))],

]
    
class DemTest(MatterBaseTest):
   
    async def read_dem_attribute_exepct_success(self, endpoint, attribute):
        full_attr= getattr(Clusters.DeviceEnergyManagement.Attributes, attribute)
        cluster = Clusters.Objects.DeviceEnergyManagement
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=cluster, attribute= full_attr)
   
    @async_test_body
    async def test_TC_DEM_2_1(self):
        for attribute_name, rw, value_to_write, expected_value, step in Attribute_names: 
            function, args = step
            getattr(self, function)(*args)
            
            await self.read_dem_attribute_exepct_success(endpoint = 1, attribute=attribute_name)
            #asserts.assert_not_equal(attribute, None, f"{attribute_name} should return a value but instead returns None")
        
        
if __name__ == "__main__":
    default_matter_test_main()

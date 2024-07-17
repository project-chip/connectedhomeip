#
#    Copyright (c) 2024 Project CHIP Authors
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

# test-runner-runs: run1
# test-runner-run/run1/app: ${ALL_CLUSTERS_APP}
# test-runner-run/run1/factoryreset: True
# test-runner-run/run1/quiet: True
# test-runner-run/run1/app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
# test-runner-run/run1/script-args: --storage-path admin_storage.json --commissioning-method on-network --discriminator 1234 --passcode 20202021 --trace-to json:${TRACE_TEST_JSON}.json --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto

import logging

import chip.clusters as Clusters
from chip.clusters.Types import NullValue
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from mobly import asserts


class TC_PWRTL_2_1(MatterBaseTest):

    def pics_TC_PWRTL_2_1(self) -> list[str]:
        return ["PWRTL.S"]

    @async_test_body
    async def test_TC_PWRTL_2_1(self):

        attributes = Clusters.PowerTopology.Attributes

        endpoint = self.user_params.get("endpoint", 1)
        
        powertop_attr_list = Clusters.Objects.PowerTopology.Attributes.AttributeList
        powertop_cluster = Clusters.Objects.PowerTopology
        attribute_list = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=powertop_cluster, attribute=powertop_attr_list)
        avail_endpoints_attr_id = Clusters.Objects.PowerTopology.Attributes.ActiveEndpoints.attribute_id
        act_endpoints_attr_id = Clusters.Objects.PowerTopology.Attributes.AvailableEndpoints.attribute_id

        self.print_step(1, "Commissioning, already done")

        self.print_step(2, "Read AvailableAttributes attribute")
        if avail_endpoints_attr_id in attribute_list:
            available_endpoints = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=Clusters.Objects.PowerTopology, attribute=attributes.AvailableEndpoints)

            if available_endpoints == []:
                logging.info("AvailableEndpoints is an empty list")
            else:
                logging.info("AvailableEndpoints: %s" % (available_endpoints))
                asserts.assert_less_equal(len(available_endpoints), 21,
                                          "AvailableEndpoints length %d must be less than 21!" % len(available_endpoints))

        else:
            logging.info('Skipping test as available endpoints attribute ID not in attribute list on DUT')
            return

        self.print_step(3, "Read ActiveEndpoints attribute")
        if act_endpoints_attr_id in attribute_list:
            active_endpoints = await self.read_single_attribute_check_success(endpoint=endpoint, cluster=Clusters.Objects.PowerTopology,  attribute=attributes.ActiveEndpoints)
            logging.info("ActiveEndpoints: %s" % (active_endpoints))

            if available_endpoints == NullValue or available_endpoints == []:
                asserts.assert_true(active_endpoints == NullValue or active_endpoints == [],
                                    "ActiveEndpoints should be null when AvailableEndpoints is null: %s" % active_endpoints)

        else:
            logging.info('Skipping test step 3 as active endpoints attribute ID not in attribute list on DUT')
            return


if __name__ == "__main__":
    default_matter_test_main()

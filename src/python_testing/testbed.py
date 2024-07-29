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

import chip.clusters as Clusters
import chip.clusters.ClusterObjects as ClusterObjects
from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main, per_node_test
from chip.interaction_model import Status
from basic_composition_support import BasicCompositionTests
import global_attribute_ids


class testbed(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def setup_class(self):
        self.user_params["use_pase_only"] = False
        super().setup_class()
        await self.setup_class_helper()

    @per_node_test
    async def test_testbed(self):
        for endpoint_id, endpoint in self.endpoints.items():
            for cluster_type, cluster in endpoint.items():
                if global_attribute_ids.cluster_id_type(cluster_type.id) != global_attribute_ids.ClusterIdType.kStandard:
                    continue

            all_attrs = set(list(ClusterObjects.ALL_ATTRIBUTES[cluster_type.id].keys()))
            dut_attrs = set(cluster[cluster_type.Attributes.AttributeList])

            unsupported = [id for id in list(all_attrs - dut_attrs) if global_attribute_ids.attribute_id_type(id) == global_attribute_ids.AttributeIdType.kStandardNonGlobal]

            if unsupported:
                await self.read_single_attribute_expect_error(endpoint=endpoint_id, cluster=cluster_type, attribute=ClusterObjects.ALL_ATTRIBUTES[cluster_type.id][unsupported[0]], error=Status.UnsupportedAttribute)



if __name__ == "__main__":
    default_matter_test_main()
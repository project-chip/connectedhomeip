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
import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, async_test_body, default_matter_test_main


class TestCheckSoftwareVersion(MatterBaseTest):

    @async_test_body
    async def setup_class(self):
        super().setup_class()
        logging.info("This log has started")

    @async_test_body
    async def test_ota_image_version(self):
        self.print_step(0, "Commissioning - already done")
        expected_software_version = self.matter_test_config.global_test_params['SOFTWAREVERSION']

        self.print_step(1, f"Verify cluster version is: {expected_software_version}")
        current_software_version = await self.read_single_attribute_check_success(cluster=Clusters.BasicInformation, attribute=Clusters.BasicInformation.Attributes.SoftwareVersion)
        asserts.assert_equal(current_software_version, expected_software_version,
                             f"Version {current_software_version} is not the expected {expected_software_version}")


if __name__ == '__main__':
    default_matter_test_main()

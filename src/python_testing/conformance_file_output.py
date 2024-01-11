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

from matter_testing_support import MatterBaseTest, async_test_body, default_matter_test_main
from basic_composition_support import BasicCompositionTests


class TC_DeviceBasicComposition(MatterBaseTest, BasicCompositionTests):
    @async_test_body
    async def test_conformance_file_output(self):
        super().setup_class()
        await self.setup_class_helper(default_device_composition_path='sample_conformance_output_full', limited_wildcard=False)


if __name__ == "__main__":
    default_matter_test_main()

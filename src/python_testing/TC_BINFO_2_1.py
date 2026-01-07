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
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


from support_modules.binfo_attributes_verification import BasicInformationAttributesVerificationBase

import matter.clusters as Clusters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import TestStep
from matter.testing.runner import default_matter_test_main


class TC_BINFO_2_1(BasicInformationAttributesVerificationBase):
    def desc_TC_BINFO_2_1(self) -> str:
        return "[TC-BINFO-2.1] Attributes [DUT-Server]"

    def steps_TC_BINFO_2_1(self) -> list[TestStep]:
        return self.steps()

    def pics_TC_BINFO_2_1(self) -> list[str]:
        return self.pics("BINFO")

    @async_test_body
    async def test_TC_BINFO_2_1(self):
        await self.implementation(Clusters.BasicInformation)


if __name__ == "__main__":
    default_matter_test_main()

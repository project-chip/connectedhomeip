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


# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method thread-meshcop
#       --discriminator 1234
#       --passcode 20202021
#       --thread-ba-host 127.0.0.1
#       --thread-ba-port 49152
#       --thread-dataset-hex 0e08000000000001000000030000104a0300001635060004001fffe0020884fa18779329ac770708fd269658e44aa21a030f4f70656e5468726561642d32386335010228c50c0402a0f7f8051000112233445566778899aabbccddeeff041000112233445566778899aabbccddeeff
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from matter import ChipDeviceCtrl
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest, TestStep
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_SC_TC_4_1(MatterBaseTest):
    def desc_TC_SC_TC_4_1(self) -> str:
        return "[TC-SC-TC-4.1] Message Framing and PASE Session Establishment [DUT – Commissionee]"

    def steps_TC_SC_TC_4_1(self) -> list[TestStep]:
        return [
            TestStep(1, "DUT is ready to be discovered", is_commissioning=False),
            TestStep(2, "TH establishes PASE session over Thread Meshcop with DUT", is_commissioning=False),
        ]

    @async_test_body
    async def test_TC_SC_TC_4_1(self):
        self.step(1)
        self.wait_for_user_input("Power on the DUT")
        self.step(2)
        commissioner: ChipDeviceCtrl.ChipDeviceController = self.default_controller
        commissioner.SetThreadOperationalDataset(self.matter_test_config.thread_operational_dataset)
        await commissioner.EstablishPASESessionThreadMeshcop(baAddr=self.matter_test_config.thread_ba_host,
                                                             setupCode=self.first_setup_code(),
                                                             nodeId=self.matter_test_config.dut_node_ids[0],
                                                             baPort=self.matter_test_config.thread_ba_port)


if __name__ == "__main__":
    default_matter_test_main()

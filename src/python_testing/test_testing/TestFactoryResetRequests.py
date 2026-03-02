#
#    Copyright (c) 2026 Project CHIP Authors
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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.

# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     app-ready-pattern: "APP STATUS: Starting event loop"
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging

from matter import ChipDeviceCtrl
from matter.testing.commissioning import SetupParameters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TestFactoryResetRequests(MatterBaseTest):

    def desc_TestFactoryResetRequests(self) -> str:
        return "[TestFactoryResetRequests] Test Factory Reset Requests"

    def get_new_controller(self) -> ChipDeviceCtrl.ChipDeviceController:
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)
        return new_fabric_admin.NewController(paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))

    async def factory_reset_and_pase(self, setup_params: SetupParameters, pase_node_id: int, reset_ctrl: bool):
        # Get a new controller
        th2 = self.get_new_controller()

        # Request factory reset
        await self.request_device_factory_reset(reset_ctrl=reset_ctrl)

        # Open a PASE session
        await th2.FindOrEstablishPASESession(
            setupCode=setup_params.qr_code,
            nodeId=pase_node_id
        )

    @async_test_body
    async def test_TestFactoryResetRequests(self):
        pase_node_id = self.dut_node_id + 1
        setup_params = SetupParameters(
            discriminator=1234,
            passcode=20202021
        )

        logger.info("\n\n\t** Testing 'Factory Reset App Only' Back to Back **\n")
        await self.factory_reset_and_pase(setup_params=setup_params, pase_node_id=pase_node_id, reset_ctrl=False)
        await self.factory_reset_and_pase(setup_params=setup_params, pase_node_id=pase_node_id, reset_ctrl=False)

        logger.info("\n\n\t** Testing previous 'Factory Reset App Only' followed by a Back to Back 'Factory Reset' **\n")
        await self.factory_reset_and_pase(setup_params=setup_params, pase_node_id=pase_node_id, reset_ctrl=True)
        await self.factory_reset_and_pase(setup_params=setup_params, pase_node_id=pase_node_id, reset_ctrl=True)

        logger.info("\n\n\t** Testing previous 'Factory Reset' followed by a 'Factory Reset App Only' **\n")
        await self.factory_reset_and_pase(setup_params=setup_params, pase_node_id=pase_node_id, reset_ctrl=False)


if __name__ == "__main__":
    default_matter_test_main()

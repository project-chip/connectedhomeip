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
#       --endpoint 1
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging

from matter import ChipDeviceCtrl
from matter.testing.commissioning import SetupParameters
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import default_matter_test_main

logger = logging.getLogger(__name__)


class TC_TEMP00(MatterBaseTest):
    def get_new_controller(self) -> ChipDeviceCtrl.ChipDeviceController:
        new_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        new_fabric_admin = new_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.matter_test_config.fabric_id + 1)
        return new_fabric_admin.NewController(paaTrustStorePath=str(self.matter_test_config.paa_trust_store_path))

    @async_test_body
    async def test_TC_TEMP00(self):

        await asyncio.sleep(1)
        pase_node_id = self.dut_node_id + 1

        await self.request_factory_reset(app_only=False)

        # Open a PASE session
        th2 = self.get_new_controller()
        setupPayloadInfo = self.get_setup_payload_info()
        logger.info("\n\n\n\t\t[DEBUGGER] Opening PASE session...")
        setup_params = SetupParameters(
            discriminator=setupPayloadInfo[0].filter_value,
            passcode=setupPayloadInfo[0].passcode
        )
        await th2.FindOrEstablishPASESession(
            setupCode=setup_params.qr_code,
            nodeId=pase_node_id
        )
        logger.info("[DEBUGGER] PASE session opened successfully\n\n\n")


if __name__ == "__main__":
    default_matter_test_main()

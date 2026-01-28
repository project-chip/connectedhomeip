import asyncio
import logging

from matter import ChipDeviceCtrl
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.commissioning import SetupParameters
from matter.testing.decorators import async_test_body
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
        th2 = self.get_new_controller()
        pase_node_id = self.dut_node_id + 1

        await self.request_device_factory_reset()

        # Open a PASE session
        logger.info("\n\n\n\t\t[FRFZ]Opening PASE session...")
        setupPayloadInfo = self.get_setup_payload_info()
        setup_params = SetupParameters(
            discriminator=setupPayloadInfo[0].filter_value,
            passcode=setupPayloadInfo[0].passcode)
        await th2.FindOrEstablishPASESession(setupCode=setup_params.qr_code, nodeId=pase_node_id)
        logger.info("[FRFZ]PASE session opened successfully\n\n\n")


if __name__ == "__main__":
    default_matter_test_main()



import logging

import matter.clusters as Clusters
from matter.clusters import ClusterObjects as ClusterObjects
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)


class TC_FAN_SB(MatterBaseTest):
    def desc_TC_FAN_SB(self) -> str:
        return "[TC-FAN-3.5] Optional step functionality with DUT as Server"

    def steps_TC_FAN_SB(self):
        return [TestStep("1", "[FC] Commissioning already done.", is_commissioning=True)]

    def pics_TC_FAN_SB(self) -> list[str]:
        return ["FAN.S"]

    @async_test_body
    async def test_TC_FAN_SB(self) -> None:
        # Setup
        logger.info("Starting test_TC_FAN_SB")
        self.step("1")

        logger.info("[TEF] cluster_present FanControl on EP1: %s", await self.cluster_present(1, Clusters.FanControl))
        logger.info("[TEF] cluster_present OnOff on EP1: %s", await self.cluster_present(1, Clusters.OnOff))
        logger.info("[TEF] cluster_present ColorControl on EP1: %s", await self.cluster_present(1, Clusters.ColorControl))


if __name__ == "__main__":
    default_matter_test_main()

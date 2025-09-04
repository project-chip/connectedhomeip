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

import asyncio
import logging
import re
import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main


REQ_ENDPOINT = 0
PROV_ENDPOINT = 0
REQ_NODE_ID = 2
PROV_NODE_ID = 1
FABRIC_INDEX = 1

# Target version that is expected after the update (coincides with -vn of the .ota)
TARGET_VERSION = 2


class TC_SU_2_5(MatterBaseTest):
    "This test case verifies that the DUT behaves according to the spec when it is applying the software update."

    def desc_TC_SU_2_5(self) -> str:
        return "[TC-SU-2.5] Handling Different ApplyUpdateResponse Scenarios on Requestor"

    def pics_TC_SU_2_5(self):
        """Return the PICS definitions associated with this test."""
        return ["MCORE.OTA.Requestor"]

    def steps_TC_SU_2_5(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Commissioning already done", is_commissioning=True),
            TestStep(1, "OTA-P/TH sends the ApplyUpdateResponse Command to the DUT. Action field is set to 'Proceed', DelayedActionTime is set to 0."),
            TestStep(2, "Proceed with 3 minutes delay"),
            TestStep(3, "AwaitNextAction 1 minute (DUT retries >= 2min)"),
            TestStep(4, "AwaitNextAction 3 minutes and then Proceed"),
            TestStep(5, "Discontinue"),
        ]
        return steps
    # ------- helpers -------

    async def _write_default_providers(self, dev_ctrl):
        ProviderLoc = Clusters.OtaSoftwareUpdateRequestor.Structs.ProviderLocation
        value = [ProviderLoc(providerNodeID=PROV_NODE_ID,
                             endpoint=PROV_ENDPOINT,
                             fabricIndex=FABRIC_INDEX)]
        await self.write_single_attribute(
            attribute_value=Clusters.OtaSoftwareUpdateRequestor.Attributes.DefaultOTAProviders(value),
            endpoint_id=REQ_ENDPOINT
        )

    async def _read_update_state(self, dev_ctrl):
        return await self.read_single_attribute(
            dev_ctrl=dev_ctrl, node_id=REQ_NODE_ID, endpoint=REQ_ENDPOINT,
            attribute=Clusters.OtaSoftwareUpdateRequestor.Attributes.UpdateState
        )

    async def _read_sw_version(self, dev_ctrl):
        return await self.read_single_attribute(
            dev_ctrl=dev_ctrl, node_id=REQ_NODE_ID, endpoint=0,
            attribute=Clusters.BasicInformation.Attributes.SoftwareVersion
        )

    async def _wait_for_state(self, dev_ctrl, target, timeout_s=240, poll_s=1.0):
        end = self.event_loop.time() + timeout_s
        last = None
        while self.event_loop.time() < end:
            last = await self._read_update_state(dev_ctrl)
            if last == target:
                return
            await asyncio.sleep(poll_s)
        raise TimeoutError(f"No reached state {target} in {timeout_s}s (last={last})")

    async def _ensure_idle(self, dev_ctrl):
        UpdateStateEnum = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum
        st = await self._read_update_state(dev_ctrl)
        if st != UpdateStateEnum.kIdle:
            raise AssertionError(f"Expected Idle, got {st}")

    async def _set_provider_behavior(self, *, action: str, delay_s: int = 0):
        # attach flags/IPC/HTTP to the provider when you have it
        logging.info(f"[Provider] Behavior = {action}, delay={delay_s}s")

    def _count_apply_update_requests_in_log(self, path: str) -> int:
        """
        Count occurrences of ApplyUpdateRequest in the provider's log.
        Returns -1 if the log does not exist.
        """
        try:
            with open(path, "r", errors="ignore") as f:
                txt = f.read()
        except FileNotFoundError:
            return -1
        pattern = re.compile(r"ApplyUpdateRequest", re.IGNORECASE)
        return len(pattern.findall(txt))

    @async_test_body
    async def test_TC_SU_2_5(self):
        controller = self.default_controller
        await self._ensure_idle(controller)
        await self._write_default_providers(controller)

        self.step(0)  # Commissioning already done

        U = Clusters.OtaSoftwareUpdateRequestor.Enums.UpdateStateEnum

        # ===== Step 1: Proceed without delay (with asserts of expected outcome) =====
        self.step(1)

        # snapshot of the provider's log (adjust the path if you use another)
        before = self._count_apply_update_requests_in_log("/tmp/ota_provider.log")

        await self._set_provider_behavior(action="Proceed", delay_s=0)

        await self._wait_for_state(controller, U.kDownloading, 120)
        await self._wait_for_state(controller, U.kApplying,    180)
        await self._wait_for_state(controller, U.kIdle,        240)

        ver = await self._read_sw_version(controller)
        logging.info(f"[S1] SW version: {ver}")
        assert int(ver) == int(TARGET_VERSION), f"Expected version {TARGET_VERSION}, actual={ver}"

        # small wait for the provider to write logs with delay
        await asyncio.sleep(2)
        after = self._count_apply_update_requests_in_log("/tmp/ota_provider.log")
        if before >= 0 and after >= 0:
            delta = after - before
            # In this scenario we expect 1 ApplyUpdateRequest (the normal flow)
            assert delta == 1, f"Se esperaban 1 ApplyUpdateRequest; encontrados {delta} (before={before}, after={after})"
        else:
            logging.warning("No se pudo validar ApplyUpdateRequest (log del provider no disponible).")

        # ===== Steps 2–5: without changes (placeholders) =====

        # Step 2: Proceed with 3 minutes delay
        self.step(2)
        await self._set_provider_behavior(action="Proceed", delay_s=180)
        await self._wait_for_state(controller, U.kDownloading,     120)
        await self._wait_for_state(controller, U.kWaitingToApply,  180)
        await self._wait_for_state(controller, U.kApplying,        240)
        await self._wait_for_state(controller, U.kIdle,            300)
        ver = await self._read_sw_version(controller)
        logging.info(f"[S2] SW version: {ver}")

        # Step 3: AwaitNextAction 1min (DUT retries >= 2min), no apply yet
        self.step(3)
        await self._set_provider_behavior(action="AwaitNextAction", delay_s=60)
        await self._wait_for_state(controller, U.kDownloading, 120)
        await self._wait_for_state(controller, U.kWaitingToApply, 180)
        try:
            await self._wait_for_state(controller, U.kApplying, timeout_s=90)
            raise AssertionError("Should not apply within 90s after AwaitNextAction(60s)")
        except TimeoutError:
            logging.info("[S3] OK: not applied in short window")

        # Step 4: Await 3min and then Proceed
        self.step(4)
        await self._set_provider_behavior(action="AwaitNextAction", delay_s=180)
        await self._wait_for_state(controller, U.kDownloading,    120)
        await self._wait_for_state(controller, U.kWaitingToApply, 240)
        await self._set_provider_behavior(action="Proceed", delay_s=0)
        await self._wait_for_state(controller, U.kApplying, 300)
        await self._wait_for_state(controller, U.kIdle,     360)
        ver = await self._read_sw_version(controller)
        logging.info(f"[S4] SW version: {ver}")

        # Step 5: Discontinue
        self.step(5)
        await self._set_provider_behavior(action="Discontinue", delay_s=0)
        await self._wait_for_state(controller, U.kDownloading, 120)
        await self._wait_for_state(controller, U.kIdle,        180)
        ver = await self._read_sw_version(controller)
        logging.info(f"[S5] SW version (should be the previous one): {ver}")


if __name__ == "__main__":
    default_matter_test_main()

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

# Test excluded from CI: requires a physical NFC reader and an NFC-tagged DUT.

import asyncio
import logging

from mobly import asserts
from onboarding_payload_checks import OnboardingPayloadChecks

import matter.testing.nfc
from matter.setup_payload import SetupPayload
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterTestUncommissionedDevice
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)

# Bound on how long to wait, after the tester confirms the reader has been brought close to
# the DUT's NFC tag, for the background monitor to actually detect and read tag data.
_NFC_TAG_DETECTION_TIMEOUT_SEC = 5.0
_NFC_TAG_POLL_INTERVAL_SEC = 0.1


class TC_DD_1_3(OnboardingPayloadChecks, MatterTestUncommissionedDevice):

    @pics('MCORE.ROLE.COMMISSIONEE', 'MCORE.DD.NFC')
    @async_test_body
    async def test_TC_DD_1_3(self):
        """[TC-DD-1.3] NFC Tag Content [DUT - Commissionee]"""
        self.step(1, "Power up the DUT and put the DUT in pairing mode")
        log.info("DUT expected to be powered on and in pairing mode.")

        self.step(2, "Bring the NFC reader close to the DUT",
                  expectation="NFC tag is detected and read, its content is an onboarding payload")
        reader = matter.testing.nfc.NFCReader()
        monitoring_task = asyncio.create_task(reader.activate_tag_monitoring())
        await self.wait_for_user_input_async(
            "Bring the TH NFC reader close to the DUT's NFC tag. Press Enter when done.")
        try:
            await asyncio.wait_for(self._wait_for_nfc_tag_data(reader), timeout=_NFC_TAG_DETECTION_TIMEOUT_SEC)
        except TimeoutError:
            asserts.fail("Timed out waiting for the NFC reader to detect tag data")
        finally:
            reader.deactivate_tag_monitoring()
        nfc_tag_content = await monitoring_task
        log.info("nfc_tag_content: %s", nfc_tag_content)
        asserts.assert_true(reader.is_onboarding_data(nfc_tag_content), "No NFC tag with onboarding data found")

        payload = SetupPayload().ParseQrCode(nfc_tag_content)

        self.step("3a", "Verify the onboarding payload version", expectation="Version field is 0")
        self.check_payload_version(payload)

        self.step("3b", "Verify the 8-bit Discovery Capabilities Bitmask",
                  expectation="No reserved bits are set, and at least one discovery method is advertised")
        self.check_discovery_capabilities_bitmask(payload)

        self.step("3c", "Verify the 12-bit discriminator matches the value the DUT advertises during commissioning",
                  expectation="DUT is discoverable using the discriminator encoded in the NFC tag")
        asserts.assert_is_not_none(payload.long_discriminator, "NFC tag payload is missing the long discriminator")
        await self.check_advertised_discriminator(payload.long_discriminator, long=True)

        self.step("3d", "Verify the onboarding payload contains a 27-bit Passcode",
                  expectation="Passcode is between 0x0000001 and 0x5f5e0fe")
        self.check_passcode_range(payload)

        self.step("3e", "Verify passcode is valid", expectation="Passcode is not a disallowed default value")
        self.check_passcode_validity(payload)

        self.step("3f", "Verify the NFC tag's onboarding payload code prefix", expectation="Prefix is \"MT:\"")
        self.check_code_prefix(nfc_tag_content[:3])

        self.step("3g", "Verify Vendor ID and Product ID are present in the payload")
        self.log_vendor_and_product_id_not_verified(payload)

        self.step(4, "Verify the packed binary data structure",
                  expectation="Structure is padded with 0 bits at the end to the nearest byte boundary")
        # QRCodeSetupPayloadParser (src/setup_payload/QRCodeSetupPayloadParser.cpp) rejects the
        # payload with CHIP_ERROR_INVALID_ARGUMENT if the trailing padding bits are not all 0, so
        # the successful parse above already confirms the packed structure is correctly padded.
        log.info("Padding bits validated implicitly by the successful parse.")

        self.step(5, "Verify the Commissioning Flow value", expectation="Value is 0, 1 or 2")
        self.check_commissioning_flow(payload)

    async def wait_for_user_input_async(self, *args, **kwargs):
        loop = asyncio.get_running_loop()
        return await loop.run_in_executor(None, self.wait_for_user_input, *args, **kwargs)

    @staticmethod
    async def _wait_for_nfc_tag_data(reader: matter.testing.nfc.NFCReader) -> None:
        """Poll until the NFC monitor has read some tag data.

        NFCReader.activate_tag_monitoring() only resolves once deactivate_tag_monitoring() is
        called, so it can't be used by itself to detect that a tag has actually been read, and
        there is no public API for that. This polls the same observer.last_ndef field that
        TagMonitorManager.activate() returns once deactivated, so monitoring can be stopped as
        soon as data is available instead of only when the tester presses Enter. An
        asyncio.Event isn't an option here: last_ndef is set by TagEventObserver.update() on the
        pcsc backend's own monitoring thread, not scheduled onto this event loop.
        """
        while (reader._monitor_manager is None or reader._monitor_manager.observer is None  # noqa: ASYNC110
                or reader._monitor_manager.observer.last_ndef is None):
            await asyncio.sleep(_NFC_TAG_POLL_INTERVAL_SEC)


if __name__ == "__main__":
    default_matter_test_main()

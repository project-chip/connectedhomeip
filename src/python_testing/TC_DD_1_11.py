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

import asyncio
import logging
import os
import tempfile

import qrcode
from mobly import asserts

from matter.testing.apps import AppServerSubprocess
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterTestCommissioner, SetupParameters, TestStep
from matter.testing.pixit import pixit
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_DD_1_11(MatterTestCommissioner):
    disable_wildcard_subscription = True

    def setup_class(self):
        super().setup_class()
        self.app1 = None
        self.app2 = None
        self.storage = None

    def teardown_test(self):
        if self.app1 is not None:
            self.app1.terminate()
            self.app1 = None
        if self.app2 is not None:
            self.app2.terminate()
            self.app2 = None
        if self.storage is not None:
            self.storage.cleanup()
            self.storage = None
        super().teardown_test()

    def desc_TC_DD_1_11(self) -> str:
        return "[TC-DD-1.11] Concatenation - QR Code Onboarding Payload Verification [DUT - Commissioner]"

    def pics_TC_DD_1_11(self) -> list[str]:
        return ["MCORE.DD.CTRL_CONCATENATED_QR_CODE_1"]

    def steps_TC_DD_1_11(self) -> list[TestStep]:
        return [
            TestStep(
                1,
                "TH starts up two example apps with different discriminators and passcodes and displays a concatenated QR code. TH asks the commissioner to trigger commissioning using the given concatenated QR code.",
            ),
            TestStep(
                2,
                "TH asks the user if the commissioner requested individual QR codes and saves the answer as individual_qr_codes_required",
            ),
            TestStep(
                3,
                "If individual_qr_codes_required is false, TH waits until both example apps have been commissioned",
                "Both example apps are commissioned",
            ),
            TestStep(
                4,
                "If individual_qr_codes_required is true, TH displays the code for the first example application and waits until it is commissioned",
                "The first example app is commissioned",
            ),
            TestStep(
                5,
                "If individual_qr_codes_required is true, TH displays the code for the second example application and waits until it is commissioned",
                "The second example app is commissioned",
            ),
        ]

    @pixit("th_server_app_path", str, "Path to TH server application (all-devices-app)")
    @async_test_body
    async def test_TC_DD_1_11(self):
        th_server_app = self.pixit("th_server_app_path")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        self.step(1)

        self.storage = tempfile.TemporaryDirectory(prefix=self.__class__.__name__)

        setup_params_1 = SetupParameters(discriminator=1111, passcode=20202021)
        setup_params_2 = SetupParameters(discriminator=2222, passcode=20202022)

        app_ready_patterns = ["Server initialization complete", "APP STATUS: Starting event loop"]

        self.app1 = AppServerSubprocess(
            th_server_app,
            storage_dir=self.storage.name,
            port=5541,
            discriminator=setup_params_1.discriminator,
            passcode=setup_params_1.passcode,
            kvs_path=os.path.join(self.storage.name, "kvs_app1"),
        )
        self.app1.start(
            expected_output=app_ready_patterns,
            timeout=30,
        )

        self.app2 = AppServerSubprocess(
            th_server_app,
            storage_dir=self.storage.name,
            port=5542,
            discriminator=setup_params_2.discriminator,
            passcode=setup_params_2.passcode,
            kvs_path=os.path.join(self.storage.name, "kvs_app2"),
        )
        self.app2.start(
            expected_output=app_ready_patterns,
            timeout=30,
        )

        # Allow the subprocess stdout forwarding threads to finish flushing the matched line.
        await asyncio.sleep(0.2)

        qr_code_1 = setup_params_1.qr_code
        qr_code_2 = setup_params_2.qr_code
        concatenated_qr_code = f"{qr_code_1}*{qr_code_2.removeprefix('MT:')}"

        log.info("Concatenated QR code: %s", concatenated_qr_code)
        print(f"Concatenated QR code: {concatenated_qr_code}")
        qr = qrcode.QRCode()
        qr.add_data(concatenated_qr_code)
        qr.print_ascii(invert=True)

        self.wait_for_user_input(
            prompt_msg=(
                f"Please trigger commissioning on the commissioner using the concatenated QR code ({concatenated_qr_code}).\n"
                "Press 1 if the commissioner was able to successfully commission, or 2 if individual QR codes were requested by the commissioner."
            )
        )


if __name__ == "__main__":
    default_matter_test_main()

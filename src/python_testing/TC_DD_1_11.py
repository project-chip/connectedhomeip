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

import io
import logging
import os
import tempfile

import qrcode
from mdns_discovery.mdns_discovery import MdnsDiscovery
from mobly import asserts

from matter.testing.apps import AppServerSubprocess
from matter.testing.decorators import async_test_body, pics
from matter.testing.matter_testing import MatterTestCommissioner, SetupParameters
from matter.testing.pixit import pixit
from matter.testing.runner import default_matter_test_main

log = logging.getLogger(__name__)


class TC_DD_1_11(MatterTestCommissioner):
    disable_wildcard_subscription = True

    # Default path where the all-devices-app binary is installed in the CSA Test Harness (chip-cert-bins) container.
    DEFAULT_TH_APP_PATH = "/root/apps/all-devices-app"

    PROMPT_TIMEOUT_NOTE = (
        "Note: If this prompt times out in the Test Harness, pass --prompt-timeout <seconds> "
        'or set "prompt_timeout_seconds": 300 in your TH config file.'
    )

    # Set a large timeout (10 minutes) to allow sufficient time for manual user
    # prompts and commissioning of two applications by the DUT commissioner.
    @property
    def default_timeout(self) -> int:
        return 600

    def setup_class(self):
        super().setup_class()
        self.app1 = None
        self.app2 = None

    def teardown_test(self):
        if self.app1 is not None:
            self.app1.terminate()
            self.app1 = None
        if self.app2 is not None:
            self.app2.terminate()
            self.app2 = None
        super().teardown_test()

    @staticmethod
    def _render_qr_code(qr_code_data: str) -> str:
        qr_buf = io.StringIO()
        qr = qrcode.QRCode()
        qr.add_data(qr_code_data)
        qr.print_ascii(out=qr_buf, invert=True)
        return qr_buf.getvalue()

    def _prompt_user(
        self,
        prompt_msg: str,
        prompt_msg_placeholder: str = "Submit anything to continue",
        default_value: str = "y",
    ) -> str:
        resp = self.wait_for_user_input(
            prompt_msg=f"{prompt_msg}\n\n{self.PROMPT_TIMEOUT_NOTE}",
            prompt_msg_placeholder=prompt_msg_placeholder,
            default_value=default_value,
        )
        asserts.assert_is_not_none(
            resp,
            f"No response received (prompt may have timed out). {self.PROMPT_TIMEOUT_NOTE}",
        )
        return resp

    @pics("MCORE.DD.CTRL_CONCATENATED_QR_CODE_1")
    @pixit(
        "th_server_app_path",
        str,
        "Path to TH server application (all-devices-app)",
        required=False,
        default=DEFAULT_TH_APP_PATH,
    )
    @async_test_body
    async def test_TC_DD_1_11(self):
        """[TC-DD-1.11] Concatenation - QR Code Onboarding Payload Verification [DUT - Commissioner]"""
        th_server_app = self.pixit("th_server_app_path")
        if not os.path.exists(th_server_app):
            asserts.fail(f"The path {th_server_app} does not exist")

        self.step(
            1,
            "TH starts up two example apps with different discriminators and passcodes and displays a concatenated QR code. "
            "TH asks the commissioner to trigger commissioning using the given concatenated QR code and prompts the user to "
            "indicate if commissioning succeeded, if individual QR codes were requested (saved as individual_qr_codes_required), "
            "or if commissioning failed.",
            expectation="Commissioner does not fail to commission using the concatenated QR code",
        )

        port1 = 5541
        port2 = 5542
        setup_params_1 = SetupParameters(discriminator=1111, passcode=20202021)
        setup_params_2 = SetupParameters(discriminator=2222, passcode=20202022)

        self.app1 = AppServerSubprocess(
            th_server_app,
            storage_dir=tempfile.gettempdir(),
            port=port1,
            discriminator=setup_params_1.discriminator,
            passcode=setup_params_1.passcode,
        )
        self.app1.start(
            expected_output="Server initialization complete",
            timeout=30,
        )

        self.app2 = AppServerSubprocess(
            th_server_app,
            storage_dir=tempfile.gettempdir(),
            port=port2,
            discriminator=setup_params_2.discriminator,
            passcode=setup_params_2.passcode,
        )
        self.app2.start(
            expected_output="Server initialization complete",
            timeout=30,
        )

        qr_code_1 = setup_params_1.qr_code
        qr_code_2 = setup_params_2.qr_code
        concatenated_qr_code = f"{qr_code_1}*{qr_code_2.removeprefix('MT:')}"
        qr_str = self._render_qr_code(concatenated_qr_code)

        resp = self._prompt_user(
            prompt_msg=(
                "Please trigger commissioning on the commissioner using the concatenated QR code:\n"
                f"{concatenated_qr_code}\n\n"
                f"{qr_str}\n"
                "Press 1 if the commissioner was able to successfully commission\n"
                "Press 2 if individual QR codes were requested by the commissioner\n"
                "Press 3 if the commissioner was unable to commission using the QR code"
            ),
            prompt_msg_placeholder="1, 2 or 3",
            default_value="",
        ).strip()
        asserts.assert_true(
            bool(resp),
            f"No response received (prompt may have timed out). {self.PROMPT_TIMEOUT_NOTE}",
        )
        asserts.assert_not_equal(resp, "3", "Commissioner could not commission using the concatenated QR code")
        asserts.assert_in(resp, ["1", "2"], f"Invalid response {resp!r}: expected 1, 2 or 3")
        individual_qr_codes_required = resp == "2"

        if individual_qr_codes_required:
            self.step(
                2,
                "If individual_qr_codes_required is true, TH displays the QR code for the first example application and "
                "waits for user confirmation that commissioning is complete",
            )
            qr_str_1 = self._render_qr_code(qr_code_1)
            self._prompt_user(
                prompt_msg=(
                    f"Please provide the QR code for the first example application (discriminator: {setup_params_1.discriminator}) to the commissioner:\n"
                    f"{qr_code_1}\n\n"
                    f"{qr_str_1}\n"
                    "Press Enter once commissioning of the first example application is complete"
                )
            )

            self.step(
                3,
                "If individual_qr_codes_required is true, TH displays the QR code for the second example application and "
                "waits for user confirmation that commissioning is complete",
            )
            qr_str_2 = self._render_qr_code(qr_code_2)
            self._prompt_user(
                prompt_msg=(
                    f"Please provide the QR code for the second example application (discriminator: {setup_params_2.discriminator}) to the commissioner:\n"
                    f"{qr_code_2}\n\n"
                    f"{qr_str_2}\n"
                    "Press Enter once commissioning of the second example application is complete"
                )
            )
        else:
            self.skip_step(2)
            self.skip_step(3)

        self.step(
            4,
            "TH verifies via operational discovery that both example apps have been commissioned",
            expectation="Both example apps are commissioned",
        )
        mdns = MdnsDiscovery()
        operational_services = await mdns.get_operational_services(log_output=True)
        commissioned_ports = {service.port for service in operational_services}
        asserts.assert_in(port1, commissioned_ports, "First example app is not advertising as operational")
        asserts.assert_in(port2, commissioned_ports, "Second example app is not advertising as operational")


if __name__ == "__main__":
    default_matter_test_main()

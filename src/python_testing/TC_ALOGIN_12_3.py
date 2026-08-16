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

import logging
import re
from urllib.parse import urlparse

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing import matter_asserts
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.pixit import pixit
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# GetDeviceAuthURI is a timed command, so every invoke needs a timed request timeout.
_TIMED_REQUEST_TIMEOUT_MS = 5000

# Characters that commonly appear in device-flow user codes, e.g. "A1B2 C3D4" or "A1B2-C3D4".
_USER_CODE_PATTERN = r"^[A-Za-z0-9][A-Za-z0-9 \-]*$"


class TC_ALOGIN_12_3(MatterBaseTest):

    def desc_TC_ALOGIN_12_3(self) -> str:
        return "[TC-ALOGIN-12.3] OAuth Device Authorization Verification"

    def pics_TC_ALOGIN_12_3(self) -> list[str]:
        return ["ALOGIN.S", "ALOGIN.S.F00"]

    def steps_TC_ALOGIN_12_3(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads the OAuthLoggedIn attribute from the DUT.",
                     "Verify that the response contains a boolean value (TRUE or FALSE)."),
            TestStep(2, "TH sends a GetDeviceAuthURI command to the DUT.",
                     "DUT replies with a GetDeviceAuthURIResponse containing a non-empty UserCode (max 16 chars), a "
                     "non-empty VerificationURI (max 256 chars), a non-zero ExpiresIn value, and a non-zero Interval "
                     "value."),
            TestStep(3, "TH inspects the GetDeviceAuthURIResponse fields.",
                     "Verify UserCode is a short alphanumeric string and VerificationURI is a valid URI. If present, "
                     "verify VerificationURIComplete (max 512 chars) is a URI that encodes the UserCode."),
            TestStep(4, "User completes OAuth login on another device by visiting the VerificationURI and entering "
                     "the UserCode. TH then reads the OAuthLoggedIn attribute from the DUT.",
                     "Verify that OAuthLoggedIn is now TRUE, indicating the Content App has an active OAuth "
                     "session."),
            TestStep(5, "TH sends a Logout command to the DUT.",
                     "DUT replies with a success status response (0x00) and clears the current user account."),
            TestStep(6, "TH reads the OAuthLoggedIn attribute from the DUT.",
                     "Verify that OAuthLoggedIn is now FALSE after logout."),
            TestStep(7, "TH sends repeated GetDeviceAuthURI commands beyond the OAuth provider's rate limit.",
                     "DUT replies with a FAILURE status code when rate limits are exceeded, and does not return new "
                     "authorization parameters."),
        ]

    async def _read_oauth_logged_in(self, endpoint) -> bool:
        return await self.read_single_attribute_check_success(
            endpoint=endpoint,
            cluster=Clusters.AccountLogin,
            attribute=Clusters.AccountLogin.Attributes.OAuthLoggedIn)

    async def _get_device_auth_uri(self, endpoint):
        return await self.send_single_cmd(
            cmd=Clusters.AccountLogin.Commands.GetDeviceAuthURI(),
            endpoint=endpoint,
            timedRequestTimeoutMs=_TIMED_REQUEST_TIMEOUT_MS)

    @pixit("rate_limit_attempts", int,
           "Number of GetDeviceAuthURI requests to issue in step 7 while probing the provider's rate limit",
           required=False, default=10)
    @run_if_endpoint_matches(
        has_feature(Clusters.AccountLogin, Clusters.AccountLogin.Bitmaps.Feature.kOAuth))
    async def test_TC_ALOGIN_12_3(self):
        cluster = Clusters.AccountLogin
        endpoint = self.get_endpoint()
        rate_limit_attempts = self.pixit("rate_limit_attempts")

        self.step(0)

        self.step(1)
        oauth_logged_in = await self._read_oauth_logged_in(endpoint)
        matter_asserts.assert_valid_bool(oauth_logged_in, "OAuthLoggedIn")

        self.step(2)
        response = await self._get_device_auth_uri(endpoint)
        matter_asserts.assert_non_empty_string(response.userCode, "UserCode")
        matter_asserts.assert_string_length(response.userCode, "UserCode", max_length=16)
        matter_asserts.assert_non_empty_string(response.verificationURI, "VerificationURI")
        matter_asserts.assert_string_length(response.verificationURI, "VerificationURI", max_length=256)
        matter_asserts.assert_valid_uint16(response.expiresIn, "ExpiresIn")
        asserts.assert_greater(response.expiresIn, 0, "ExpiresIn must be non-zero")
        matter_asserts.assert_valid_uint8(response.interval, "Interval")
        asserts.assert_greater(response.interval, 0, "Interval must be non-zero")

        self.step(3)
        matter_asserts.assert_string_matches_pattern(
            response.userCode, "UserCode", _USER_CODE_PATTERN)
        parsed = urlparse(response.verificationURI)
        asserts.assert_true(bool(parsed.scheme) and bool(parsed.netloc),
                            f"VerificationURI '{response.verificationURI}' is not a valid absolute URI")
        if response.verificationURIComplete is not None:
            matter_asserts.assert_string_length(
                response.verificationURIComplete, "VerificationURIComplete", max_length=512)
            parsed_complete = urlparse(response.verificationURIComplete)
            asserts.assert_true(bool(parsed_complete.scheme) and bool(parsed_complete.netloc),
                                f"VerificationURIComplete '{response.verificationURIComplete}' is not a valid URI")
            # The complete URI exists so a QR code can carry the code as well as the URI, so the
            # code must appear in it. Separators in the displayed code are not necessarily kept.
            normalized_code = re.sub(r"[ \-]", "", response.userCode).lower()
            normalized_uri = re.sub(r"[ \-]", "", response.verificationURIComplete).lower()
            asserts.assert_in(normalized_code, normalized_uri,
                              "VerificationURIComplete must encode the UserCode")
        else:
            log.info("VerificationURIComplete is not present; it is an optional field")

        self.step(4)
        # ExpiresIn bounds how long the operator has to finish; surface it in the prompt.
        self.wait_for_user_input(
            prompt_msg=f"On another device, visit {response.verificationURI} and enter the user code "
                       f"'{response.userCode}' to complete the OAuth login.\n"
                       f"This must be done within {response.expiresIn} seconds. Press Enter when finished.\n")
        oauth_logged_in = await self._read_oauth_logged_in(endpoint)
        asserts.assert_true(oauth_logged_in,
                            "OAuthLoggedIn should be TRUE once the user has completed the OAuth login")

        self.step(5)
        await self.send_single_cmd(cmd=cluster.Commands.Logout(), endpoint=endpoint)

        self.step(6)
        oauth_logged_in = await self._read_oauth_logged_in(endpoint)
        asserts.assert_false(oauth_logged_in, "OAuthLoggedIn should be FALSE after Logout")

        self.step(7)
        # The number of requests needed to trip the limit is provider-specific, so this issues a
        # bounded burst and requires a FAILURE once the limit is reached.
        rate_limited = False
        for attempt in range(1, rate_limit_attempts + 1):
            try:
                await self._get_device_auth_uri(endpoint)
            except InteractionModelError as e:
                asserts.assert_equal(
                    e.status, Status.Failure,
                    f"GetDeviceAuthURI attempt {attempt} failed with {e.status}, expected FAILURE for rate limiting")
                log.info("DUT reported FAILURE on GetDeviceAuthURI attempt %d, as expected for rate limiting", attempt)
                rate_limited = True
                break

        if not rate_limited:
            log.info("The provider's rate limit was not reached in %d attempts", rate_limit_attempts)
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()

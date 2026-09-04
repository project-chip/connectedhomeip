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

from TC_MESSTestBase import has_spoken_or_audio_messages

import matter.clusters as Clusters
from matter.testing import matter_asserts
from matter.testing.decorators import run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

# RFC 5646 language tag: a primary subtag of 1-8 alpha characters followed by optional
# subtags of up to 8 alphanumeric characters, e.g. "en-US", "fr-FR" or the private-use
# form "x-test". This is a shape check, not full RFC 5646 registry validation.
_LANGUAGE_TAG_PATTERN = r"^[A-Za-z]{1,8}(-[A-Za-z0-9]{1,8})*$"

# MIME type: type/subtype over the full RFC 2045 token character set (any CHAR except
# SPACE, CTLs and tspecials), e.g. "audio/mpeg" or "audio/x-foo~bar".
_MIME_TOKEN = r"[A-Za-z0-9!#$%&'*+^_`{|}~.-]+"
_MIME_TYPE_PATTERN = rf"^{_MIME_TOKEN}/{_MIME_TOKEN}$"


class TC_MESS_1_1(MatterBaseTest):

    def desc_TC_MESS_1_1(self) -> str:
        return "[TC-MESS-1.1] Read Audio and Speech Attributes Verification (DUT as Server)"

    def pics_TC_MESS_1_1(self) -> list[str]:
        return ["MESS.S"]

    def steps_TC_MESS_1_1(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads the SupportedLanguageCodes attribute.",
                     "Verify that a non-empty list of language tag strings is returned. Each entry is a valid "
                     "RFC 5646 language tag (e.g. 'en-US', 'fr-FR')."),
            TestStep(2, "TH reads the SupportedMimeTypes attribute.",
                     "Verify that a non-empty list of MIME type strings is returned. Each entry follows the MIME "
                     "type format (e.g. 'audio/mpeg', 'audio/wav')."),
        ]

    @run_if_endpoint_matches(has_spoken_or_audio_messages())
    async def test_TC_MESS_1_1(self):
        cluster = Clusters.Messages
        endpoint = self.get_endpoint()

        self.step(0)
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.FeatureMap)
        supports_spoken = bool(feature_map & cluster.Bitmaps.Feature.kSpokenMessages)
        supports_audio = bool(feature_map & cluster.Bitmaps.Feature.kAudioMessages)

        self.step(1)
        if supports_spoken:
            language_codes = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SupportedLanguageCodes)
            matter_asserts.assert_list(language_codes, "SupportedLanguageCodes", min_length=1)
            for language_code in language_codes:
                matter_asserts.assert_is_string(language_code, "SupportedLanguageCodes entry")
                matter_asserts.assert_string_matches_pattern(
                    language_code, "SupportedLanguageCodes entry", _LANGUAGE_TAG_PATTERN)
            log.info("DUT supports language codes: %s", language_codes)
        else:
            # SupportedLanguageCodes is mandatory only when SpokenMessages is supported.
            self.mark_current_step_skipped()

        self.step(2)
        if supports_audio:
            mime_types = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.SupportedMimeTypes)
            matter_asserts.assert_list(mime_types, "SupportedMimeTypes", min_length=1)
            for mime_type in mime_types:
                matter_asserts.assert_is_string(mime_type, "SupportedMimeTypes entry")
                matter_asserts.assert_string_matches_pattern(
                    mime_type, "SupportedMimeTypes entry", _MIME_TYPE_PATTERN)
            log.info("DUT supports MIME types: %s", mime_types)
        else:
            # SupportedMimeTypes is mandatory only when AudioMessages is supported.
            self.mark_current_step_skipped()


if __name__ == "__main__":
    default_matter_test_main()

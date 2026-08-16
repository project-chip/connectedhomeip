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
from dataclasses import dataclass

from mobly import asserts
from TC_MEDIAPLAYBACKTestBase import MEDIAPLAYBACKTestBase

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.testing import matter_asserts
from matter.testing.decorators import has_attribute, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)

_MEDIA_TYPE = Clusters.MediaPlayback.Enums.MediaType


@dataclass(frozen=True)
class _ContentInfoField:
    """A descriptive ContentInfoStruct field, its length limit, and where it applies.

    Per the MediaType type description, each content type is described by a specific set
    of fields. Fields outside that set SHOULD be omitted but MAY be present, so only the
    applicable ones are required for a given ContentType.
    """
    name: str
    max_length: int
    applies_to: tuple


_CONTENT_INFO_FIELDS = (
    _ContentInfoField("title", 256, (_MEDIA_TYPE.kGeneric,)),
    _ContentInfoField("show", 64, (_MEDIA_TYPE.kTVShow, _MEDIA_TYPE.kPodcast)),
    _ContentInfoField("season", 64, (_MEDIA_TYPE.kTVShow,)),
    _ContentInfoField("episode", 64, (_MEDIA_TYPE.kTVShow, _MEDIA_TYPE.kPodcast)),
    _ContentInfoField("provider", 64, (_MEDIA_TYPE.kMusic,)),
    _ContentInfoField("artist", 64, (_MEDIA_TYPE.kMusic,)),
    _ContentInfoField("album", 64, (_MEDIA_TYPE.kMusic,)),
    _ContentInfoField("track", 64, (_MEDIA_TYPE.kMusic,)),
)


class TC_MEDIAPLAYBACK_6_12(MatterBaseTest, MEDIAPLAYBACKTestBase):

    def desc_TC_MEDIAPLAYBACK_6_12(self) -> str:
        return "[TC-MEDIAPLAYBACK-6.12] Content Info Verification"

    def pics_TC_MEDIAPLAYBACK_6_12(self) -> list[str]:
        return ["MEDIAPLAYBACK.S", "MEDIAPLAYBACK.S.A000c"]

    def steps_TC_MEDIAPLAYBACK_6_12(self) -> list[TestStep]:
        return [
            TestStep(0, "Commissioning, already done", is_commissioning=True),
            TestStep(1, "TH reads the Presets attribute from the DUT.",
                     "DUT replies with a list of Content Preset structs in which all PresetIDs are unique and "
                     "all Names are unique."),
            TestStep(2, "TH sends a PlayPreset command to the DUT with the first value in the list from step 1.",
                     "DUT replies with a success response and starts playing some kind of content."),
            TestStep(3, "TH reads the ContentInfo attribute from the DUT.",
                     "DUT replies with a ContentInfo struct describing the currently playing content. ContentType is "
                     "one of the available MediaType enum values and the fields appropriate to that MediaType are "
                     "populated."),
            TestStep(4, "TH sends a PlayPreset command to the DUT with the second value in the list from step 1.",
                     "DUT replies with a success response and starts playing some other kind of content."),
            TestStep(5, "TH reads the ContentInfo attribute from the DUT.",
                     "DUT replies with a ContentInfo struct describing the currently playing content that differs "
                     "from the struct received in step 3, with the fields appropriate to its MediaType populated."),
        ]

    def _verify_content_info(self, content_info, label: str):
        """Assert the struct is well formed and describes its declared MediaType."""
        asserts.assert_true(content_info is not NullValue,
                            f"ContentInfo must not be null while content is playing ({label})")
        matter_asserts.assert_valid_enum(content_info.contentType, f"ContentInfo.ContentType {label}", _MEDIA_TYPE)

        # Any field that is present must respect its length constraint, whether or not it
        # is one of the fields expected for this MediaType.
        for field in _CONTENT_INFO_FIELDS:
            value = getattr(content_info, field.name)
            if value is None or value is NullValue:
                continue
            matter_asserts.assert_string_length(
                value, f"ContentInfo.{field.name} {label}", max_length=field.max_length)

        expected_fields = [field.name for field in _CONTENT_INFO_FIELDS
                           if content_info.contentType in field.applies_to]
        populated = [name for name in expected_fields
                     if getattr(content_info, name) not in (None, NullValue, "")]
        log.info("ContentInfo %s: ContentType=%s, populated descriptive fields: %s",
                 label, content_info.contentType.name, populated or "none")
        asserts.assert_true(
            len(populated) > 0,
            f"ContentInfo {label} declares ContentType {content_info.contentType.name} but none of the fields "
            f"that describe it ({', '.join(expected_fields)}) are populated")

    @run_if_endpoint_matches(has_attribute(Clusters.MediaPlayback.Attributes.ContentInfo))
    async def test_TC_MEDIAPLAYBACK_6_12(self):
        cluster = Clusters.MediaPlayback
        endpoint = self.get_endpoint()

        self.step(0)

        self.step(1)
        presets = await self.read_content_presets(endpoint)
        if presets is not None:
            self.verify_presets_unique(presets)
        else:
            self.mark_current_step_skipped()

        self.step(2)
        await self.switch_content(endpoint, presets, 0)

        self.step(3)
        first_info = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ContentInfo)
        self._verify_content_info(first_info, "for the first content")

        self.step(4)
        await self.switch_content(endpoint, presets, 1)

        self.step(5)
        second_info = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=cluster.Attributes.ContentInfo)
        self._verify_content_info(second_info, "for the second content")
        asserts.assert_not_equal(second_info, first_info,
                                 "ContentInfo should differ once different content is playing")


if __name__ == "__main__":
    default_matter_test_main()

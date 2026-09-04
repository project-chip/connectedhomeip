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

"""Shared helpers for the Audio Control cluster (0x0512) test cases.

The Audio Control test plan gates most steps on whether an optional attribute is
implemented. Rather than relying on PICS strings alone, these helpers read the
cluster's AttributeList and FeatureMap once and expose ``supports_*`` predicates,
so a run against a real DUT skips the right steps even when the PICS file is
incomplete.
"""

import logging

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.interaction_model import InteractionModelError, Status

log = logging.getLogger(__name__)

_CLUSTER = Clusters.AudioControl


class AUDIOCONTROLTestBase:

    # Cluster capability discovery

    async def read_audiocontrol_capabilities(self, endpoint):
        """Read AttributeList and FeatureMap, caching them for the supports_* helpers."""
        self._audiocontrol_attribute_list = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=_CLUSTER, attribute=_CLUSTER.Attributes.AttributeList)
        self._audiocontrol_feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=_CLUSTER, attribute=_CLUSTER.Attributes.FeatureMap)

    def supports_attribute(self, attribute) -> bool:
        """True when the DUT reported this attribute in its AttributeList."""
        return attribute.attribute_id in self._audiocontrol_attribute_list

    def supports_feature(self, feature: Clusters.AudioControl.Bitmaps.Feature) -> bool:
        return bool(self._audiocontrol_feature_map & feature)

    @property
    def supports_beq(self) -> bool:
        return self.supports_feature(_CLUSTER.Bitmaps.Feature.kBasicEqualizer)

    # Attribute access

    async def read_audiocontrol_attribute_expect_success(self, endpoint, attribute):
        return await self.read_single_attribute_check_success(endpoint=endpoint, cluster=_CLUSTER, attribute=attribute)

    async def write_audiocontrol_attribute_expect_success(self, endpoint, attribute, value) -> Status:
        status = await self.write_single_attribute(attribute_value=attribute(value), endpoint_id=endpoint)
        asserts.assert_equal(status, Status.Success, f"Write to {attribute.__name__} was not successful")
        return status

    # Commands

    async def _send_audiocontrol_cmd(self, endpoint, cmd, expected_status: Status):
        try:
            result = await self.send_single_cmd(cmd=cmd, endpoint=endpoint)
            asserts.assert_equal(expected_status, Status.Success,
                                 f"{type(cmd).__name__} succeeded but {expected_status} was expected")
            return result
        except InteractionModelError as e:
            asserts.assert_equal(e.status, expected_status, f"Unexpected status returned for {type(cmd).__name__}")
            return None

    async def send_mute(self, endpoint, expected_status: Status = Status.Success):
        return await self._send_audiocontrol_cmd(endpoint, _CLUSTER.Commands.Mute(), expected_status)

    async def send_unmute(self, endpoint, expected_status: Status = Status.Success):
        return await self._send_audiocontrol_cmd(endpoint, _CLUSTER.Commands.Unmute(), expected_status)

    async def send_toggle_muted(self, endpoint, expected_status: Status = Status.Success):
        return await self._send_audiocontrol_cmd(endpoint, _CLUSTER.Commands.ToggleMuted(), expected_status)

    async def send_set_volume(self, endpoint, new_volume: int, unmute_policy=None,
                              expected_status: Status = Status.Success):
        cmd = _CLUSTER.Commands.SetVolume(newVolume=new_volume, unmutePolicy=unmute_policy)
        return await self._send_audiocontrol_cmd(endpoint, cmd, expected_status)

    async def send_increase_volume(self, endpoint, step_size=None, unmute_policy=None, unmute_volume=None,
                                   expected_status: Status = Status.Success):
        cmd = _CLUSTER.Commands.IncreaseVolume(
            stepSize=step_size, unmutePolicy=unmute_policy, unmuteVolume=unmute_volume)
        return await self._send_audiocontrol_cmd(endpoint, cmd, expected_status)

    async def send_decrease_volume(self, endpoint, step_size=None, unmute_policy=None,
                                   expected_status: Status = Status.Success):
        cmd = _CLUSTER.Commands.DecreaseVolume(stepSize=step_size, unmutePolicy=unmute_policy)
        return await self._send_audiocontrol_cmd(endpoint, cmd, expected_status)

    # Convenience assertions

    async def verify_soft_muted(self, endpoint, expected: bool) -> bool | None:
        """Read SoftMuted and assert its value.

        SoftMuted is mandatory, so this normally always runs; None is returned only if a
        DUT omits it from its AttributeList, in which case the omission is logged.
        """
        if not self.supports_attribute(_CLUSTER.Attributes.SoftMuted):
            log.info("SoftMuted is not in the DUT's AttributeList; cannot verify it is %s", expected)
            return None
        value = await self.read_audiocontrol_attribute_expect_success(endpoint, _CLUSTER.Attributes.SoftMuted)
        asserts.assert_equal(value, expected, f"SoftMuted expected to be {expected}")
        return value

    async def verify_volume(self, endpoint, expected: int) -> int | None:
        """Read Volume and assert its value.

        Volume is mandatory, so this normally always runs; None is returned only if a DUT
        omits it from its AttributeList, in which case the omission is logged.
        """
        if not self.supports_attribute(_CLUSTER.Attributes.Volume):
            log.info("Volume is not in the DUT's AttributeList; cannot verify it is %d", expected)
            return None
        value = await self.read_audiocontrol_attribute_expect_success(endpoint, _CLUSTER.Attributes.Volume)
        asserts.assert_equal(value, expected, f"Volume expected to be {expected}")
        return value

    @staticmethod
    def pick_different_enum(current, enum_type):
        """Return a valid enum member of enum_type that differs from current."""
        for candidate in enum_type:
            if candidate == enum_type.kUnknownEnumValue:
                continue
            if candidate != current:
                return candidate
        asserts.fail(f"Could not find an alternative value for {enum_type.__name__}")
        return None

    @staticmethod
    def pick_different_in_range(current, low: int, high: int):
        """Return a value in [low, high] that differs from current, or None if impossible."""
        if low > high:
            return None
        if current is NullValue or current is None:
            return low
        for candidate in (current + 1, current - 1, low, high):
            if low <= candidate <= high and candidate != current:
                return candidate
        return None

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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ELECTRICAL_PROTECTION_APP}
#     app-args: >
#       --discriminator 1234
#       --KVS kvs1
#       --trace-to json:${TRACE_APP}.json
#       --enable-key 000102030405060708090a0b0c0d0e0f
#     app-ready-pattern: "APP STATUS: Starting event loop"
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --endpoint 2
#       --hex-arg enableKey:000102030405060708090a0b0c0d0e0f
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===


from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import InteractionModelError, Status
from matter.testing.decorators import has_feature, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

cluster = Clusters.ElectricalAlarm
attrs = cluster.Attributes
_F = cluster.Bitmaps.Feature

# The over/under threshold pairs, in the order the test plan exercises them. Each contributes a
# fourteen-step block (esalm_threshold_testcase.adoc) and the step counter runs continuously, so
# the blocks start at 13, 27, 41 and 55.
PAIRS = [
    ("OverVoltageThreshold", "UnderVoltageThreshold", _F.kOverVoltage, _F.kUnderVoltage),
    ("OverFrequencyThreshold", "UnderFrequencyThreshold", _F.kOverFrequency, _F.kUnderFrequency),
    ("OverPowerThreshold", "UnderPowerThreshold", _F.kOverPower, _F.kUnderPower),
    ("OverCurrentThreshold", "UnderCurrentThreshold", _F.kOverCurrent, _F.kUnderCurrent),
]

# Every threshold attribute, in the order steps 3-12 read them and steps 86-105 restore them.
ALL_THRESHOLDS = [
    ("OverVoltageThreshold", _F.kOverVoltage),
    ("UnderVoltageThreshold", _F.kUnderVoltage),
    ("OverFrequencyThreshold", _F.kOverFrequency),
    ("UnderFrequencyThreshold", _F.kUnderFrequency),
    ("OverPowerThreshold", _F.kOverPower),
    ("UnderPowerThreshold", _F.kUnderPower),
    ("OverCurrentThreshold", _F.kOverCurrent),
    ("UnderCurrentThreshold", _F.kUnderCurrent),
    ("PowerImportThreshold", _F.kPowerImport),
    ("PowerExportThreshold", _F.kPowerExport),
]

_SUCCESS = "Verify DUT responds w/ status SUCCESS(0x00)."
_CONSTRAINT = "Verify that the DUT response contains CONSTRAINT_ERROR."


def _kwarg(attribute_name: str) -> str:
    return attribute_name[0].lower() + attribute_name[1:]


# The plan's name for each stored original value, used verbatim in the step text.
_ORIG_NAMES = {
    "OverVoltageThreshold": "ORIG_OVT",
    "UnderVoltageThreshold": "ORIG_UVT",
    "OverFrequencyThreshold": "ORIG_OFT",
    "UnderFrequencyThreshold": "ORIG_UFT",
    "OverPowerThreshold": "ORIG_OPT",
    "UnderPowerThreshold": "ORIG_UPT",
    "OverCurrentThreshold": "ORIG_OCT",
    "UnderCurrentThreshold": "ORIG_UCT",
    "PowerImportThreshold": "ORIG_PIMT",
    "PowerExportThreshold": "ORIG_PEXT",
}


def _orig(attribute_name: str) -> str:
    return _ORIG_NAMES[attribute_name]


class TC_ESALM_2_2(MatterBaseTest):

    def desc_TC_ESALM_2_2(self) -> str:
        return "[TC-ESALM-2.2] AdjustableThresholds feature functionality with Server as DUT"

    def pics_TC_ESALM_2_2(self) -> list[str]:
        return ["ESALM.S", "ESALM.S.F20"]

    def _pair_steps(self, base: int, over: str, under: str) -> list[TestStep]:
        """The fourteen steps of esalm_threshold_testcase.adoc, starting at `base`."""
        o, u = _orig(over), _orig(under)
        return [
            TestStep(base, f"TH sends command SetElectricalAlarmThresholds with {over} set to {o} + 1000.",
                     _SUCCESS),
            TestStep(base + 1, f"TH reads from the DUT the {over}.",
                     f"Verify that the DUT response contains {o} + 1000."),
            TestStep(base + 2, f"TH sends command SetElectricalAlarmThresholds with {under} set to {u} - 1000.",
                     _SUCCESS),
            TestStep(base + 3, f"TH reads from the DUT the {under}.",
                     f"Verify that the DUT response contains {u} - 1000."),
            TestStep(base + 4, f"TH sends command SetElectricalAlarmThresholds with {over} set to the current "
                     f"{under} value (violates constraint min = {under} + 1).", _CONSTRAINT),
            TestStep(base + 5, f"TH reads from the DUT the {over}.",
                     f"Verify that the DUT response contains {o} + 1000 (constraint violation did not modify "
                     "the attribute)."),
            TestStep(base + 6, f"TH sends command SetElectricalAlarmThresholds with {under} set to the current "
                     f"{over} value (violates constraint max = {over} - 1).", _CONSTRAINT),
            TestStep(base + 7, f"TH reads from the DUT the {under}.",
                     f"Verify that the DUT response contains {u} - 1000 (constraint violation did not modify "
                     "the attribute)."),
            TestStep(base + 8, f"TH sends command SetElectricalAlarmThresholds with {over} set to {u} - 3000 "
                     f"and {under} set to {u} - 4000 in a single command (new values are mutually valid but "
                     f"would violate the current {over} if applied sequentially).",
                     _SUCCESS + " DUT evaluates constraints atomically on the final state."),
            TestStep(base + 9, f"TH reads from the DUT the {over}.",
                     f"Verify that the DUT response contains {u} - 3000."),
            TestStep(base + 10, f"TH reads from the DUT the {under}.",
                     f"Verify that the DUT response contains {u} - 4000."),
            TestStep(base + 11, f"TH sends command SetElectricalAlarmThresholds with {over} and {under} both "
                     f"set to {u} - 3000 (equal values violate constraint {over} >= {under} + 1).", _CONSTRAINT),
            TestStep(base + 12, f"TH reads from the DUT the {over}.",
                     f"Verify that the DUT response contains {u} - 3000 (constraint violation did not modify "
                     "the attribute)."),
            TestStep(base + 13, f"TH reads from the DUT the {under}.",
                     f"Verify that the DUT response contains {u} - 4000 (constraint violation did not modify "
                     "the attribute)."),
        ]

    def steps_TC_ESALM_2_2(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commission DUT to TH", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the FeatureMap attribute.",
                     "Verify that the DUT response contains a uint32 value. Store the value as FeatureMap."),
        ]
        for i, (name, _feature) in enumerate(ALL_THRESHOLDS):
            steps.append(TestStep(3 + i, f"TH reads from the DUT the {name}.",
                                  "Verify that the DUT response contains an int64 value. Store the value as "
                                  f"{_orig(name)}."))
        for i, (over, under, _fo, _fu) in enumerate(PAIRS):
            steps.extend(self._pair_steps(13 + 14 * i, over, under))

        steps += [
            TestStep(69, "TH sends command SetElectricalAlarmThresholds with PowerImportThreshold set to 1000 "
                     "(absolute value; PowerImportThreshold is constrained to min 0 so relative-delta approach "
                     "is not used).", _SUCCESS),
            TestStep(70, "TH reads from the DUT the PowerImportThreshold.",
                     "Verify that the DUT response contains 1000."),
            TestStep(71, "TH sends command SetElectricalAlarmThresholds with PowerImportThreshold set to -1 "
                     "(violates min 0).", _CONSTRAINT),
            TestStep(72, "TH reads from the DUT the PowerImportThreshold.",
                     "Verify that the DUT response contains 1000 (constraint violation did not modify the "
                     "attribute)."),
            TestStep(73, "TH sends command SetElectricalAlarmThresholds with PowerExportThreshold set to -1000 "
                     "(absolute value; PowerExportThreshold is constrained to max 0).", _SUCCESS),
            TestStep(74, "TH reads from the DUT the PowerExportThreshold.",
                     "Verify that the DUT response contains -1000."),
            TestStep(75, "TH sends command SetElectricalAlarmThresholds with PowerExportThreshold set to 1 "
                     "(violates max 0).", _CONSTRAINT),
            TestStep(76, "TH reads from the DUT the PowerExportThreshold.",
                     "Verify that the DUT response contains -1000 (constraint violation did not modify the "
                     "attribute)."),
            TestStep(77, "TH sends command SetElectricalAlarmThresholds with PowerImportThreshold set to 2000 "
                     "and PowerExportThreshold set to 1 in a single command (PowerExportThreshold violates "
                     "max 0).", _CONSTRAINT),
            TestStep(78, "TH reads from the DUT the PowerImportThreshold.",
                     "Verify that the DUT response contains 1000 (constraint violation did not modify the "
                     "attribute)."),
            TestStep(79, "TH reads from the DUT the PowerExportThreshold.",
                     "Verify that the DUT response contains -1000 (constraint violation did not modify the "
                     "attribute)."),
            TestStep(80, "TH sends command SetElectricalAlarmThresholds with PowerImportThreshold set to 2000 "
                     "and PowerExportThreshold set to -2000 in a single command (new values satisfy all "
                     "constraints: PowerImportThreshold >= 0, PowerExportThreshold <= 0, PowerImportThreshold "
                     ">= PowerExportThreshold + 1).",
                     _SUCCESS + " DUT evaluates constraints atomically on the final state."),
            TestStep(81, "TH reads from the DUT the PowerImportThreshold.",
                     "Verify that the DUT response contains 2000."),
            TestStep(82, "TH reads from the DUT the PowerExportThreshold.",
                     "Verify that the DUT response contains -2000."),
            TestStep(83, "TH sends command SetElectricalAlarmThresholds with PowerImportThreshold set to 0 and "
                     "PowerExportThreshold set to 0 in a single command (equal values violate constraint "
                     "PowerImportThreshold >= PowerExportThreshold + 1).", _CONSTRAINT),
            TestStep(84, "TH reads from the DUT the PowerImportThreshold.",
                     "Verify that the DUT response contains 2000 (constraint violation did not modify the "
                     "attribute)."),
            TestStep(85, "TH reads from the DUT the PowerExportThreshold.",
                     "Verify that the DUT response contains -2000 (constraint violation did not modify the "
                     "attribute)."),
        ]
        for i, (name, _feature) in enumerate(ALL_THRESHOLDS):
            o = _orig(name)
            steps.append(TestStep(86 + 2 * i,
                                  f"TH sends command SetElectricalAlarmThresholds with {name} set to {o}.",
                                  _SUCCESS))
            steps.append(TestStep(87 + 2 * i, f"TH reads from the DUT the {name}.",
                                  f"Verify that the DUT response contains {o}."))
        return steps

    async def _set(self, endpoint: int, **kwargs) -> None:
        await self.send_single_cmd(cmd=cluster.Commands.SetElectricalAlarmThresholds(**kwargs),
                                   endpoint=endpoint)

    async def _set_expect_constraint_error(self, endpoint: int, **kwargs) -> None:
        try:
            await self.send_single_cmd(cmd=cluster.Commands.SetElectricalAlarmThresholds(**kwargs),
                                       endpoint=endpoint)
            asserts.fail(f"Expected CONSTRAINT_ERROR for {kwargs} but the command succeeded")
        except InteractionModelError as e:
            asserts.assert_equal(e.status, Status.ConstraintError,
                                 f"Expected CONSTRAINT_ERROR for {kwargs}, got {e.status}")

    async def _read(self, endpoint: int, name: str) -> int:
        return await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=getattr(attrs, name))

    async def _read_step(self, step, endpoint: int, name: str, expected: int) -> None:
        self.step(step)
        asserts.assert_equal(await self._read(endpoint, name), expected, f"{name} mismatch")

    def _skip(self, first: int, count: int) -> None:
        for offset in range(count):
            self.step(first + offset)
            self.mark_current_step_skipped()

    async def _pair_block(self, endpoint: int, base: int, over: str, under: str,
                          orig_over, orig_under) -> None:
        """Execute the fourteen steps declared by _pair_steps for one over/under pair.

        The plan gates steps 1 and 2 of the block on the Over feature, steps 3 and 4 on the Under
        feature, and the remaining ten on both, so orig_over or orig_under may be None.
        """
        ko, ku = _kwarg(over), _kwarg(under)
        has_over, has_under = orig_over is not None, orig_under is not None

        if has_over:
            new_over = orig_over + 1000
            self.step(base)
            await self._set(endpoint, **{ko: new_over})
            await self._read_step(base + 1, endpoint, over, new_over)
        else:
            self._skip(base, 2)

        if has_under:
            new_under = orig_under - 1000
            self.step(base + 2)
            await self._set(endpoint, **{ku: new_under})
            await self._read_step(base + 3, endpoint, under, new_under)
        else:
            self._skip(base + 2, 2)

        if not (has_over and has_under):
            self._skip(base + 4, 10)
            return

        self.step(base + 4)
        await self._set_expect_constraint_error(endpoint, **{ko: new_under})
        await self._read_step(base + 5, endpoint, over, new_over)

        self.step(base + 6)
        await self._set_expect_constraint_error(endpoint, **{ku: new_over})
        await self._read_step(base + 7, endpoint, under, new_under)

        # Mutually valid values that would violate the constraint if applied one at a time.
        both_over, both_under = orig_under - 3000, orig_under - 4000
        self.step(base + 8)
        await self._set(endpoint, **{ko: both_over, ku: both_under})
        await self._read_step(base + 9, endpoint, over, both_over)
        await self._read_step(base + 10, endpoint, under, both_under)

        self.step(base + 11)
        await self._set_expect_constraint_error(endpoint, **{ko: both_over, ku: both_over})
        await self._read_step(base + 12, endpoint, over, both_over)
        await self._read_step(base + 13, endpoint, under, both_under)

    @run_if_endpoint_matches(has_feature(cluster, _F.kAdjustableThresholds))
    async def test_TC_ESALM_2_2(self):
        endpoint = self.get_endpoint()

        self.step(1)

        self.step(2)
        feature_map = await self.read_single_attribute_check_success(
            endpoint=endpoint, cluster=cluster, attribute=attrs.FeatureMap)

        originals: dict[str, int] = {}
        for i, (name, feature) in enumerate(ALL_THRESHOLDS):
            self.step(3 + i)
            if not feature_map & feature:
                self.mark_current_step_skipped()
                continue
            originals[name] = await self._read(endpoint, name)

        for i, (over, under, feat_over, feat_under) in enumerate(PAIRS):
            base = 13 + 14 * i
            if over in originals or under in originals:
                await self._pair_block(endpoint, base, over, under,
                                       originals.get(over), originals.get(under))
            else:
                self._skip(base, 14)

        has_import = "PowerImportThreshold" in originals
        has_export = "PowerExportThreshold" in originals

        if has_import:
            self.step(69)
            await self._set(endpoint, powerImportThreshold=1000)
            await self._read_step(70, endpoint, "PowerImportThreshold", 1000)
            self.step(71)
            await self._set_expect_constraint_error(endpoint, powerImportThreshold=-1)
            await self._read_step(72, endpoint, "PowerImportThreshold", 1000)
        else:
            self._skip(69, 4)

        if has_export:
            self.step(73)
            await self._set(endpoint, powerExportThreshold=-1000)
            await self._read_step(74, endpoint, "PowerExportThreshold", -1000)
            self.step(75)
            await self._set_expect_constraint_error(endpoint, powerExportThreshold=1)
            await self._read_step(76, endpoint, "PowerExportThreshold", -1000)
        else:
            self._skip(73, 4)

        if has_import and has_export:
            self.step(77)
            await self._set_expect_constraint_error(endpoint, powerImportThreshold=2000,
                                                    powerExportThreshold=1)
            await self._read_step(78, endpoint, "PowerImportThreshold", 1000)
            await self._read_step(79, endpoint, "PowerExportThreshold", -1000)

            self.step(80)
            await self._set(endpoint, powerImportThreshold=2000, powerExportThreshold=-2000)
            await self._read_step(81, endpoint, "PowerImportThreshold", 2000)
            await self._read_step(82, endpoint, "PowerExportThreshold", -2000)

            self.step(83)
            await self._set_expect_constraint_error(endpoint, powerImportThreshold=0,
                                                    powerExportThreshold=0)
            await self._read_step(84, endpoint, "PowerImportThreshold", 2000)
            await self._read_step(85, endpoint, "PowerExportThreshold", -2000)
        else:
            self._skip(77, 9)

        # Restore every threshold the DUT supports to the value read in steps 3-12.
        for i, (name, _feature) in enumerate(ALL_THRESHOLDS):
            first = 86 + 2 * i
            if name not in originals:
                self._skip(first, 2)
                continue
            self.step(first)
            await self._set(endpoint, **{_kwarg(name): originals[name]})
            await self._read_step(first + 1, endpoint, name, originals[name])


if __name__ == "__main__":
    default_matter_test_main()

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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${CAMERA_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --endpoint 1
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import chip.clusters as Clusters
from chip.interaction_model import Status
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_CHIMETestBase import CHIMETestBase


class TC_CHIME_2_3(MatterBaseTest, CHIMETestBase):

    def desc_TC_CHIME_2_3(self) -> str:
        return "[TC-CHIME-2.3] Verify that the SelectedChime can be changed"

    def steps_TC_CHIME_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read the InstalledChimeSounds attribute, store as myChimeSounds."),
            TestStep(3, "Read the SelectedChime attribute, store as mySelectedChime"),
            TestStep(4, "If myChimeSounds has only 1 value, jump to step 7."),
            TestStep(5, "Write to SelectedChime a new value from myChimeSounds."),
            TestStep(6, "Read the SelectedChime attribute, verfy that it's the same as the value written in step 5"),
            TestStep(7, "Write to SelectedChime a value not found in myChimeSounds. Verify a NotFound error response"),
            TestStep(8, "Read the SelectedChime attribute, verfy that it's unchanged"),

        ]
        return steps

    def pics_TC_CHIME_2_3(self) -> list[str]:
        pics = [
            "CHIME.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.Chime))
    async def test_TC_CHIME_2_3(self):
        cluster = Clusters.Objects.Chime
        attributes = cluster.Attributes
        endpoint = self.get_endpoint(default=1)

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        myChimeSounds = await self.read_chime_attribute_expect_success(endpoint, attributes.InstalledChimeSounds)

        self.step(3)
        mySelectedChime = await self.read_chime_attribute_expect_success(endpoint, attributes.SelectedChime)

        self.step(4)
        if len(myChimeSounds) > 1:
            self.step(5)
            newSelectedChime = mySelectedChime
            for chime in myChimeSounds:
                if chime.chimeID != mySelectedChime:
                    newSelectedChime = chime.chimeID
                    break
            await self.write_chime_attribute_expect_success(endpoint, attributes.SelectedChime, newSelectedChime)

            self.step(6)
            mySelectedChime = await self.read_chime_attribute_expect_success(endpoint, attributes.SelectedChime)
            asserts.assert_equal(mySelectedChime, newSelectedChime, "Read SelectedChime does not match written SelectedChime")
        else:
            self.skip_step(5)
            self.skip_step(6)

        self.step(7)
        foundNotPresent = False
        valueToUse = 0
        for i in range(256):
            if not any(chime.chimeID == i for chime in myChimeSounds):
                foundNotPresent = True
                valueToUse = i
                break

        if foundNotPresent:
            await self.write_chime_attribute_expect_failure(endpoint, attributes.SelectedChime, valueToUse, Status.NotFound)

            self.step(8)
            step8SelectedChime = await self.read_chime_attribute_expect_success(endpoint, attributes.SelectedChime)

            asserts.assert_equal(step8SelectedChime, mySelectedChime, "SelectedChime has been written to an invalid value")
        else:
            self.skip_step(8)


if __name__ == "__main__":
    default_matter_test_main()

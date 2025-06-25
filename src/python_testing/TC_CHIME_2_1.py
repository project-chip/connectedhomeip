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
from chip.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_cluster, run_if_endpoint_matches
from mobly import asserts
from TC_CHIMETestBase import CHIMETestBase


class TC_CHIME_2_1(MatterBaseTest, CHIMETestBase):

    def desc_TC_CHIME_2_1(self) -> str:
        return "[TC-CHIME-2.1] Attributes with DUT as Server"

    def steps_TC_CHIME_2_1(self) -> list[TestStep]:
        steps = [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Read InstalledChimeSounds, verify list and field value conformance"),
            TestStep(3, "Verify that all of the ChimeIDs in InstalledChimeSounds are unique"),
            TestStep(4, "Verify that all of the Names in InstalledChimeSounds are unique"),
            TestStep(5, "Read and verify SelectedChime attribute"),
            TestStep(6, "Read and verify Enabled attribute"),
        ]
        return steps

    def pics_TC_CHIME_2_1(self) -> list[str]:
        pics = [
            "CHIME.S",
        ]
        return pics

    @run_if_endpoint_matches(has_cluster(Clusters.Chime))
    async def test_TC_CHIME_2_1(self):
        cluster = Clusters.Objects.Chime
        attributes = cluster.Attributes
        endpoint = self.get_endpoint(default=1)

        self.step(1)  # Already done, immediately go to step 2

        self.step(2)
        myChimeSounds = await self.read_chime_attribute_expect_success(endpoint, attributes.InstalledChimeSounds)
        asserts.assert_greater_equal(len(myChimeSounds), 1, "InstalledChimeSounds contains fewer than the minimum number of chimes")
        asserts.assert_less_equal(len(myChimeSounds), 255,
                                  "InstalledChimeSounds contains more than the maximum number of chimes")

        myChimeIDs = []
        myChimeNames = []
        for chime in myChimeSounds:
            asserts.assert_greater_equal(chime.chimeID, 0, "ChimeID must be zero or greater")
            asserts.assert_less_equal(chime.chimeID, 255, "ChimeID must be less than or equal to 255")
            asserts.assert_greater_equal(len(chime.name), 1, "Name must be at least 1 character or greater")
            asserts.assert_less_equal(len(chime.name), 48, "Name must be no more than 48 characters")
            myChimeIDs.append(chime.chimeID)
            myChimeNames.append(chime.name)

        self.step(3)
        asserts.assert_equal(len(myChimeIDs), len(set(myChimeIDs)), "Chime IDs non-unique")

        self.step(4)
        asserts.assert_equal(len(myChimeNames), len(set(myChimeNames)), "Chime Names non-unique")

        self.step(5)
        mySelectedChime = await self.read_chime_attribute_expect_success(endpoint, attributes.SelectedChime)
        asserts.assert_greater_equal(mySelectedChime, 0, "SelectedChime must be zero or greater")
        asserts.assert_less_equal(mySelectedChime, 255, "SelectedChime must be less than or equal to 255")

        self.step(6)
        myEnabled = await self.read_chime_attribute_expect_success(endpoint, attributes.Enabled)
        if not isinstance(myEnabled, bool):
            asserts.fail("Enabled is not a boolean value")


if __name__ == "__main__":
    default_matter_test_main()

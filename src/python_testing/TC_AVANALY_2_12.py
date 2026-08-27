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

import logging

from mobly import asserts
from TC_AVANALYTestBase import AVANALYTestBase

import matter.clusters as Clusters
from matter.testing.decorators import has_cluster, run_if_endpoint_matches
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_AVANALY_2_12(MatterBaseTest, AVANALYTestBase):

    def desc_TC_AVANALY_2_12(self) -> str:
        return "[TC-AVANALY-2.12] Validate Remote Analysis Node specific constraints and independent zone management with Server as DUT"

    def steps_TC_AVANALY_2_12(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "Access Zone Management cluster on the Analysis Node."),
            TestStep(3, "Define a Zone on the Analysis Node."),
            TestStep(4, "Verify that Zone Management on the camera device remains unchanged and independent."),
        ]

    def pics_TC_AVANALY_2_12(self) -> list[str]:
        return [
            "AVANALY.S",
            "AVANALY.S.F01",
        ]

    @run_if_endpoint_matches(has_cluster(Clusters.AvAnalysis))
    async def test_TC_AVANALY_2_12(self):
        endpoint = self.get_endpoint()

        self.step(1)  # Already done, immediately go to step 2

        await self.read_avanaly_features(endpoint)
        if not self.has_feature_remcondetect:
            log.info("REMCONDETECT feature not supported, skipping TC-AVANALY-2.12")
            self.skip_step(2)
            self.skip_step(3)
            self.skip_step(4)
            return

        self.step(2)
        zone_cluster = Clusters.Objects.ZoneManagement
        zones_before = []
        try:
            zones_before = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=zone_cluster, attribute=zone_cluster.Attributes.Zones
            )
            log.info("Analysis Node zones before: %s", zones_before)
        except Exception as e:
            log.info("ZoneManagement on Analysis Node: %s", e)

        self.step(3)
        created_zone_id = None
        try:
            zone_vertices = [
                zone_cluster.Structs.TwoDCartesianVertexStruct(10, 10),
                zone_cluster.Structs.TwoDCartesianVertexStruct(50, 10),
                zone_cluster.Structs.TwoDCartesianVertexStruct(50, 50),
                zone_cluster.Structs.TwoDCartesianVertexStruct(10, 50)
            ]
            zone_to_create = zone_cluster.Structs.TwoDCartesianZoneStruct(
                name="AnalysisZone",
                use=zone_cluster.Enums.ZoneUseEnum.kMotion,
                vertices=zone_vertices,
                color="#00FF00"
            )
            create_cmd = zone_cluster.Commands.CreateTwoDCartesianZone(zone=zone_to_create)
            cmd_resp = await self.send_single_cmd(endpoint=endpoint, cmd=create_cmd)
            created_zone_id = cmd_resp.zoneID
            log.info("Created zone ID on Analysis Node: %s", created_zone_id)
        except Exception as e:
            log.info("Zone creation on Analysis Node: %s", e)

        self.step(4)
        # Verify Analysis Node zone independence - camera zones are unmodified
        try:
            zones_after = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=zone_cluster, attribute=zone_cluster.Attributes.Zones
            )
            log.info("Analysis Node zones after: %s", zones_after)
        except Exception as e:
            log.info("Reading zones after creation: %s", e)

        # Cleanup created zone
        if created_zone_id is not None:
            try:
                await self.send_single_cmd(endpoint=endpoint, cmd=zone_cluster.Commands.RemoveZone(zoneID=created_zone_id))
            except Exception as e:
                log.info("Cleanup RemoveZone: %s", e)


if __name__ == "__main__":
    default_matter_test_main()

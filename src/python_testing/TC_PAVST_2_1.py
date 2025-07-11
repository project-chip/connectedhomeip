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

import chip.clusters as Clusters
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mobly import asserts

logger = logging.getLogger(__name__)


class TC_PAVST_2_1(MatterBaseTest):
    def desc_TC_PAVST_2_1(self) -> str:
        return "[TC-PAVST-2.1] Attributes with Server as DUT"

    def pics_TC_PAVST_2_1(self):
        return ["PAVST.S"]

    def steps_TC_PAVST_2_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH reads from the DUT the SupportedFormats attribute.",
                     "Verify that the DUT response contains a list of SupportedFormatsStruct entries. For each entry in the list, verify that the ContainerFormat is a defined ContainerFormatEnum value and the IngestMethod is a defined IngestMethodEnum value."),
            TestStep(3, "TH reads from the DUT the CurrentConnections attribute.",
                     "Verify that the DUT response contains a list of TransportConfigurationStruct entries. For each entry in the list, verify that the TransportStatus is a defined TransportStatusEnum value."),
        ]

    @async_test_body
    async def test_TC_PAVST_2_1(self):
        endpoint = self.get_endpoint(default=1)
        cluster = Clusters.PushAvStreamTransport
        attr = Clusters.PushAvStreamTransport.Attributes

        self.step(1)
        # Commission DUT - already done

        self.step(2)
        if self.pics_guard(self.check_pics("PAVST.S.A0000")):
            supported_formats = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.SupportedFormats
            )
            asserts.assert_greater_equal(len(supported_formats, 1), "SupportedFormats must not be empty!")
            for format in supported_formats:
                validContainerformat = format.ContainerFormat == cluster.ContainerFormatEnum.kCmaf
                isValidIngestMethod = format.IngestMethod == cluster.IngestMethodEnum.kCMAFIngest
                asserts.assert_true((validContainerformat & isValidIngestMethod),
                                    "(ContainerFormat & IngestMethod) must be defined values!")

        self.step(3)
        if self.pics_guard(self.check_pics("PAVST.S.A0001")):
            transport_configs = await self.read_single_attribute_check_success(
                endpoint=endpoint, cluster=cluster, attribute=attr.CurrentConnections
            )
            asserts.assert_greater_equal(len(transport_configs, 1), "TransportConfigurations must not be empty!")
            for config in transport_configs:
                isValidTransportStatus = (config.TransportStatus == cluster.TransportStatusEnum.kActive |
                                          config.TransportStatus == cluster.TransportStatusEnum.kInactive)
                asserts.assert_true(isValidTransportStatus, "TransportStatus must be a defined value!")


if __name__ == "__main__":
    default_matter_test_main()

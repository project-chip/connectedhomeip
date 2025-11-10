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
#

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${OTA_REQUESTOR_APP}
#     app-args: >
#       --discriminator 1234
#       --passcode 20202021
#       --secured-device-port 5541
#       --KVS /tmp/chip_kvs_requestor
#       --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --vendor-id 65521
#       --product-id 32769
#       --endpoint 0
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio
import logging
import queue
import threading
import time

from mobly import asserts
from TC_SUTestBase import SoftwareUpdateBaseTest, ACLHandler


import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.clusters.Types import NullValue
from matter.testing.apps import OTAProviderSubprocess
from matter.testing.event_attribute_reporting import AttributeMatcher, AttributeSubscriptionHandler, EventSubscriptionHandler
from matter.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main

# Create a logger
logger = logging.getLogger(__name__)


class TC_SU_2_3(SoftwareUpdateBaseTest):

    def desc_TC_SU_2_3(self) -> str:
        return "[TC-SU-2.3] Transfer of Software Update Images between DUT and TH/OTA-P"

    def pics_TC_SU_2_2(self):
        """Return the PICS definitions associated with this test."""
        pics = [
            "MCORE.OTA.Requestor",      # Pics
        ]
        return pics

    def steps_TC_SU_2_3(self) -> list[TestStep]:
        steps = [
            TestStep(0, "Prerequisite: Commission the DUT (Requestor) with the TH/OTA-P (Provider)",
                     is_commissioning=True),
            TestStep(1, "DUT sends a QueryImage command to the TH/OTA-P. RequestorCanConsent is set to True by DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. "
                     "OTA-P/TH responds with a QueryImageResponse with UserConsentNeeded field set to True.",
                     "Verify that the DUT obtains the User Consent from the user prior to transfer of software update image. This step is vendor specific."),
            TestStep(2, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. Set ImageURI to the location where the image is located.",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT. "
                     "Verify that the Maximum Block Size requested by DUT should be: "
                     "- no larger than 1024 (2^10) bytes over non-TCP transports. "
                     "- no larger than 8192 (2^13) bytes over TCP transport."),
            TestStep(3, "DUT sends a QueryImage command to the TH/OTA-P. TH/OTA-P sends a QueryImageResponse back to DUT. "
                     "QueryStatus is set to 'UpdateAvailable'. Set ImageURI with the https url of the software image.",
                     "Verify that there is a transfer of the software image from the TH/OTA-P to the DUT "
                     "from the https url and not from the OTA-P."),
            TestStep(4, "During the transfer of the image to the DUT, force fail the transfer before it completely transfers the image. "
                     "Wait for the Idle timeout so that reading the UpdateState Attribute of the OTA Requestor returns the value as Idle. "
                     "Initiate another QueryImage Command from DUT to the TH/OTA-P.",
                     "Verify that the BDX Idle timeout should be no less than 5 minutes. "
                     "Verify that the DUT starts a new transfer of software image when sending another QueryImage request."),
            TestStep(5, "During the transfer of the image to the DUT, force fail the transfer before it completely transfers the image. "
                     "Initiate another QueryImage Command from DUT to the TH/OTA-P. "
                     "Set the RC[STARTOFS] bit and associated STARTOFS field in the ReceiveInit Message to indicate the resumption of a transfer previously aborted.",
                     "ImageURI should have the https url from where the image can be downloaded.",
                     "Verify that the DUT starts receiving the rest of the software image after resuming the image transfer.")
        ]
        return steps

    @async_test_body
    async def teardown_test(self):
        self.current_provider_app_proc.terminate()
        super().teardown_test()

    @async_test_body
    async def test_TC_SU_2_3(self):
        # ------------------------------------------------------------------------------------
        # Run script
        # ------------------------------------------------------------------------------------
        # Run Python test with commission Provisioner/Requestor from Terminal:
        #   python3 src/python_testing/TC_SU_2_3.py \
        #   --commissioning-method on-network \
        #   --discriminator 1234 \
        #   --passcode 20202021 \
        #   --admin-vendor-id 65521 \
        #   --int-arg product-id:32769 \
        #   --nodeId 2
        # ------------------------------------------------------------------------------------

        self.step(0)
        self.step(1)
        self.step(2)
        self.step(3)
        # NOTE: Step skipped not implemented in spec.
        self.step(4)
        self.step(5)
        # NOTE: Step skipped not implemented in spec.


if __name__ == "__main__":
    default_matter_test_main()

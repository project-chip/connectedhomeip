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
#

# The DUT is a Proximity Ranging client (commissioner) that originates the commands under
# test, so this test is driven by an operator working the DUT's own interface. The
# is_pics_sdk_ci_only path lets the harness controller stand in for the client so the test
# also runs unattended in CI.
#
# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     script-args: >
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --storage-path admin_storage.json
#       --string-arg th_server_app_path:${ALL_DEVICES_APP}
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

from support_modules.proximity_ranger_th_server import BLERBC_SPEC, BLTCS_SPEC, WIFI_SPEC, ProximityRangerTHServerTest

from matter.testing.decorators import async_test_body
from matter.testing.runner import TestStep, default_matter_test_main

PICS_C = "PROXR.C"


class TC_PROXR_3_1(ProximityRangerTHServerTest):

    def desc_TC_PROXR_3_1(self) -> str:
        return "[TC-PROXR-3.1] Trigger Instant Proximity Ranging (DUT as Client)"

    def pics_TC_PROXR_3_1(self) -> list[str]:
        return [PICS_C]

    def steps_TC_PROXR_3_1(self) -> list[TestStep]:
        return [
            TestStep(1, "Commission TH_I and TH_R to DUT.",
                     "TH_I and TH_R are commissioned and reachable by the client."),
            TestStep(2, "Enable the WFUSDPD feature and configure the Technology to WiFiRoundTripTimeRanging and "
                        "disable other Proximity Ranging features on TH_I and TH_R."),
            TestStep(3, "Trigger DUT to read all attributes from TH_I and TH_R.",
                     "DUT discovers TH_I and TH_R and obtains their attributes."),
            TestStep(4, "Trigger DUT to send StartRangingRequest to TH_I (subscriber) and TH_R (publisher), "
                        "which does not include the RangingInstanceInterval field.",
                     "DUT sends StartRangingRequest to TH_I with Role=WiFiSubscriberRole and PeerWiFiDevIK=TH_R's "
                     "WiFiDevIK, and to TH_R with Role=WiFiPublisherRole and PeerWiFiDevIK=TH_I's WiFiDevIK; PMK "
                     "common to both; StartTime 0."),
            TestStep(5, "TH_I and TH_R each send a StartRangingResponse with a SessionID.",
                     "Each SessionID is non-zero and, for that instance, has incremented over its previous "
                     "StartRangingResponse (uint8 wrap-around allowed)."),
            TestStep(6, "TH_I emits a RangingResult event; DUT obtains it and sends a StopRangingRequest to TH_R "
                        "with TH_R's SessionID.",
                     "DUT obtains the RangingResult event and TH_R stops the session named by its SessionID."),
            TestStep(7, "Enable the BLTCS feature and disable other Proximity Ranging features on TH_I and TH_R; "
                        "configure BLTCSSecurityLevel and BLTCSModeCapability the same on both."),
            TestStep(8, "Trigger DUT to read all attributes from TH_I and TH_R.",
                     "DUT discovers TH_I and TH_R and obtains their attributes."),
            TestStep(9, "Trigger DUT to send StartRangingRequest to TH_I (initiator) and TH_R (reflector), "
                        "which does not include the RangingInstanceInterval field.",
                     "DUT sends StartRangingRequest to TH_I with Role=BLTInitiatorRole and PeerBLTDevIK=TH_R's "
                     "BLTDevIK, and to TH_R with Role=BLTReflectorRole and PeerBLTDevIK=TH_I's BLTDevIK; LTK, "
                     "BLTCSMode and BLTCSSecurityLevel common to both; StartTime 0."),
            TestStep(10, "TH_I and TH_R each send a StartRangingResponse with a SessionID.",
                     "Each SessionID is non-zero and, for that instance, has incremented over its previous "
                     "StartRangingResponse (uint8 wrap-around allowed)."),
            TestStep(11, "TH_I emits a RangingResult event; DUT obtains it and sends a StopRangingRequest to TH_R "
                         "with TH_R's SessionID.",
                     "DUT obtains the RangingResult event and TH_R stops the session named by its SessionID."),
            TestStep(12, "Enable the BLERBC feature and disable other Proximity Ranging features on TH_I and TH_R."),
            TestStep(13, "Trigger DUT to read all attributes from TH_I and TH_R.",
                     "DUT discovers TH_I and TH_R and obtains their attributes."),
            TestStep(14, "Trigger DUT to send StartRangingRequest to TH_I (scanning) and TH_R (beacon), "
                         "which does not include the RangingInstanceInterval field.",
                     "DUT sends StartRangingRequest to TH_I with Role=BLEScanningRole and PeerBLEDeviceID=TH_R's "
                     "BLEDeviceID, and to TH_R with Role=BLEBeaconRole and PeerBLEDeviceID=TH_I's BLEDeviceID; "
                     "SessionKey and BLERBCSecurityMode common to both; StartTime 0."),
            TestStep(15, "TH_I and TH_R each send a StartRangingResponse with a SessionID.",
                     "Each SessionID is non-zero and, for that instance, has incremented over its previous "
                     "StartRangingResponse (uint8 wrap-around allowed)."),
            TestStep(16, "TH_I emits a RangingResult event; DUT obtains it and sends a StopRangingRequest to TH_R "
                         "with TH_R's SessionID.",
                     "DUT obtains the RangingResult event and TH_R stops the session named by its SessionID."),
        ]

    @async_test_body
    async def test_TC_PROXR_3_1(self):
        self.step(1)
        await self.commission_th_servers_onto_harness()
        if not self.is_pics_sdk_ci_only:
            await self.ask_dut_to_commission_th_servers()

        # Steps 2, 7, 12 configure a single-feature FeatureMap on the TH. The reference
        # all-devices-app hardcodes all three ranging adapters (LoggingProximityRanger.cpp)
        # and derives the FeatureMap from them with only a build-time switch
        # (ProximityRanger.h), so a single-feature precondition is not representable. The
        # server still accepts a single-technology StartRangingRequest, so the passes run;
        # only the isolation precondition cannot be enforced.
        self.skip_step(2)

        await self.run_technology_pass(WIFI_SPEC, periodic=False,
                                       read_step=3, start_step=4, response_step=5, final_step=6)

        self.skip_step(7)
        await self.run_technology_pass(BLTCS_SPEC, periodic=False,
                                       read_step=8, start_step=9, response_step=10, final_step=11)

        self.skip_step(12)
        await self.run_technology_pass(BLERBC_SPEC, periodic=False,
                                       read_step=13, start_step=14, response_step=15, final_step=16)


if __name__ == "__main__":
    default_matter_test_main()

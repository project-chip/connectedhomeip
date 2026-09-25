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
# also runs unattended in CI. This is the periodic variant of TC-PROXR-3.1: the
# StartRangingRequest carries a RangingInstanceInterval of 3 and the TH emits one
# RangingResult per interval (there is no Stop step).
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

from matter.testing.decorators import async_test_body, pics
from matter.testing.runner import default_matter_test_main


class TC_PROXR_3_2(ProximityRangerTHServerTest):

    def desc_TC_PROXR_3_2(self) -> str:
        return "[TC-PROXR-3.2] Trigger Periodic Proximity Ranging (DUT as Client)"

    @pics("PROXR.C")
    @async_test_body
    async def test_TC_PROXR_3_2(self):
        self.step(1, "Commission TH_I and TH_R to DUT.",
                  expectation="TH_I and TH_R are commissioned and reachable by the client.")
        await self.commission_th_servers_onto_harness()
        if not self.is_pics_sdk_ci_only:
            await self.ask_dut_to_commission_th_servers()

        # Steps 2, 7, 12 configure a single-feature FeatureMap on the TH; not representable on
        # the reference all-devices-app (see TC_PROXR_3_1 for the root cause). Additionally, on
        # this app only the BLERBC adapter advertises periodicRangingSupport=true
        # (LoggingProximityRanger.cpp); the WiFi and BLTCS periodic passes still exercise the
        # command path because the server validates only that the interval is non-zero
        # (ProximityRangingCluster.cpp), but they do not confirm the DUT honours the capability.
        # Each is advanced then marked skipped (what skip_step does), keeping all 16 steps in
        # order for the runner.
        self.step(2, "Enable the WFUSDPD feature and configure the Technology to WiFiRoundTripTimeRanging and "
                  "disable other Proximity Ranging features on TH_I and TH_R.")
        self.mark_current_step_skipped()

        self.step(3, "Trigger DUT to read all attributes from TH_I and TH_R.",
                  expectation="DUT discovers TH_I and TH_R and obtains their attributes.")
        peer_for_i, peer_for_r = await self.read_pass_attributes(WIFI_SPEC)
        async with self.ranging_result_subscription() as result_handler:
            self.step(4, "Trigger DUT to send StartRangingRequest to TH_I (subscriber) and TH_R (publisher), with "
                      "StartTime 0, EndTime 30 and RangingInstanceInterval 3.",
                      expectation="DUT sends StartRangingRequest to TH_I with Role=WiFiSubscriberRole and "
                      "PeerWiFiDevIK=TH_R's WiFiDevIK, and to TH_R with Role=WiFiPublisherRole and "
                      "PeerWiFiDevIK=TH_I's WiFiDevIK; PMK common to both; StartTime 0, EndTime 30, "
                      "RangingInstanceInterval 3 on TH_I.")
            await self.send_start_ranging(WIFI_SPEC, peer_for_i, peer_for_r, periodic=True)
            self.step(5, "TH_I and TH_R each send a StartRangingResponse with a SessionID.",
                      expectation="Each SessionID is non-zero and, for that instance, has incremented over its "
                      "previous StartRangingResponse (uint8 wrap-around allowed).")
            await self.verify_start_responses(WIFI_SPEC)
            self.step(6, "TH_I emits a RangingResult event once every 3 seconds; DUT obtains the events.",
                      expectation="DUT obtains one RangingResult per interval (~3 s cadence, +/- 3 s).")
            await self.verify_periodic_cadence(WIFI_SPEC, result_handler)

        self.step(7, "Enable the BLTCS feature and disable other Proximity Ranging features on TH_I and TH_R; "
                  "configure BLTCSSecurityLevel and BLTCSModeCapability the same on both.")
        self.mark_current_step_skipped()

        self.step(8, "Trigger DUT to read all attributes from TH_I and TH_R.",
                  expectation="DUT discovers TH_I and TH_R and obtains their attributes.")
        peer_for_i, peer_for_r = await self.read_pass_attributes(BLTCS_SPEC)
        async with self.ranging_result_subscription() as result_handler:
            self.step(9, "Trigger DUT to send StartRangingRequest to TH_I (initiator) and TH_R (reflector), with "
                      "StartTime 0, EndTime 30 and RangingInstanceInterval 3.",
                      expectation="DUT sends StartRangingRequest to TH_I with Role=BLTInitiatorRole and "
                      "PeerBLTDevIK=TH_R's BLTDevIK, and to TH_R with Role=BLTReflectorRole and "
                      "PeerBLTDevIK=TH_I's BLTDevIK; LTK, BLTCSMode and BLTCSSecurityLevel common to both; "
                      "StartTime 0, EndTime 30, RangingInstanceInterval 3 on TH_I.")
            await self.send_start_ranging(BLTCS_SPEC, peer_for_i, peer_for_r, periodic=True)
            self.step(10, "TH_I and TH_R each send a StartRangingResponse with a SessionID.",
                      expectation="Each SessionID is non-zero and, for that instance, has incremented over its "
                      "previous StartRangingResponse (uint8 wrap-around allowed).")
            await self.verify_start_responses(BLTCS_SPEC)
            self.step(11, "TH_I emits a RangingResult event once every 3 seconds; DUT obtains the events.",
                      expectation="DUT obtains one RangingResult per interval (~3 s cadence, +/- 3 s).")
            await self.verify_periodic_cadence(BLTCS_SPEC, result_handler)

        self.step(12, "Enable the BLERBC feature and disable other Proximity Ranging features on TH_I and TH_R.")
        self.mark_current_step_skipped()

        self.step(13, "Trigger DUT to read all attributes from TH_I and TH_R.",
                  expectation="DUT discovers TH_I and TH_R and obtains their attributes.")
        peer_for_i, peer_for_r = await self.read_pass_attributes(BLERBC_SPEC)
        async with self.ranging_result_subscription() as result_handler:
            self.step(14, "Trigger DUT to send StartRangingRequest to TH_I (scanning) and TH_R (beacon), with "
                      "StartTime 0, EndTime 30 and RangingInstanceInterval 3.",
                      expectation="DUT sends StartRangingRequest to TH_I with Role=BLEScanningRole and "
                      "PeerBLEDeviceID=TH_R's BLEDeviceID, and to TH_R with Role=BLEBeaconRole and "
                      "PeerBLEDeviceID=TH_I's BLEDeviceID; SessionKey and BLERBCSecurityMode common to both; "
                      "StartTime 0, EndTime 30, RangingInstanceInterval 3 on TH_I.")
            await self.send_start_ranging(BLERBC_SPEC, peer_for_i, peer_for_r, periodic=True)
            self.step(15, "TH_I and TH_R each send a StartRangingResponse with a SessionID.",
                      expectation="Each SessionID is non-zero and, for that instance, has incremented over its "
                      "previous StartRangingResponse (uint8 wrap-around allowed).")
            await self.verify_start_responses(BLERBC_SPEC)
            self.step(16, "TH_I emits a RangingResult event once every 3 seconds; DUT obtains the events.",
                      expectation="DUT obtains one RangingResult per interval (~3 s cadence, +/- 3 s).")
            await self.verify_periodic_cadence(BLERBC_SPEC, result_handler)


if __name__ == "__main__":
    default_matter_test_main()

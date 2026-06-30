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

# See https://github.com/project-chip/connectedhomeip/blob/master/docs/testing/python.md#defining-the-ci-test-arguments
# for details about the block below.
#
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --endpoint 0
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --PICS src/app/tests/suites/certification/ci-pics-values
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import logging
from typing import Any

from mobly import asserts

import matter.clusters as Clusters
from matter.interaction_model import Status
from matter.testing.decorators import async_test_body
from matter.testing.matter_asserts import is_valid_uint_value
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

log = logging.getLogger(__name__)


class TC_HSTAT_2_1(MatterBaseTest):
    def desc_TC_HSTAT_2_1(self) -> str:
        return "[TC-HSTAT-2.1] Primary functionality with DUT as Server

    def steps_TC_FAN_2_1(self):
        return [TestStep(1, "Commissioning already done.", is_commissioning=True),
                TestStep(2, "TH reads from the DUT the Mode attribute.",
                         "Verify that the DUT response contains a value between 0 and 4 inclusive."),
                TestStep(3,"TH reads from the DUT the SystemState attribute.",
                         "Verify that the DUT response contains a value between 0 and 4 inclusive."),
                TestStep(4, "TH reads from the DUT the MinSetpoint attribute.",
                            "Verify that the DUT response contains a value between 0 and 99 inclusive. " +
                            "Store the value as MinSetpointValue"),
                TestStep(5, "TH reads from the DUT the MaxSetpoint attribute.",
                            "Verify that the DUT response contains a value between MinSetpointValue and 100. " +
                            "Store the value as MaxSetpointValue."),
                TestStep(6, "TH reads from the DUT the Step attribute.",
                            "Store the value as StepValue. " +
                            "StepValue is between 1 and (MaxSetpointValue - MinSetpointValue) inclusive. " +
                            "StepValue is such that (MaxSetpointValue - MinSetpointValue) % value == 0."),
                TestStep(7, "Store the value as StepValue.",
                            "Store the value as SetpointValue. " +
                            "SetpointValue is between MinSetpointValue and MaxSetpointValue inclusive. " +
                            "SetpointValue is such that (SetpointValue - MinSetpointValue) % StepValue == 0."),
                TestStep(8, "Store the value as SetpointValue.",
                            "Verify that the DUT response contains a value between MinSetpointValue and MaxSetpointValue inclusive."),
                TestStep(9, "TH reads from the DUT the MistType attribute.",
                            "Verify that the DUT response contains a value or 0 or 1"),
                TestStep(10, "TH reads from the DUT the Continuous attribute.",
                             "Verify that the DUT response contains a value or True or False."),
                TestStep(11, "Verify that the DUT response contains a value or True or False",
                             "Verify that the DUT response contains a value or True or False."),
                TestStep(12, "TH reads from the DUT the Optimal attribute.",
                             "Verify that the DUT response contains a value or True or False"),
                TestStep(13, "TH writes to the DUT the Mode attribute with a value of Off",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(14, "TH writes to the DUT the Mode attribute with a value of Humidifier"
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(15, "TH reads from the DUT the Mode attribute.",
                             "Verify that the DUT response contains Humidifier."),
                TestStep(16, "TH reads from the DUT the SystemState attribute.",
                             "Verify that the DUT response contains Humidifying."),
                TestStep(17, "TH writes to the DUT the Mode attribute with a value of Dehumidifier",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(18, "TH reads from the DUT the Mode attribute.",
                             "Verify that the DUT response contains Dehumidifier."),
                TestStep(19, "TH reads from the DUT the SystemState attribute.",
                             "Verify that the DUT response contains Dehumidifying."),
                TestStep(20, "TH writes to the DUT the Mode attribute with a value of Auto",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(21, "TH writes to the DUT the Mode attribute with a value of FanOnly",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(22, "TH reads from the DUT the SystemState attribute.",
                             "Verify that the DUT response contains Fan."),
                TestStep(23, "TH writes to the DUT the Mode attribute with a value of Humidifier",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(24, "TH writes to the DUT the Mode attribute with a value of Dehumidifier",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(25, "TH writes to the DUT the UserSetpoint attribute with value MinSetpointValue."
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(26, "TH reads from the DUT the UserSetpoint attribute.",
                             "Verify that the DUT response contains MinSetpointValue."),
                TestStep(27, "TH writes to the DUT the UserSetpoint attribute with value MaxSetpointValue."
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(28, "TH reads from the DUT the UserSetpoint attribute.",
                             "Verify that the DUT response contains MaxSetpointValue."),
                TestStep(29, "TH writes to the DUT the UserSetpoint attribute with value MinSetpointValue + StepValue.",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(30, "TH reads from the DUT the UserSetpoint attribute.",
                             "Verify that the DUT response contains MinSetpointValue + StepValue."),
                TestStep(31, "TH reads from the DUT the Continuous attribute."
                             "Store the value as ContState"),
                TestStep(32, "TH writes to the DUT the Continuous attribute with a value of !ContState.",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(33, "TH reads from the DUT the Continuous attribute.",
                             "Verify that the DUT response contains !ContState."),
                TestStep(34, "TH writes to the DUT the Continuous attribute with a value of False.",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(35, "TH reads from the DUT the Sleep attribute.",
                             "Store the value as SleepState"),
                TestStep(36, "TH writes to the DUT the Sleep attribute with a value of !SleepState.",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(37, "TH reads from the DUT the Sleep attribute.",
                             "Verify that the DUT response contains !SleepState."),
                TestStep(38, "TH writes to the DUT the Sleep attribute with a value of False.",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(39, "TH reads from the DUT the Optimal attribute.",
                             "Store the value as OptState"),
                TestStep(40, "TH writes to the DUT the Optimal attribute with a value of !OptState.",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(41, "Store the value as OptState",
                             "Verify that the DUT response contains !OptState.")
                TestStep(42, "TH writes to the DUT the Optimal attribute with a value of False.",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(43, "TH writes to the DUT the UserSetpoint attribute with value MinSetpointValue + StepValue/2.",
                             "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
                TestStep(44, "TH writes to the DUT the UserSetpoint attribute with value MinSetpointValue - 1.",
                             "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
                TestStep(45, "TH writes to the DUT the UserSetpoint attribute with value MaxSetpointValue + 1.",
                             "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
                TestStep(46, "TH writes to the DUT the Mode attribute with a value of Humidifier",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(47, "TH writes to the DUT the MistType attribute with a value of MistWarm",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(48, "TH reads from the DUT the MistType attribute.",
                             "Verify that the DUT response contains MistWarm."),
                TestStep(49, "TH writes to the DUT the MistType attribute with a value of MistCold",
                             "Verify DUT responds w/ status SUCCESS(0x00)"),
                TestStep(50, "TH reads from the DUT the MistType attribute.",
                             "Verify that the DUT response contains a value of MistCold."),
                TestStep(51, "TH writes to the DUT the MistType attribute with a value of MistWarm.",
                             "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),
                TestStep(52, "TH writes to the DUT the MistType attribute with a value of MistCold.",
                             "Verify DUT responds w/ status CONSTRAINT_ERROR(0x87)"),

        ]



                

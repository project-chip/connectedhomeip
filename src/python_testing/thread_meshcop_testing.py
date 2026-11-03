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

from mobly import asserts

from matter import ChipDeviceCtrl, discovery
from matter.testing.matter_testing import MatterBaseTest, SetupParameters

THREAD_RENDEZVOUS_INFORMATION = 1 << 5


def _first_setup_payload_info(test: MatterBaseTest):
    setup_payloads = test.get_setup_payload_info()
    asserts.assert_true(setup_payloads,
                        "Thread MeshCoP provisional tests require --qr-code, --manual-code, or --discriminator/--passcode.")
    return setup_payloads[0]


def _long_discriminator_from_config_or_payload(test: MatterBaseTest) -> int:
    if test.matter_test_config.discriminators:
        return test.matter_test_config.discriminators[0]

    setup_payload = _first_setup_payload_info(test)
    asserts.assert_equal(setup_payload.filter_type, discovery.FilterType.LONG_DISCRIMINATOR,
                         "Long-discriminator MeshCoP test steps require --qr-code or --discriminator/--passcode.")
    return setup_payload.filter_value


def get_setup_code(test: MatterBaseTest, use_short_discriminator: bool) -> str:
    if not use_short_discriminator and test.matter_test_config.qr_code_content:
        return test.matter_test_config.qr_code_content[0]

    if use_short_discriminator and test.matter_test_config.manual_code:
        return test.matter_test_config.manual_code[0]

    if test.matter_test_config.setup_passcodes and test.matter_test_config.discriminators:
        setup_params = SetupParameters(passcode=test.matter_test_config.setup_passcodes[0],
                                       discriminator=test.matter_test_config.discriminators[0],
                                       capabilities=THREAD_RENDEZVOUS_INFORMATION)
        return setup_params.manual_code if use_short_discriminator else setup_params.qr_code

    setup_payload = _first_setup_payload_info(test)
    if not use_short_discriminator:
        asserts.assert_is_not_none(setup_payload.setup_code,
                                   "Long-discriminator MeshCoP test steps require --qr-code or --discriminator/--passcode.")
        return setup_payload.setup_code

    if setup_payload.filter_type == discovery.FilterType.SHORT_DISCRIMINATOR:
        manual_code_discriminator = setup_payload.filter_value << 8
    else:
        manual_code_discriminator = setup_payload.filter_value
    return test.default_controller.CreateManualCode(manual_code_discriminator, setup_payload.passcode)


def discriminator_from_config(test: MatterBaseTest, use_short_discriminator: bool) -> int:
    long_discriminator = _long_discriminator_from_config_or_payload(test)
    return (long_discriminator >> 8) & 0x0F if use_short_discriminator else long_discriminator


def verify_meshcop_config(test: MatterBaseTest) -> None:
    asserts.assert_is_not_none(test.matter_test_config.thread_operational_dataset,
                               "Thread MeshCoP provisional tests require --thread-dataset-hex.")
    asserts.assert_is_not_none(test.matter_test_config.thread_ba_host,
                               "Thread MeshCoP provisional tests require --thread-ba-host.")
    asserts.assert_is_not_none(test.matter_test_config.thread_ba_port,
                               "Thread MeshCoP provisional tests require --thread-ba-port.")


async def establish_pase_over_thread_meshcop(test: MatterBaseTest, setup_code: str) -> dict:
    verify_meshcop_config(test)

    commissioner: ChipDeviceCtrl.ChipDeviceController = test.default_controller
    commissioner.SetThreadOperationalDataset(test.matter_test_config.thread_operational_dataset)
    await commissioner.EstablishPASESessionThreadMeshcop(baAddr=test.matter_test_config.thread_ba_host,
                                                         setupCode=setup_code,
                                                         nodeId=test.matter_test_config.dut_node_ids[0],
                                                         baPort=test.matter_test_config.thread_ba_port)
    return commissioner.GetLastThreadMeshcopDiscoveryDiagnostic()


def assert_common_diagnostic_fields(diagnostic: dict, expected_discriminator: int, use_short_discriminator: bool) -> None:
    asserts.assert_true(diagnostic["valid"], "Thread MeshCoP discovery diagnostic was not captured.")
    asserts.assert_equal(diagnostic["requested_discriminator_type"],
                         "short" if use_short_discriminator else "long")
    asserts.assert_equal(diagnostic["requested_discriminator"], expected_discriminator)
    asserts.assert_true(diagnostic["steering_data_hex"], "Steering data must be present.")
    asserts.assert_greater(diagnostic["joiner_id"], 0, "Joiner/source identifier must be captured.")
    asserts.assert_greater(diagnostic["joiner_udp_port"], 0, "Joiner UDP port must be captured.")

    announcement = diagnostic["dns_announcement"]
    asserts.assert_true(announcement["thread_meshcop"], "DNS announcement must be from Thread MeshCoP discovery.")
    asserts.assert_greater(announcement["service_port"], 0, "DNS announcement must advertise a Matter joiner UDP port.")
    asserts.assert_true(0 <= announcement["long_discriminator"] <= 4095,
                        "DNS announcement must contain a valid commissionable node long discriminator.")

    if use_short_discriminator:
        actual_short_discriminator = (announcement["long_discriminator"] >> 8) & 0x0F
        asserts.assert_equal(actual_short_discriminator, expected_discriminator,
                             "DNS announcement long discriminator must match the requested short discriminator.")
    else:
        asserts.assert_equal(announcement["long_discriminator"], expected_discriminator,
                             "DNS announcement long discriminator must match the requested long discriminator.")

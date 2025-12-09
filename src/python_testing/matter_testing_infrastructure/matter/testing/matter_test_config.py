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
import pathlib
import typing
from dataclasses import dataclass, field
from datetime import timedelta
from typing import List, Optional

from matter.testing.defaults import TestingDefaults


@dataclass
class MatterTestConfig:
    storage_path: pathlib.Path = pathlib.Path(".")
    logs_path: pathlib.Path = pathlib.Path(".")
    paa_trust_store_path: Optional[pathlib.Path] = None
    ble_controller: Optional[int] = None
    commission_only: bool = False

    admin_vendor_id: int = TestingDefaults.ADMIN_VENDOR_ID
    case_admin_subject: Optional[int] = None
    global_test_params: dict = field(default_factory=dict)
    # List of explicit tests to run by name. If empty, all tests will run
    tests: List[str] = field(default_factory=list)
    timeout: typing.Union[int, None] = None
    endpoint: typing.Union[int, None] = 0
    app_pid: int = 0
    pipe_name: typing.Union[str, None] = None
    fail_on_skipped_tests: bool = False

    commissioning_method: Optional[str] = None
    in_test_commissioning_method: Optional[str] = None
    discriminators: List[int] = field(default_factory=list)
    setup_passcodes: List[int] = field(default_factory=list)
    commissionee_ip_address_just_for_testing: Optional[str] = None
    # By default, we start with maximized cert chains, as required for RR-1.1.
    # This allows cert tests to be run without re-commissioning for RR-1.1.
    maximize_cert_chains: bool = True

    # By default, let's set validity to 10 years
    certificate_validity_period = int(timedelta(days=10*365).total_seconds())

    qr_code_content: List[str] = field(default_factory=list)
    manual_code: List[str] = field(default_factory=list)

    wifi_ssid: Optional[str] = None
    wifi_passphrase: Optional[str] = None
    thread_operational_dataset: Optional[bytes] = None

    pics: dict[str, bool] = field(default_factory=dict)

    # Node ID for basic DUT
    dut_node_ids: List[int] = field(default_factory=list)
    # Node ID to use for controller/commissioner
    controller_node_id: int = TestingDefaults.CONTROLLER_NODE_ID
    # CAT Tags for default controller/commissioner
    # By default, we commission with CAT tags specified for RR-1.1
    # so the cert tests can be run without re-commissioning the device
    # for this one test. This can be overwritten from the command line
    controller_cat_tags: List[int] = field(default_factory=lambda: [0x0001_0001])

    # Fabric ID which to use
    fabric_id: int = 1

    # "Alpha" by default
    root_of_trust_index: int = TestingDefaults.TRUST_ROOT_INDEX

    # If this is set, we will reuse root of trust keys at that location
    chip_tool_credentials_path: Optional[pathlib.Path] = None

    trace_to: List[str] = field(default_factory=list)

    # Accepted Terms and Conditions if used
    tc_version_to_simulate: int = None
    tc_user_response_to_simulate: int = None
    # path to device attestation revocation set json file
    dac_revocation_set_path: Optional[pathlib.Path] = None

    legacy: bool = False

    # Restart flag file for rebooting the DUT during test runs
    restart_flag_file: Optional[pathlib.Path] = None

    # Debug mode to capture attribute dump at end of test modules
    debug: bool = False

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

"""
This module contains CommissionDeviceTest class designed to handle the commissioning process of Matter devices.
"""

from typing import List

from mobly import signals

from matter.testing.commissioning import CommissioningInfo, SetupPayloadInfo, commission_devices
from matter.testing.matter_testing import MatterBaseTest


class CommissionDeviceTest(MatterBaseTest):
    """Test class auto-injected at the start of test list to commission a device when requested"""

    def __init__(self, *args):
        super().__init__(*args)
        # This class is used to commission the device so is set to True
        self.is_commissioning = True

        # Use inherited matter_test_config property instead of manual extraction
        config = self.matter_test_config
        self.dut_node_ids: List[int] = config.dut_node_ids
        self.commissioning_info: CommissioningInfo = CommissioningInfo(
            commissionee_ip_address_just_for_testing=config.commissionee_ip_address_just_for_testing,
            commissioning_method=config.commissioning_method,
            thread_operational_dataset=config.thread_operational_dataset,
            wifi_passphrase=config.wifi_passphrase,
            wifi_ssid=config.wifi_ssid,
            tc_version_to_simulate=config.tc_version_to_simulate,
            tc_user_response_to_simulate=config.tc_user_response_to_simulate,
            thread_ba_host=config.thread_ba_host,
            thread_ba_port=config.thread_ba_port,
        )
        # Use inherited get_setup_payload_info method
        self.setup_payloads: List[SetupPayloadInfo] = self.get_setup_payload_info()

    def test_run_commissioning(self):
        """This method is the test called by mobly, which try to commission the device until is complete or raises an error.
        Raises:
            signals.TestAbortAll: Failed to commission node(s)
        """
        if not self.event_loop.run_until_complete(commission_devices(
            dev_ctrl=self.default_controller,
            dut_node_ids=self.dut_node_ids,
            setup_payloads=self.setup_payloads,
            commissioning_info=self.commissioning_info
        )):
            raise signals.TestAbortAll("Failed to commission node(s)")

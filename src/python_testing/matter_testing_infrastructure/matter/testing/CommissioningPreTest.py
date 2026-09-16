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


import logging
import os

from mobly import signals

from matter.testing.commissioning import CommissioningInfo, SetupPayloadInfo, commission_devices
from matter.testing.defaults import SNAPSHOT_COMMISSIONED_STATE
from matter.testing.matter_testing import MatterBaseTest

logger = logging.getLogger(__name__)


class CommissionDeviceTest(MatterBaseTest):
    """Test class auto-injected at the start of test list to commission a device when requested"""

    def __init__(self, *args):
        super().__init__(*args)
        # This class is used to commission the device so is set to True
        self.is_commissioning = True

        # Use inherited matter_test_config property instead of manual extraction
        config = self.matter_test_config
        self.dut_node_ids: list[int] = config.dut_node_ids
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
        self.setup_payloads: list[SetupPayloadInfo] = self.get_setup_payload_info()

    def _request_commissioned_state_snapshot(self):
        """Ask the test runner to snapshot the app's state now, before any test has touched the DUT.

        The runner restores that snapshot before later runs of the same app, so each test starts
        from a DUT that is commissioned to this fabric and otherwise at factory defaults.
        """
        restart_flag_file = self.get_restart_flag_file()
        if restart_flag_file is None:
            logger.info("No restart flag file, cannot request a snapshot of the commissioned state")
            return

        # Written atomically: the runner polls this file and treats a partial read as a restart request.
        temp_file = f"{restart_flag_file}.tmp"
        with open(temp_file, "w") as f:
            f.write(SNAPSHOT_COMMISSIONED_STATE)
        os.replace(temp_file, restart_flag_file)

        # The runner removes the file once the snapshot is taken, which keeps the tests from
        # touching the DUT before it is captured.
        self.event_loop.run_until_complete(
            self.wait_for_restart_flag_file_removal(restart_flag_file, "commissioned state snapshot"))
        logger.info("Commissioned state snapshot taken by the test runner")

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

        if self.matter_test_config.snapshot_commissioned_state:
            self._request_commissioned_state_snapshot()

        if self.matter_test_config.commission_only_re_open_window:
            for node_id, setup_payload in zip(self.dut_node_ids, self.setup_payloads):
                logger.info("Re-opening commissioning window on DUT")
                try:
                    params = self.default_controller.CommissioningWindowPasscode
                    self.event_loop.run_until_complete(
                        self.default_controller.OpenCommissioningWindow(
                            nodeId=node_id,
                            timeout=900,
                            iteration=1000,
                            discriminator=setup_payload.filter_value,
                            option=params.kTokenWithProvidedPin,
                            setupPinCode=setup_payload.passcode
                        )
                    )
                    logger.info("Commissioning window opened successfully.")
                except Exception as e:
                    logger.exception("Failed to open commissioning window")
                    raise signals.TestAbortAll(f"Failed to open commissioning window on node {node_id}: {str(e)}")

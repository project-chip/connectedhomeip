"""Matter device commissioning functionality."""

import logging

from chip.ChipDeviceCtrl import DiscoveryFilterType
from chip.exceptions import ChipStackError
from mobly import signals

from .matter_base_test import MatterBaseTest
from .models import SetupPayloadInfo


class CommissionDeviceTest(MatterBaseTest):
    """Test class auto-injected at the start of test list to commission a device when requested"""

    def __init__(self, *args):
        super().__init__(*args)
        self.is_commissioning = True

    def test_run_commissioning(self):
        if not self.event_loop.run_until_complete(self.commission_devices()):
            raise signals.TestAbortAll("Failed to commission node(s)")

    async def commission_devices(self) -> bool:
        conf = self.matter_test_config

        commissioned = []
        setup_payloads = self.get_setup_payload_info()
        for node_id, setup_payload in zip(conf.dut_node_ids, setup_payloads):
            logging.info(f"Starting commissioning for root index {conf.root_of_trust_index}, "
                         f"fabric ID 0x{conf.fabric_id:016X}, node ID 0x{node_id:016X}")
            logging.info(f"Commissioning method: {conf.commissioning_method}")
            commissioned.append(await self.commission_device(node_id, setup_payload))

        return all(commissioned)

    async def commission_device(self, node_id: int, info: SetupPayloadInfo) -> bool:
        dev_ctrl = self.default_controller
        conf = self.matter_test_config

        if conf.tc_version_to_simulate is not None and conf.tc_user_response_to_simulate is not None:
            logging.debug(
                f"Setting TC Acknowledgements to version {conf.tc_version_to_simulate} with user response {conf.tc_user_response_to_simulate}.")
            dev_ctrl.SetTCAcknowledgements(conf.tc_version_to_simulate, conf.tc_user_response_to_simulate)

        if conf.commissioning_method == "on-network":
            try:
                await dev_ctrl.CommissionOnNetwork(
                    nodeId=node_id,
                    setupPinCode=info.passcode,
                    filterType=info.filter_type,
                    filter=info.filter_value
                )
                return True
            except ChipStackError as e:
                logging.error("Commissioning failed: %s" % e)
                return False
        elif conf.commissioning_method == "ble-wifi":
            try:
                await dev_ctrl.CommissionWiFi(
                    info.filter_value,
                    info.passcode,
                    node_id,
                    conf.wifi_ssid,
                    conf.wifi_passphrase,
                    isShortDiscriminator=(info.filter_type == DiscoveryFilterType.SHORT_DISCRIMINATOR)
                )
                return True
            except ChipStackError as e:
                logging.error("Commissioning failed: %s" % e)
                return False
        elif conf.commissioning_method == "ble-thread":
            try:
                await dev_ctrl.CommissionThread(
                    info.filter_value,
                    info.passcode,
                    node_id,
                    conf.thread_operational_dataset,
                    isShortDiscriminator=(info.filter_type == DiscoveryFilterType.SHORT_DISCRIMINATOR)
                )
                return True
            except ChipStackError as e:
                logging.error("Commissioning failed: %s" % e)
                return False
        elif conf.commissioning_method == "on-network-ip":
            try:
                logging.warning("==== USING A DIRECT IP COMMISSIONING METHOD NOT SUPPORTED IN THE LONG TERM ====")
                await dev_ctrl.CommissionIP(
                    ipaddr=conf.commissionee_ip_address_just_for_testing,
                    setupPinCode=info.passcode,
                    nodeid=node_id,
                )
                return True
            except ChipStackError as e:
                logging.error("Commissioning failed: %s" % e)
                return False
        else:
            raise ValueError("Invalid commissioning method %s!" % conf.commissioning_method)

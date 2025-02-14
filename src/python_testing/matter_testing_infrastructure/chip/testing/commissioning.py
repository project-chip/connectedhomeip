#
#    Copyright (c) 2022-2025 Project CHIP Authors
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


import logging
from dataclasses import dataclass
from typing import List, Optional

import chip.logging
import chip.native
from chip import ChipDeviceCtrl  # Needed before chip.FabricAdmin
from chip import discovery
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.exceptions import ChipStackError

# isort: on


@dataclass
class SetupPayloadInfo:
    filter_type: discovery.FilterType = discovery.FilterType.LONG_DISCRIMINATOR
    filter_value: int = 0
    passcode: int = 0


@dataclass
class CommissioningInfo:
    commissionee_ip_address_just_for_testing: Optional[str] = None
    commissioning_method: Optional[str] = None
    thread_operational_dataset: Optional[str] = None
    wifi_passphrase: Optional[str] = None
    wifi_ssid: Optional[str] = None
    # Accepted Terms and Conditions if used
    tc_version_to_simulate: int = None
    tc_user_response_to_simulate: int = None


@dataclass
class CustomCommissioningParameters:
    commissioningParameters: CommissioningParameters
    randomDiscriminator: int


async def commission_device(
    dev_ctrl: ChipDeviceCtrl.ChipDeviceController, node_id: int, info: SetupPayloadInfo, commissioning_info: CommissioningInfo
) -> bool:
    if commissioning_info.tc_version_to_simulate is not None and commissioning_info.tc_user_response_to_simulate is not None:
        logging.debug(
            f"Setting TC Acknowledgements to version {commissioning_info.tc_version_to_simulate} with user response {commissioning_info.tc_user_response_to_simulate}."
        )
        dev_ctrl.SetTCAcknowledgements(commissioning_info.tc_version_to_simulate, commissioning_info.tc_user_response_to_simulate)

    if commissioning_info.commissioning_method == "on-network":
        try:
            await dev_ctrl.CommissionOnNetwork(
                nodeId=node_id, setupPinCode=info.passcode, filterType=info.filter_type, filter=info.filter_value
            )
            return True
        except ChipStackError as e:
            logging.error("Commissioning failed: %s" % e)
            return False
    elif commissioning_info.commissioning_method == "ble-wifi":
        try:
            await dev_ctrl.CommissionWiFi(
                info.filter_value,
                info.passcode,
                node_id,
                commissioning_info.wifi_ssid,
                commissioning_info.wifi_passphrase,
                isShortDiscriminator=(info.filter_type == DiscoveryFilterType.SHORT_DISCRIMINATOR),
            )
            return True
        except ChipStackError as e:
            logging.error("Commissioning failed: %s" % e)
            return False
    elif commissioning_info.commissioning_method == "ble-thread":
        try:
            await dev_ctrl.CommissionThread(
                info.filter_value,
                info.passcode,
                node_id,
                commissioning_info.thread_operational_dataset,
                isShortDiscriminator=(info.filter_type == DiscoveryFilterType.SHORT_DISCRIMINATOR),
            )
            return True
        except ChipStackError as e:
            logging.error("Commissioning failed: %s" % e)
            return False
    elif commissioning_info.commissioning_method == "on-network-ip":
        try:
            logging.warning("==== USING A DIRECT IP COMMISSIONING METHOD NOT SUPPORTED IN THE LONG TERM ====")
            await dev_ctrl.CommissionIP(
                ipaddr=commissioning_info.commissionee_ip_address_just_for_testing,
                setupPinCode=info.passcode,
                nodeid=node_id,
            )
            return True
        except ChipStackError as e:
            logging.error("Commissioning failed: %s" % e)
            return False
    else:
        raise ValueError("Invalid commissioning method %s!" % commissioning_info.commissioning_method)


async def commission_devices(
    dev_ctrl: ChipDeviceCtrl.ChipDeviceController,
    dut_node_ids: List[int],
    setup_payloads: List[SetupPayloadInfo],
    commissioning_info: CommissioningInfo,
) -> bool:
    commissioned = []
    for node_id, setup_payload in zip(dut_node_ids, setup_payloads):
        logging.info(f"Commissioning method: {commissioning_info.commissioning_method}")
        commissioned.append(await commission_device(dev_ctrl, node_id, setup_payload, commissioning_info))

    return all(commissioned)

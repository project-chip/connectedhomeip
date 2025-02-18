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

"""
This module contains classes and functions designed to handle the commissioning process of Matter devices.
"""

import logging
from dataclasses import dataclass
from typing import List, Optional

from chip import ChipDeviceCtrl, discovery
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.clusters import ClusterObjects as ClusterObjects
from chip.exceptions import ChipStackError

logger = logging.getLogger("matter.python_testing")
logger.setLevel(logging.INFO)

DiscoveryFilterType = ChipDeviceCtrl.DiscoveryFilterType


@dataclass
class SetupPayloadInfo:
    """
    Represents information required to set up a payload during commissioning.

    Attributes:
        filter_type (discovery.FilterType): The type of filter used for discrimination. Default is `FilterType.LONG_DISCRIMINATOR`.
        filter_value (int): The value associated with the filter type. Default is `0`.
        passcode (int): A unique code or password required for setup. Default is `0`.
    """
    filter_type: discovery.FilterType = discovery.FilterType.LONG_DISCRIMINATOR
    filter_value: int = 0
    passcode: int = 0


@dataclass
class CommissioningInfo:
    """
    Represents the information required for commissioning a device.

    Attributes:
        commissionee_ip_address_just_for_testing (Optional[str]):
            The IP address of the commissionee used only for testing purposes.

        commissioning_method (Optional[str]):
            The method by which the device is being commissioned.

        thread_operational_dataset (Optional[str]):
            The Thread operational dataset if applicable during commissioning.

        wifi_passphrase (Optional[str]):
            The passphrase to connect to a Wi-Fi network, if required.

        wifi_ssid (Optional[str]):
            The name of the Wi-Fi network to which the device should connect.

        tc_version_to_simulate (Optional[int]):
            The version of the Terms and Conditions to simulate during testing.
            This is used when accepting terms and conditions in a simulated environment.

        tc_user_response_to_simulate (Optional[int]):
            The user response to simulate for the Terms and Conditions, if applicable.
    """
    commissionee_ip_address_just_for_testing: Optional[str] = None
    commissioning_method: Optional[str] = None
    thread_operational_dataset: Optional[str] = None
    wifi_passphrase: Optional[str] = None
    wifi_ssid: Optional[str] = None
    tc_version_to_simulate: Optional[int] = None
    tc_user_response_to_simulate: Optional[int] = None


@dataclass
class CustomCommissioningParameters:
    """
    A custom data class that encapsulates commissioning parameters with an additional random discriminator.

    Attributes:
        commissioningParameters (CommissioningParameters): The underlying commissioning parameters.
        randomDiscriminator (int): A randomly generated value used to uniquely identify or distinguish instances during commissioning processes.
    """
    commissioningParameters: CommissioningParameters
    randomDiscriminator: int


async def commission_device(
    dev_ctrl: ChipDeviceCtrl.ChipDeviceController, node_id: int, info: SetupPayloadInfo, commissioning_info: CommissioningInfo
) -> bool:
    """
    Starts the commissioning process of a chip device.

    This function handles different commissioning methods based on the specified method.
    It supports various commissioning techniques such as "on-network", "ble-wifi", and "ble-thread".

    Parameters:
        dev_ctrl: The chip device controller instance.
        node_id: Unique identifier for the chip node.
        info: Contains setup information including passcode, filter_type, and filter_value.
        commissioning_info: Specifies the type of commissioning method to use.

    Returns:
        True if the commissioning process completes successfully. False otherwise,
        except in case of an error which logs the exception details.

    Note:
        The "on-network-ip" method is deprecated as it's not supported in long-term
        environments.
    """

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
    """
    Attempt to commission all specified device nodes with their respective setup payloads.

    Args:
        dev_ctrl: The chip device controller being used.
        dut_node_ids: List of node IDs that need to be commissioned.
        setup_payloads: List of SetupPayloadInfo objects containing configuration data
            for each node to be set up and commissioned.
        commissioning_info: Information about the commissioning process, including method,
            parameters, etc.

    Returns:
        bool: True if all devices were successfully commissioned; False otherwise.
    """
    commissioned = []
    for node_id, setup_payload in zip(dut_node_ids, setup_payloads):
        logging.info(f"Commissioning method: {commissioning_info.commissioning_method}")
        commissioned.append(await commission_device(dev_ctrl, node_id, setup_payload, commissioning_info))

    return all(commissioned)

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
from typing import Any, List, Optional

import chip.testing.global_stash as global_stash
from chip import ChipDeviceCtrl, discovery
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.exceptions import ChipStackError
from chip.setup_payload import SetupPayload
from mobly import asserts, base_test, signals

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

        thread_operational_dataset (Optional[bytes]):
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
    thread_operational_dataset: Optional[bytes] = None
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


class PairingStatus:
    """
    This class is used to store the pairing status of a commissioning process with a device.
    if the commissioning Process is unsuccessful then we pass the exception to the class which sets the PairingStatus
    to False. If we do not pass any exception when creating the Pairing status then the commissioning status
    is set to true to indicate that the commissioning process has succeeded.
    """

    def __init__(self, exception: Optional[Exception] = None):
        self.exception = exception

    def __bool__(self):
        return self.exception is None

    def __str__(self):
        return str(self.exception) if self.exception else "Pairing Successful"


async def commission_device(
    dev_ctrl: ChipDeviceCtrl.ChipDeviceController, node_id: int, info: SetupPayloadInfo, commissioning_info: CommissioningInfo
) -> PairingStatus:
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
        PairingStatus object which can evaluated in conditional statements
        if the commissioning process completes successfully PairingStatus is evaluated to True else False along
        with storing the reason for pairing failure by storing the exception raised during commissioning process.

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
            return PairingStatus()
        except ChipStackError as e:  # chipstack-ok: Can not use 'with' because we handle and return the exception, not assert it
            logging.error("Commissioning failed: %s" % e)
            return PairingStatus(exception=e)
    elif commissioning_info.commissioning_method == "ble-wifi":
        try:
            asserts.assert_is_not_none(commissioning_info.wifi_ssid, "WiFi SSID must be provided for ble-wifi commissioning")
            asserts.assert_is_not_none(commissioning_info.wifi_passphrase,
                                       "WiFi Passphrase must be provided for ble-wifi commissioning")
            # Type assertions to help mypy understand these are not None after the asserts
            assert commissioning_info.wifi_ssid is not None
            assert commissioning_info.wifi_passphrase is not None
            await dev_ctrl.CommissionWiFi(
                info.filter_value,
                info.passcode,
                node_id,
                commissioning_info.wifi_ssid,
                commissioning_info.wifi_passphrase,
                isShortDiscriminator=(info.filter_type == DiscoveryFilterType.SHORT_DISCRIMINATOR),
            )
            return PairingStatus()
        except ChipStackError as e:  # chipstack-ok: Can not use 'with' because we handle and return the exception, not assert it
            logging.error("Commissioning failed: %s" % e)
            return PairingStatus(exception=e)
    elif commissioning_info.commissioning_method == "ble-thread":
        try:
            asserts.assert_is_not_none(commissioning_info.thread_operational_dataset,
                                       "Thread dataset must be provided for ble-thread commissioning")
            # Type assertion to help mypy understand this is not None after the assert
            assert commissioning_info.thread_operational_dataset is not None
            await dev_ctrl.CommissionThread(
                info.filter_value,
                info.passcode,
                node_id,
                commissioning_info.thread_operational_dataset,
                isShortDiscriminator=(info.filter_type == DiscoveryFilterType.SHORT_DISCRIMINATOR),
            )
            return PairingStatus()
        except ChipStackError as e:  # chipstack-ok: Can not use 'with' because we handle and return the exception, not assert it
            logging.error("Commissioning failed: %s" % e)
            return PairingStatus(exception=e)
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


def get_setup_payload_info_config(matter_test_config: Any) -> List[SetupPayloadInfo]:
    """
    Get and builds the payload info provided in the execution.

    Args:
        matter_test_config: Matter test configuration object

    Returns:
         List[SetupPayloadInfo]: List of Payload used by the test case
    """
    setup_payloads = []
    for qr_code in matter_test_config.qr_code_content:
        try:
            setup_payloads.append(SetupPayload().ParseQrCode(qr_code))
        except ChipStackError:  # chipstack-ok: This disables ChipStackError linter check. Can not use 'with' because it is not expected to fail
            asserts.fail(f"QR code '{qr_code} failed to parse properly as a Matter setup code.")

    for manual_code in matter_test_config.manual_code:
        try:
            setup_payloads.append(SetupPayload().ParseManualPairingCode(manual_code))
        except ChipStackError:  # chipstack-ok: This disables ChipStackError linter check. Can not use 'with' because it is not expected to fail
            asserts.fail(
                f"Manual code code '{manual_code}' failed to parse properly as a Matter setup code. Check that all digits are correct and length is 11 or 21 characters.")

    infos = []
    for setup_payload in setup_payloads:
        info = SetupPayloadInfo()
        info.passcode = setup_payload.setup_passcode
        if setup_payload.short_discriminator is not None:
            info.filter_type = discovery.FilterType.SHORT_DISCRIMINATOR
            info.filter_value = setup_payload.short_discriminator
        else:
            info.filter_type = discovery.FilterType.LONG_DISCRIMINATOR
            info.filter_value = setup_payload.long_discriminator
        infos.append(info)

    num_passcodes = 0 if matter_test_config.setup_passcodes is None else len(matter_test_config.setup_passcodes)
    num_discriminators = 0 if matter_test_config.discriminators is None else len(matter_test_config.discriminators)
    asserts.assert_equal(num_passcodes, num_discriminators, "Must have same number of discriminators as passcodes")
    if matter_test_config.discriminators:
        for idx, discriminator in enumerate(matter_test_config.discriminators):
            info = SetupPayloadInfo()
            info.passcode = matter_test_config.setup_passcodes[idx]
            info.filter_type = DiscoveryFilterType.LONG_DISCRIMINATOR
            info.filter_value = discriminator
            infos.append(info)

    return infos


class CommissionDeviceTest(base_test.BaseTestClass):
    """Test class auto-injected at the start of test list to commission a device when requested"""

    def __init__(self, *args):
        super().__init__(*args)
        # This class is used to commission the device so is set to True
        self.is_commissioning = True
        # Save the stashed values into attributes to avoid mobly conflic with ctypes when mobly performs copy().
        test_config = args[0]
        self.default_controller = test_config.user_params['default_controller']
        meta_config = test_config.user_params['meta_config']
        self.dut_node_ids: List[int] = meta_config['dut_node_ids']
        self.commissioning_info: CommissioningInfo = CommissioningInfo(
            commissionee_ip_address_just_for_testing=meta_config['commissionee_ip_address_just_for_testing'],
            commissioning_method=meta_config['commissioning_method'],
            thread_operational_dataset=meta_config['thread_operational_dataset'],
            wifi_passphrase=meta_config['wifi_passphrase'],
            wifi_ssid=meta_config['wifi_ssid'],
            tc_version_to_simulate=meta_config['tc_version_to_simulate'],
            tc_user_response_to_simulate=meta_config['tc_user_response_to_simulate'],
        )
        self.setup_payloads: List[SetupPayloadInfo] = get_setup_payload_info_config(
            global_stash.unstash_globally(test_config.user_params['matter_test_config']))

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

    # Default controller is used by commission_devices
    @property
    def default_controller(self) -> ChipDeviceCtrl.ChipDeviceController:
        return global_stash.unstash_globally(self._default_controller)

    @default_controller.setter
    def default_controller(self, tmp_default_controller):
        self._default_controller = tmp_default_controller

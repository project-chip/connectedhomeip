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

import asyncio
import contextlib
import logging
from dataclasses import dataclass
from typing import Any, List, Optional

from mobly import asserts, base_test, signals

import matter.testing.global_stash as global_stash
from matter import ChipDeviceCtrl, discovery
from matter.ChipDeviceCtrl import CommissioningParameters
from matter.exceptions import ChipStackError
from matter.setup_payload import SetupPayload

LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)

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
        LOGGER.debug(
            f"Setting TC Acknowledgements to version {commissioning_info.tc_version_to_simulate} with user response "
            f"{commissioning_info.tc_user_response_to_simulate}."
        )
        dev_ctrl.SetTCAcknowledgements(commissioning_info.tc_version_to_simulate, commissioning_info.tc_user_response_to_simulate)

    if commissioning_info.commissioning_method == "on-network":
        try:
            await dev_ctrl.CommissionOnNetwork(
                nodeId=node_id, setupPinCode=info.passcode, filterType=info.filter_type, filter=info.filter_value
            )
            return PairingStatus()
        except ChipStackError as e:  # chipstack-ok: Can not use 'with' because we handle and return the exception, not assert it
            LOGGER.error("Commissioning failed: %s" % e)
            return PairingStatus(exception=e)
    elif commissioning_info.commissioning_method == "ble-wifi":
        try:
            asserts.assert_is_not_none(commissioning_info.wifi_ssid, "WiFi SSID must be provided for ble-wifi commissioning")
            asserts.assert_is_not_none(commissioning_info.wifi_passphrase,
                                       "WiFi Passphrase must be provided for ble-wifi commissioning")
            # Type assertions to help mypy understand these are not None after the asserts
            assert commissioning_info.wifi_ssid is not None
            assert commissioning_info.wifi_passphrase is not None
            await dev_ctrl.CommissionBleWiFi(
                info.filter_value,
                info.passcode,
                node_id,
                commissioning_info.wifi_ssid,
                commissioning_info.wifi_passphrase,
                isShortDiscriminator=(info.filter_type == DiscoveryFilterType.SHORT_DISCRIMINATOR),
            )
            return PairingStatus()
        except ChipStackError as e:  # chipstack-ok: Can not use 'with' because we handle and return the exception, not assert it
            LOGGER.error("Commissioning failed: %s" % e)
            return PairingStatus(exception=e)
    elif commissioning_info.commissioning_method == "ble-thread":
        try:
            asserts.assert_is_not_none(commissioning_info.thread_operational_dataset,
                                       "Thread dataset must be provided for ble-thread commissioning")
            # Type assertion to help mypy understand this is not None after the assert
            assert commissioning_info.thread_operational_dataset is not None
            await dev_ctrl.CommissionBleThread(
                info.filter_value,
                info.passcode,
                node_id,
                commissioning_info.thread_operational_dataset,
                isShortDiscriminator=(info.filter_type == DiscoveryFilterType.SHORT_DISCRIMINATOR),
            )
            return PairingStatus()
        except ChipStackError as e:  # chipstack-ok: Can not use 'with' because we handle and return the exception, not assert it
            LOGGER.error("Commissioning failed: %s" % e)
            return PairingStatus(exception=e)
    elif commissioning_info.commissioning_method == "nfc-thread":
        try:
            asserts.assert_is_not_none(commissioning_info.thread_operational_dataset,
                                       "Thread dataset must be provided for nfc-thread commissioning")
            # Type assertion to help mypy understand this is not None after the assert
            assert commissioning_info.thread_operational_dataset is not None
            await dev_ctrl.CommissionNfcThread(
                info.filter_value,
                info.passcode,
                node_id,
                commissioning_info.thread_operational_dataset,
            )
            return PairingStatus()
        except ChipStackError as e:  # chipstack-ok: Can not use 'with' because we handle and return the exception, not assert it
            LOGGER.error("Commissioning failed: %s" % e)
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
        LOGGER.info(f"Commissioning method: {commissioning_info.commissioning_method}")
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

    manual_code_equivalents = [(s.long_discriminator >> 8, s.setup_passcode) for s in setup_payloads]
    for manual_code in matter_test_config.manual_code:
        try:
            # Remove any duplicate codes - where the discriminator and passcode match a previously added QR code.
            # This lets testers pass in the QR and equivalent manual code in order to run
            # the DD tests with a single set of parameters
            temp_payload = SetupPayload().ParseManualPairingCode(manual_code)
            if (temp_payload.short_discriminator, temp_payload.setup_passcode) in manual_code_equivalents:
                continue
            setup_payloads.append(temp_payload)
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


@dataclass
class SetupParameters:
    """
    Specifies configuration parameters for commissioning.

    Args:
        passcode (int): The setup passcode of the device.
        vendor_id (Optional[int]): Identification number specific tothe vendor.
        product_id (Optional[int]): Identification number specific to the product.
        discriminator (Optional[int]): The long discriminator for the DNS-SD advertisement. Valid range: 0-4095.
        custom_flow (Optional[int]): The custom flow type.
        capabilities (Optional[int]): Device capabilities.
        version (Optional[int]): Version number.

    """
    passcode: int
    vendor_id: int = 0xFFF1
    product_id: int = 0x8001
    discriminator: int = 3840
    custom_flow: int = 0
    capabilities: int = 0b0100
    version: int = 0

    @property
    def qr_code(self):
        return SetupPayload().GenerateQrCode(self.passcode, self.vendor_id, self.product_id, self.discriminator,
                                             self.custom_flow, self.capabilities, self.version)

    @property
    def manual_code(self):
        return SetupPayload().GenerateManualPairingCode(self.passcode, self.vendor_id, self.product_id, self.discriminator,
                                                        self.custom_flow, self.capabilities, self.version)


# Commissioning Status Detection Functions

# Default timeout for DNS-SD discovery (in seconds)
# Short timeout since DNS-SD is fast - device should respond quickly if operational
DNSSD_DISCOVERY_TIMEOUT_SEC = 3


async def _is_device_operational_via_dnssd(
    dev_ctrl: ChipDeviceCtrl.ChipDeviceController,
    node_id: int,
    discovery_timeout_sec: float = DNSSD_DISCOVERY_TIMEOUT_SEC
) -> bool:
    """
    Check if a device is advertising as operational on this fabric via DNS-SD.

    This is a fast check that avoids the long CASE timeout when a device is not
    commissioned. Devices advertise operational services on _matter._tcp.local.
    with instance name format: {compressed_fabric_id}-{node_id}

    Args:
        dev_ctrl: The chip device controller instance (used to get compressed fabric ID)
        node_id: Node ID of the device to check
        discovery_timeout_sec: Timeout for DNS-SD discovery (default 3 seconds)

    Returns:
        True if device is advertising as operational on this fabric, False otherwise
    """
    try:
        from mdns_discovery.mdns_discovery import MdnsDiscovery

        # Build expected instance name for this fabric+node
        compressed_fabric_id = dev_ctrl.GetCompressedFabricId()
        expected_instance_name = f'{compressed_fabric_id:016X}-{node_id:016X}'

        LOGGER.info(f"Checking DNS-SD for operational service: {expected_instance_name}")

        # Discover operational services
        mdns = MdnsDiscovery()
        services = await mdns.get_operational_services(
            discovery_timeout_sec=discovery_timeout_sec,
            log_output=False
        )

        # Check if our expected instance is in the discovered services
        for service in services:
            if service.instance_name == expected_instance_name:
                LOGGER.info(f"Device {node_id} found operational on fabric {compressed_fabric_id:016X} via DNS-SD")
                return True

        LOGGER.info(f"Device {node_id} not found operational on fabric {compressed_fabric_id:016X} via DNS-SD")
        return False

    except ImportError:
        LOGGER.warning("mdns_discovery module not available, skipping DNS-SD check")
        return False
    except Exception as e:
        LOGGER.warning(f"DNS-SD check failed, will fall back to connection attempt: {e}")
        return False


async def _establish_pase_or_case_session(
    dev_ctrl: ChipDeviceCtrl.ChipDeviceController,
    node_id: int,
    pase_params: Optional[dict] = None
) -> None:
    """
    Establish a session to the device by trying PASE and CASE in parallel.

    This is used as a fallback when DNS-SD check doesn't find the device operational.
    The device might be:
    - Not commissioned (PASE will succeed if pase_params provided)
    - Commissioned but DNS-SD failed for some reason (CASE will succeed)

    Whichever connection succeeds first is used; the other is cancelled.

    Args:
        dev_ctrl: The chip device controller instance
        node_id: Node ID for the session
        pase_params: Optional parameters for PASE establishment.
                    If not provided, only CASE will be attempted.

    Raises:
        ValueError: If no pase_params provided (can't attempt PASE)
        Exception: If both connection attempts fail
    """
    task_list = []

    # Add PASE task if we have parameters
    if pase_params is not None:
        setup_code = pase_params.get('setup_code')
        if not setup_code:
            passcode = pase_params.get('passcode')
            discriminator = pase_params.get('discriminator')
            if passcode is not None and discriminator is not None:
                setup_code = dev_ctrl.CreateManualCode(discriminator, passcode)

        if setup_code:
            LOGGER.info(f"Creating PASE task for node {node_id}")
            pase_future = dev_ctrl.FindOrEstablishPASESession(setup_code, node_id)
            task_list.append(asyncio.create_task(pase_future, name="pase"))

    # Always add CASE task (allowPASE=False to force CASE)
    LOGGER.info(f"Creating CASE task for node {node_id}")
    case_future = dev_ctrl.GetConnectedDevice(nodeId=node_id, allowPASE=False)
    task_list.append(asyncio.create_task(case_future, name="case"))

    if not task_list:
        raise ValueError(
            f"Cannot establish session to node {node_id}: no PASE params provided and CASE task creation failed"
        )

    LOGGER.info(f"Attempting parallel PASE/CASE connection to node {node_id}")

    # Wait for first successful completion
    done, pending = await asyncio.wait(task_list, return_when=asyncio.FIRST_COMPLETED)

    # Check if the completed task succeeded or raised an exception
    completed_task = done.pop()
    completed_name = completed_task.get_name()

    try:
        # This will raise if the task failed
        completed_task.result()
        LOGGER.info(f"Successfully established {completed_name.upper()} session to node {node_id}")
    except Exception as e:
        # First task failed, wait for the other if there is one
        if pending:
            LOGGER.info(f"{completed_name.upper()} failed ({e}), waiting for other connection attempt")
            done2, pending2 = await asyncio.wait(pending, return_when=asyncio.FIRST_COMPLETED)
            completed_task2 = done2.pop()
            completed_name2 = completed_task2.get_name()
            try:
                completed_task2.result()
                LOGGER.info(f"Successfully established {completed_name2.upper()} session to node {node_id}")
                # Cancel any remaining
                for task in pending2:
                    task.cancel()
                    with contextlib.suppress(asyncio.CancelledError):
                        await task
                return
            except Exception as e2:
                raise RuntimeError(
                    f"Both PASE and CASE connection attempts failed for node {node_id}. "
                    f"PASE error: {e}, CASE error: {e2}"
                ) from e2
        else:
            raise

    # Cancel pending tasks
    for task in pending:
        task.cancel()
        with contextlib.suppress(asyncio.CancelledError):
            await task


async def is_commissioned(
    dev_ctrl: ChipDeviceCtrl.ChipDeviceController,
    node_id: int,
    endpoint: int = 0,
    pase_params: Optional[dict] = None
) -> bool:
    """
    Check if a device has any commissioned fabrics.

    Uses DNS-SD to check if the device is operational on this fabric, avoiding long timeouts.
    Then reads the TrustedRootCertificates attribute.

    Args:
        dev_ctrl: The chip device controller instance
        node_id: Node ID of the device to check
        endpoint: Endpoint to query (default 0)
        pase_params: Optional parameters for establishing PASE if device is not commissioned.
                    Format: {'setup_code': str, 'discriminator': int, 'passcode': int}

    Returns:
        True if device has at least one commissioned fabric, False otherwise.

    Raises:
        ChipStackError: If unable to read the TrustedRootCertificates attribute
    """
    try:
        # Fast DNS-SD check to determine if device is operational on this fabric
        # If device is advertising as operational, it's definitely commissioned - no need to read attributes
        is_operational = await _is_device_operational_via_dnssd(dev_ctrl, node_id)

        if is_operational:
            # Device is operational on this fabric - it's commissioned, no need for CASE read
            LOGGER.info(f"Device {node_id} is operational via DNS-SD - confirmed commissioned")
            return True

        # Device not operational on this fabric via DNS-SD - could be:
        # 1. Not commissioned at all (factory fresh) - PASE will work
        # 2. Commissioned to other fabrics but not this one - need to read via PASE
        # 3. Commissioned to this fabric but DNS-SD failed - CASE will work
        # Try both PASE and CASE in parallel for fastest response

        if pase_params is None:
            # No PASE params and not operational via DNS-SD - can't safely proceed
            raise ValueError(
                f"Device {node_id} is not operational on this fabric and no PASE parameters provided. "
                "Cannot check commissioning status without risking long connection timeout."
            )

        # Import locally to avoid potential circular dependencies
        import matter.clusters as Clusters

        # Try both PASE and CASE in parallel - use whichever succeeds first
        LOGGER.info(f"Device {node_id} not found via DNS-SD, trying parallel PASE/CASE connection")
        await _establish_pase_or_case_session(dev_ctrl, node_id, pase_params)

        result = await dev_ctrl.ReadAttribute(
            nodeId=node_id,
            attributes=[(endpoint, Clusters.OperationalCredentials.Attributes.TrustedRootCertificates)]
        )

        # Extract the trusted root certificates list
        root_certs = result[endpoint][Clusters.OperationalCredentials][
            Clusters.OperationalCredentials.Attributes.TrustedRootCertificates
        ]

        # Device is commissioned if it has any root certificates
        return len(root_certs) > 0

    except Exception as e:
        LOGGER.error(f"Failed to check commissioning status for node {node_id}: {e}")
        raise


async def get_commissioned_fabric_count(
    dev_ctrl: ChipDeviceCtrl.ChipDeviceController,
    node_id: int,
    endpoint: int = 0,
    pase_params: Optional[dict] = None
) -> int:
    """
    Get the number of commissioned fabrics on a device.

    Uses DNS-SD to check if the device is operational on this fabric, avoiding long timeouts.
    Then reads the TrustedRootCertificates attribute and returns the count.

    Args:
        dev_ctrl: The chip device controller instance
        node_id: Node ID of the device to check
        endpoint: Endpoint to query (default 0)
        pase_params: Optional parameters for establishing PASE if device is not commissioned.
                    Format: {'setup_code': str, 'discriminator': int, 'passcode': int}

    Returns:
        Number of commissioned fabrics (count of trusted root certificates).
        Returns 0 if device is factory fresh.

    Raises:
        ChipStackError: If unable to read the TrustedRootCertificates attribute
    """
    try:
        # Import locally to avoid potential circular dependencies
        import matter.clusters as Clusters

        # Fast DNS-SD check to determine if device is operational on this fabric
        # This avoids the long CASE timeout if device is not commissioned
        is_operational = await _is_device_operational_via_dnssd(dev_ctrl, node_id)

        if is_operational:
            # Device is operational on this fabric - use CASE
            LOGGER.info(f"Device {node_id} is operational via DNS-SD, using CASE connection")
            result = await dev_ctrl.ReadAttribute(
                nodeId=node_id,
                attributes=[(endpoint, Clusters.OperationalCredentials.Attributes.TrustedRootCertificates)]
            )
        elif pase_params is not None:
            # Device not operational on this fabric via DNS-SD - could be:
            # 1. Not commissioned at all (factory fresh) - PASE will work
            # 2. Commissioned but DNS-SD failed - CASE will work
            # Try both in parallel for fastest response
            LOGGER.info(f"Device {node_id} not found via DNS-SD, trying parallel PASE/CASE connection")
            await _establish_pase_or_case_session(dev_ctrl, node_id, pase_params)
            result = await dev_ctrl.ReadAttribute(
                nodeId=node_id,
                attributes=[(endpoint, Clusters.OperationalCredentials.Attributes.TrustedRootCertificates)]
            )
        else:
            # No PASE params and not operational - can't proceed without risking long timeout
            raise ValueError(
                f"Device {node_id} is not operational on this fabric and no PASE parameters provided. "
                "Cannot get fabric count without risking long connection timeout."
            )

        # Extract the trusted root certificates list
        root_certs = result[endpoint][Clusters.OperationalCredentials][
            Clusters.OperationalCredentials.Attributes.TrustedRootCertificates
        ]

        # Return the count
        return len(root_certs)

    except Exception as e:
        LOGGER.error(f"Failed to get fabric count for node {node_id}: {e}")
        raise

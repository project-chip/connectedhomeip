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

import asyncio
import logging
import platform
import shutil
import subprocess
from typing import Optional

import chip.clusters as Clusters

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)

cnet = Clusters.NetworkCommissioning

MAX_RETRIES = 5
TIMEOUT = 900
TIMED_REQUEST_TIMEOUT_MS = 5000
WIFI_WAIT_SECONDS = 5


async def connect_wifi_linux(ssid: str, password: str) -> Optional[subprocess.CompletedProcess]:
    """
    Connects to a WiFi network using nmcli on Linux systems.

    Args:
        ssid (str): The SSID of the WiFi network to connect to.
        password (str): The password for the WiFi network.

    Returns:
        subprocess.CompletedProcess or None: Result of the connection attempt.
    """
    if isinstance(ssid, bytes):
        ssid = ssid.decode()
    if isinstance(password, bytes):
        password = password.decode()

    if not shutil.which("nmcli"):
        logger.error("'nmcli' is not installed. Install with: sudo apt install network-manager")
        return None

    result = None
    try:
        # Activates Wi-Fi in case it is deactivated
        subprocess.run(["nmcli", "radio", "wifi", "on"], check=False)

        # Detects the active Wi-Fi interface
        interface_result = subprocess.run(
            ["nmcli", "-t", "-f", "DEVICE,TYPE,STATE", "device"],
            capture_output=True, text=True
        )
        interface = None
        for line in interface_result.stdout.strip().splitlines():
            parts = line.split(":")
            if len(parts) >= 3 and parts[1] == "wifi":
                interface = parts[0]
                if parts[2] == "connected":
                    # Disconnecting interface
                    subprocess.run(["nmcli", "device", "disconnect", interface], check=False)
                break

        # Let's try to connect
        result = subprocess.run(
            ["nmcli", "d", "wifi", "connect", ssid, "password", password],
            capture_output=True, text=True
        )
        # wait the connection to be ready
        await asyncio.sleep(WIFI_WAIT_SECONDS)

        # Let's verify it is connected
        check = subprocess.run(
            ["nmcli", "-t", "-f", "ACTIVE,SSID", "dev", "wifi"],
            capture_output=True, text=True
        )
        if any(line.startswith("yes:" + ssid) for line in check.stdout.splitlines()):
            logger.info(f" --- connect_wifi_linux: Successfully connected to '{ssid}'")
        else:
            logger.info(f" --- connect_wifi_linux: Could not confirm WiFi connection to '{ssid}'")

    except subprocess.CalledProcessError as e:
        logger.error(f" --- connect_wifi_linux: nmcli command failed: {e.stderr.strip()}")
    except Exception as e:
        logger.error(f" --- connect_wifi_linux: Unexpected exception when trying to connect to '{ssid}': {e}")
    finally:
        return result


async def connect_wifi_mac(ssid: str, password: str) -> Optional[subprocess.CompletedProcess]:
    """
    Connects to a WiFi network using networksetup on macOS systems.

    Args:
        ssid (str): The SSID of the WiFi network to connect to.
        password (str): The password for the WiFi network.

    Returns:
        subprocess.CompletedProcess or None: Result of the connection attempt.
    """
    if not shutil.which("networksetup"):
        logger.error(" --- connect_wifi_mac: 'networksetup' is not present. Please install 'networksetup'.")
        return None

    result = None
    try:
        # Get the Wi-Fi interface
        interface_result = subprocess.run(
            ["/usr/sbin/networksetup", "-listallhardwareports"],
            capture_output=True, text=True
        )
        interface = "en0"   # 'en0' is by default
        for block in interface_result.stdout.split("\n\n"):
            if "Wi-Fi" in block:
                for line in block.splitlines():
                    if "Device" in line:
                        interface = line.split(":")[1].strip()
                        break

        logger.info(f" --- connect_wifi_mac: Using interface: {interface}")
        result = subprocess.run([
            "networksetup",
            "-setairportnetwork", interface, ssid, password
        ], check=True, capture_output=True, text=True)
        await asyncio.sleep(WIFI_WAIT_SECONDS)

    except subprocess.CalledProcessError as e:
        logger.error(f" --- connect_wifi_mac: Error connecting with networksetup: {e.stderr.strip()}")
    except Exception as e:
        logger.error(f" --- connect_wifi_mac: Unexpected exception while trying to connect to {ssid}: {e}")
    finally:
        return result


async def connect_host_wifi(ssid: str, password: str) -> None:
    """
    Connects the host machine to a WiFi network by detecting the OS and using the appropriate method.
    Checks in which OS (Linux or Darwin only) the script is running and calls the corresponding connect_wifi function.

    Args:
        ssid (str): The SSID of the WiFi network to connect to.
        password (str): The password for the WiFi network.
    """
    os_name = platform.system()
    logger.info(f" --- connect_host_wifi: OS detected: {os_name}")

    # Let's try to connect the TH to the second network a few times, to avoid network instability
    retry = 1
    while retry <= MAX_RETRIES:
        logger.info(f" --- connect_host_wifi: Trying to connect to {ssid} - {retry}/{MAX_RETRIES}")
        try:
            conn = None
            if os_name == "Linux":
                conn = await connect_wifi_linux(ssid, password)
            elif os_name == "Darwin":
                conn = await connect_wifi_mac(ssid, password)
            else:
                logger.error(" --- connect_host_wifi: OS not supported.")
            if conn and conn.returncode == 0:
                logger.info(f" --- connect_host_wifi: Connected to {ssid}")
                break
            elif conn:
                stderr_msg = conn.stderr if conn.stderr else "No stderr"
                logger.error(
                    f" --- connect_host_wifi: Attempt {retry} failed. Return code: {conn.returncode} stderr: {stderr_msg}")
            else:
                logger.error(" --- connect_host_wifi: No result returned from connection attempt.")

        except subprocess.CalledProcessError as e:
            logger.error(
                f" --- connect_host_wifi: Exception: {e} when trying to connect to {ssid} stderr: {conn.stderr.decode()}")
        finally:
            retry += 1


def is_network_switch_successful(err) -> bool:
    """
    Verifies if a network switch command succeeded by checking the networkingStatus attribute.

    Args:
        err: The response object or exception from the network switch command.

    Returns:
        bool: True if the network switch was successful, False otherwise.
    """
    return (
        err is None or
        (hasattr(err, "networkingStatus") and
         err.networkingStatus == cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)
    )


async def change_networks(test, cluster, ssid: str, password: str, breadcrumb: int) -> None:
    """
    Changes the DUT's WiFi network and switches the TH's network accordingly.
    Handles retries and verifies successful reconnection.

    Args:
        test: Test context to send commands.
        cluster: Network Commissioning Cluster to send commands through.
        ssid (str): The SSID of the WiFi network to switch to.
        password (str): The password for the WiFi network.
        breadcrumb (int): Breadcrumb value for ConnectNetwork command.
    """
    # ConnectNetwork tells the DUT to change to the second Wi-Fi network, while TH stays in the first one
    # so they loose connection between each other. Thus, ConnectNetwork can throw an exception
    retry = 1
    while retry <= MAX_RETRIES:
        try:
            if retry > 1:
                # Let's wait a couple seconds to finish changing networks
                await asyncio.sleep(WIFI_WAIT_SECONDS)
            err = await asyncio.wait_for(
                test.send_single_cmd(
                    cmd=cluster.Commands.ConnectNetwork(
                        networkID=ssid,
                        breadcrumb=breadcrumb
                    )
                ),
                timeout=TIMEOUT
            )
            logger.info(f" --- change_networks: err type: {type(err)}, value: {err}")
            success = is_network_switch_successful(err)
            if success:
                logger.info(" --- change_networks: Network switch successful.")
                break
            else:
                logger.warning(f" --- change_networks: Error during network switch: {err}")

        except Exception as e:
            logger.error(f" --- change_networks: Lost connection with the DUT. Exception: {e}")

        # After telling the DUT to change networks, we must change TH to the second network, so it can find the DUT
        # But first wait a couple of seconds so the DUT finishes changing networks
        await asyncio.sleep(WIFI_WAIT_SECONDS)
        await asyncio.wait_for(
            connect_host_wifi(ssid=ssid, password=password),
            timeout=TIMEOUT
        )
        retry += 1
    else:
        logger.error(f" --- change_networks: Failed to switch networks after {MAX_RETRIES} retries.")

    async def find_network_and_assert(self, networks, ssid, should_be_connected=True):
        asserts.assert_is_not_none(networks, "Could not read networks.")
        for idx, network in enumerate(networks):
            if network.networkID == ssid.encode():
                connection_state = "connected" if network.connected else "not connected"
                asserts.assert_equal(network.connected, should_be_connected, f"Wifi network {ssid} is {connection_state}.")
                return idx
        asserts.fail(f"Wifi network not found for SSID: {ssid}")

    async def verify_operational_network(self, ssid):
        networks = None
        retry = 1
        while retry <= MAX_RETRIES:
            logger.info(f" --- verify_operational_network: Trying to verify operational network: {retry}/{MAX_RETRIES}")
            try:
                networks = await asyncio.wait_for(
                    self.read_single_attribute_check_success(
                        cluster=cnet,
                        attribute=cnet.Attributes.Networks
                    ),
                    timeout=TIMEOUT
                )
            except Exception as e:
                logger.error(f" --- verify_operational_network: Exception reading networks: {e}")
                # Let's wait a couple of seconds to change networks
                await asyncio.sleep(WIFI_WAIT_SECONDS)
            finally:
                if networks and len(networks) > 0 and networks[0].connected:
                    logger.info(f" --- verify_operational_network: networks: {networks}")
                    break
                else:
                    retry += 1
        else:
            asserts.fail(f" --- verify_operational_network: Could not read networks after {MAX_RETRIES} retries.")

        userwifi_netidx = await self.find_network_and_assert(networks, ssid)
        if userwifi_netidx is not None:
            logger.info(f" --- verify_operational_network: DUT connected to SSID: {ssid}")

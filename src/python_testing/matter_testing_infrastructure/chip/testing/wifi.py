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

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)


MAX_RETRIES = 5
TIMEOUT = 900
TIMED_REQUEST_TIMEOUT_MS = 5000
WIFI_WAIT_SECONDS = 5


async def connect_wifi_linux(ssid, password):
    """ Connects to WiFi with the SSID and Password provided as arguments using 'nmcli' in Linux."""
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


async def connect_wifi_mac(ssid, password):
    """ Connects to WiFi with the SSID and Password provided as arguments using 'networksetup' in Mac."""
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


async def connect_host_wifi(ssid, password):
    """ Checks in which OS (Linux or Darwin only) the script is running and calls the corresponding connect_wifi function. """
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


def is_network_switch_successful(err):
    """ Verifies if networkingStatus is 0 (kSuccess) """
    return (
        err is None or
        (hasattr(err, "networkingStatus") and
         err.networkingStatus == cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)
    )


async def change_networks(test, cluster, ssid, password, breadcrumb):
    """ Changes networks in DUT by sending ConnectNetwork command and
        changes TH network by calling connect_host_wifi function."""
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

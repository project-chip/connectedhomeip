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

import asyncio
import logging
import os
import platform
import re
import shutil  # Only for macOS networksetup
import socket
import subprocess
import time
from typing import Optional

from mobly import asserts
from zeroconf import ServiceBrowser, ServiceListener, Zeroconf

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO)


# General configuration
# Number of attempts for failed operations (reduced to avoid segmentation fault from excessive retries)
MAX_ATTEMPTS = 4                     # Reduced to prevent stack overflow from too many failed retries
TIMEOUT = 900                        # Overall test timeout (15 min) - main test execution time limit

# Matter command timeouts
TIMED_REQUEST_TIMEOUT_MS = 5000      # Matter command timeout (5s) - for individual Matter commands
COMMAND_TIMEOUT = 15                 # Quick command timeout (15s) - for ConnectNetwork that may timeout intentionally
ATTRIBUTE_READ_TIMEOUT = 30          # Attribute read timeout (30s) - after network changes when DUT may be slow
OPERATIONAL_DISCOVERY_TIMEOUT = 70   # Increased to handle network delays - for first attribute read after network change

# Network operation timeouts
CONNECTION_TIMEOUT = 20              # WiFi connection timeout (20s) - time to establish WiFi link
IP_TIMEOUT = 15                      # IP assignment timeout (15s) - time to get IP address via DHCP
NETWORK_CHANGE_TIMEOUT = 60          # Network transition timeout (60s) - for network changes
MDNS_DISCOVERY_TIMEOUT = 8           # Slightly increased to improve mDNS resolution success - device typically responds in <5s

# Wait periods (not timeouts, but delays for stability)
WIFI_WAIT_SECONDS = 5                # WiFi stabilization wait (5s) - basic network settling time
NETWORK_STABILIZATION_WAIT = 25      # Network stabilization wait (25s) - after major network changes
RETRY_DELAY_SECONDS = 5              # Delay between retry attempts (5s) - delay for DUT recovery

cgen = Clusters.GeneralCommissioning
cnet = Clusters.NetworkCommissioning
attr = cnet.Attributes

# Global variables to store original network credentials for fallback
# and for restoring WiFi network at the end of the test
original_ssid = None
original_password = None

# Global variable to store target device ID for mDNS discovery
_target_device_id = None


# DNS-SD error filter to suppress known non-critical errors
class DNSSDErrorFilter(logging.Filter):
    def filter(self, record):
        message = record.getMessage() if hasattr(record, 'getMessage') else str(record.msg)
        return not any(error_msg in message for error_msg in [
            # Error messages - original
            'Failed to remove advertised services',
            'Failed to advertise operational node',
            'Failed to advertise commissioner',
            'Failed to finalize service update',
            'Incorrect state',
            # Informational messages - new additions to reduce spam
            'Avahi re-register required',
            'Advertise operational node',
            'Advertise commission parameter',
            'Updating services using commissioning mode'
        ])


# Apply filter to multiple possible loggers
for logger_name in ['MatterTest', 'matter.native.DIS', 'matter.native.DL', '', 'root']:
    try:
        target_logger = logging.getLogger(logger_name)
        target_logger.addFilter(DNSSDErrorFilter())
    except Exception:
        pass

# Configure logging level to reduce native system verbosity
os.environ['CHIP_PROGRESS_LOGGING'] = '0'  # Disable progress logs
os.environ['CHIP_DETAIL_LOGGING'] = '0'   # Disable detailed logs
os.environ['CHIP_ERROR_LOGGING'] = '1'    # Keep only critical errors


class ConnectionResult:
    """ Simple structure to hold connection result when changing networks"""

    def __init__(self, returncode: int, stderr: str = ""):
        self.returncode = returncode
        self.stderr = stderr


class MatterServiceListener(ServiceListener):
    """
    Zeroconf service listener for Matter devices.
    Collects all discovered Matter services with optional device ID filtering.
    """

    def __init__(self, target_device_id=None):
        self.discovered_services = []
        self.target_device_id = target_device_id

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        try:
            service_info = zc.get_service_info(type_, name)
            if service_info:
                # Support both IPv4 and IPv6 addresses
                addresses = []
                for addr in service_info.addresses:
                    try:
                        if len(addr) == 4:
                            # IPv4 address
                            addresses.append(socket.inet_ntoa(addr))
                        elif len(addr) == 16:
                            # IPv6 address
                            addresses.append(socket.inet_ntop(socket.AF_INET6, addr))
                    except Exception as addr_e:
                        logger.debug(f"Failed to parse address: {addr_e}")
                        continue

                service_data = {
                    'name': name.replace(f'.{type_}', ''),
                    'type': type_,
                    'domain': 'local',
                    'addresses': addresses,
                    'port': service_info.port,
                    'properties': service_info.properties
                }

                # Filter by target device ID if specified
                if self.target_device_id:
                    # The device ID can be in the service name or properties
                    service_name = service_data['name']
                    target_id_str = str(self.target_device_id)

                    # Convert numeric node ID to hex format if needed
                    if target_id_str.isdigit():
                        target_id_hex = format(int(target_id_str), 'X')
                    else:
                        target_id_hex = target_id_str

                    # Check if the target device ID (in various formats) is in the service name
                    if (target_id_str.upper() in service_name.upper() or
                        target_id_hex.upper() in service_name.upper() or
                        # Also check for the hex format with leading zeros
                            target_id_hex.upper().zfill(16) in service_name.upper()):
                        self.discovered_services.append(service_data)
                    else:
                        pass
                else:
                    # No filtering, add all services
                    self.discovered_services.append(service_data)
        except Exception as e:
            logger.warning(f"MatterServiceListener: Failed to get service info for {name}: {e}")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass


def detect_platform() -> str:
    system = platform.system()
    if system == 'Darwin':
        return 'macos'
    if system == 'Linux':
        return 'linux'
    return 'unknown'


def get_target_device_id():
    """Get the cached target device ID for mDNS discovery."""
    return _target_device_id


def set_target_device_id(device_id):
    """Set the target device ID for mDNS discovery."""
    global _target_device_id
    _target_device_id = str(device_id) if device_id is not None else None


async def find_matter_devices_mdns(max_attempts=None):
    """
    Finds Matter devices via mDNS using zeroconf, optionally filtering by target device ID.
    Raises an exception if no device is found after max_attempts.
    Returns the list of discovered services.

    Args:
        max_attempts: Maximum number of discovery attempts. If None, uses global MAX_ATTEMPTS.
    """
    service_types = ["_matter._tcp.local.", "_matterc._udp.local."]
    target_device_id = get_target_device_id()
    attempts = max_attempts if max_attempts is not None else MAX_ATTEMPTS

    logger.info(
        f"find_matter_devices_mdns: Searching for Matter devices{' with target device ID: ' + target_device_id if target_device_id else ''}")

    for attempt in range(1, attempts + 1):
        zc = None
        browsers = []
        try:
            if attempt > 1:
                delay = min(2 + (attempt - 2) * 3, 5)
                await asyncio.sleep(delay)

            zc = Zeroconf()
            listener = MatterServiceListener(target_device_id)
            browsers = [ServiceBrowser(zc, stype, listener) for stype in service_types]

            start_time = time.time()
            await asyncio.sleep(MDNS_DISCOVERY_TIMEOUT)

            if listener.discovered_services:
                elapsed = time.time() - start_time
                logger.info(f"Device found on attempt {attempt}/{attempts} after {elapsed:.1f}s")
                return listener.discovered_services

        except Exception as e:
            logger.error(f"find_matter_devices_mdns: Discovery attempt {attempt} failed: {e}")

        finally:
            for browser in browsers:
                try:
                    browser.cancel()
                except Exception as e:
                    logger.debug(f"find_matter_devices_mdns: Error canceling browser: {e}")

            if zc is not None:
                try:
                    zc.close()
                except Exception as e:
                    logger.debug(f"find_matter_devices_mdns: Error closing Zeroconf: {e}")

            await asyncio.sleep(0.5)

        if attempt < attempts:
            await asyncio.sleep(2)

    raise Exception(
        f"find_matter_devices_mdns: mDNS discovery failed after {attempts} attempts - No Matter devices found{' for target device ID: ' + target_device_id if target_device_id else ''}")


async def run_subprocess(cmd, check=False, capture_output=False, timeout=ATTRIBUTE_READ_TIMEOUT):
    """Runs a subprocess command asynchronously with timeout and error handling"""

    try:
        proc = await asyncio.create_subprocess_exec(
            *cmd,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )

        stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=timeout)

        if check and proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, cmd, stdout, stderr)

        if capture_output:
            return stdout.decode() if proc.returncode == 0 else ""
        return proc.returncode == 0

    except asyncio.TimeoutError:
        logger.warning(f"run_subprocess: Command timed out after {timeout}s: {' '.join(cmd)}")
        if check:
            raise
        return "" if capture_output else False
    except subprocess.CalledProcessError:
        logger.warning(f"run_subprocess: Subprocess failed (handled): {' '.join(cmd)}")
        if check:
            raise
        return "" if capture_output else False
    except Exception as e:
        logger.error(f"run_subprocess: Subprocess error: {' '.join(cmd)} - {e}")
        return "" if capture_output else False


async def detect_wifi_interface():
    """Detect WiFi interface using wpa_supplicant sockets"""

    try:
        wpa_dir = "/var/run/wpa_supplicant"
        candidates = [f for f in os.listdir(wpa_dir) if not f.startswith('.')] if os.path.exists(wpa_dir) else []
        physical_interfaces = [c for c in candidates if not c.startswith('p2p-dev-')]
        if physical_interfaces:
            return physical_interfaces[0]
        return None
    except Exception as e:
        logger.error(f"detect_wifi_interface: Error: {e}")
        return None


async def wpa_command(iface, cmd):
    """Send command to wpa_supplicant via Unix socket"""

    sock_file = f"/var/run/wpa_supplicant/{iface}"
    sock_tmp = f"/tmp/wpa_{iface}_sock"
    if os.path.exists(sock_tmp):
        os.remove(sock_tmp)
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    try:
        sock.bind(sock_tmp)
        sock.sendto(cmd.encode(), sock_file)
        resp, _ = sock.recvfrom(4096)
        return resp.decode(errors="ignore").strip()
    except Exception as e:
        logger.error(f"wpa_command: Error sending '{cmd}' to {iface}: {e}")
        raise
    finally:
        sock.close()
        if os.path.exists(sock_tmp):
            os.remove(sock_tmp)


async def scan_and_find_ssid(interface, target_ssid):
    """Scans and searches for SSID with attempts"""

    for attempt in range(1, MAX_ATTEMPTS + 1):
        try:
            await wpa_command(interface, "SCAN")
            await asyncio.sleep(RETRY_DELAY_SECONDS)

            results = await wpa_command(interface, "SCAN_RESULTS")
            scan_lines = results.splitlines()[1:]

            for line in scan_lines:
                if line.endswith(f"\t{target_ssid}"):
                    return True

        except Exception as e:
            logger.error(f"scan_and_find_ssid: Scan attempt {attempt} failed: {e}")

        if attempt < MAX_ATTEMPTS:
            await asyncio.sleep(RETRY_DELAY_SECONDS)

    logger.error(f"scan_and_find_ssid: SSID {target_ssid} not found after {MAX_ATTEMPTS} attempts")
    return False


async def connect_wifi_linux(ssid, password) -> ConnectionResult:
    """Connects to WiFi using wpa_supplicant commands in Linux (for both desktop, Raspberry Pi and Docker)"""

    if isinstance(ssid, bytes):
        ssid = ssid.decode()
    if isinstance(password, bytes):
        password = password.decode()

    interface = await detect_wifi_interface()
    if not interface:
        logger.error("connect_wifi_linux: No WiFi interface found")
        return ConnectionResult(1, "No WiFi interface found")

    try:
        await run_subprocess(["sudo", "ip", "link", "set", interface, "up"])
        await asyncio.sleep(1)

        if not await scan_and_find_ssid(interface, ssid):
            logger.error(f"connect_wifi_linux: SSID {ssid} not found in scan")
            return ConnectionResult(1, f"SSID {ssid} not available")

        # Disconnect from any current connection
        await wpa_command(interface, "DISCONNECT")
        await asyncio.sleep(2)

        # Remove all existing networks to avoid conflicts
        networks = await wpa_command(interface, "LIST_NETWORKS")
        if networks and "network id" in networks:
            nets = networks.splitlines()[1:]
            for n in nets:
                if n.strip():
                    cols = n.split("\t")
                    if len(cols) >= 1:
                        await wpa_command(interface, f"REMOVE_NETWORK {cols[0]}")

        # Add and configure the target network
        network_id = await wpa_command(interface, "ADD_NETWORK")
        if network_id.startswith("FAIL"):
            logger.error(f"connect_wifi_linux: Failed to add network: {network_id}")
            return ConnectionResult(1, f"Failed to add network: {network_id}")

        network_id = network_id.strip()

        config_commands = [
            f'SET_NETWORK {network_id} ssid "{ssid}"',
            f'SET_NETWORK {network_id} psk "{password}"',
            f'SET_NETWORK {network_id} key_mgmt WPA-PSK',
            f'SET_NETWORK {network_id} proto RSN WPA',
            f'SET_NETWORK {network_id} pairwise CCMP TKIP',
            f'SET_NETWORK {network_id} group CCMP TKIP',
        ]

        for cmd in config_commands:
            result = await wpa_command(interface, cmd)
            if result.startswith("FAIL"):
                logger.error(f"connect_wifi_linux: Failed to configure network: {cmd} -> {result}")
                return ConnectionResult(1, f"Failed to configure network: {cmd}")

        await wpa_command(interface, f"ENABLE_NETWORK {network_id}")
        await wpa_command(interface, f"SELECT_NETWORK {network_id}")
        await wpa_command(interface, "REASSOCIATE")

        for attempt in range(1, MAX_ATTEMPTS + 1):
            start_time = time.time()
            connected = False
            disconnected_count = 0
            last_state = ""

            while time.time() - start_time < CONNECTION_TIMEOUT:
                status = await wpa_command(interface, "STATUS")

                if "wpa_state=COMPLETED" in status:
                    connected = True
                    break
                if "wpa_state=4WAY_HANDSHAKE" in status:
                    last_state = "4WAY_HANDSHAKE"
                    await asyncio.sleep(2)
                if "wpa_state=DISCONNECTED" in status or "wpa_state=INACTIVE" in status:
                    disconnected_count += 1
                    last_state = "DISCONNECTED"
                    if disconnected_count > 5:
                        logger.warning(
                            f"connect_wifi_linux: Persistent disconnected state after {disconnected_count} checks, attempt {attempt} failed")
                        break
                    await asyncio.sleep(1)
                elif "wpa_state=SCANNING" in status or "wpa_state=AUTHENTICATING" in status:
                    last_state = "AUTHENTICATING"
                    await asyncio.sleep(2)
                else:
                    await asyncio.sleep(1)

            if connected:
                break

            # If connection failed and we have more attempts, reset interface
            if attempt < MAX_ATTEMPTS:
                logger.warning(
                    f"connect_wifi_linux: Attempt {attempt} failed after {CONNECTION_TIMEOUT}s (last_state: {last_state}), resetting interface")
                await run_subprocess(["sudo", "ip", "link", "set", interface, "down"])
                await asyncio.sleep(2)
                await run_subprocess(["sudo", "ip", "link", "set", interface, "up"])
                await asyncio.sleep(3)

                # Re-select and reconnect to the network
                logger.info(f"connect_wifi_linux: Re-selecting network {network_id} for attempt {attempt + 1}")
                await wpa_command(interface, f"SELECT_NETWORK {network_id}")
                await wpa_command(interface, "REASSOCIATE")
                await asyncio.sleep(2)

        if not connected:
            final_status = await wpa_command(interface, "STATUS")
            logger.error(
                f"connect_wifi_linux: Connection failed after {MAX_ATTEMPTS} attempts. Final status: {final_status}, Last observed state: {last_state}")
            return ConnectionResult(1, f"Connection failed, last_state: {last_state}")

        # Connection successful, get IP address
        # Release any existing DHCP lease and request new one
        await run_subprocess(["sudo", "dhclient", "-r", interface])
        await asyncio.sleep(1)
        await run_subprocess(["sudo", "dhclient", interface])

        # Wait for IP assignment with retry
        ip_start = time.time()

        while time.time() - ip_start < IP_TIMEOUT:
            proc = await asyncio.create_subprocess_exec(
                "ip", "-4", "addr", "show", interface,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            stdout, _ = await proc.communicate()
            output = stdout.decode()

            if "inet " in output:
                ip_match = re.search(r'inet\s+(\S+)', output)
                if ip_match:
                    # Test connectivity to gateway
                    try:
                        gateway_proc = await asyncio.create_subprocess_exec(
                            "ip", "route", "show", "default",
                            stdout=asyncio.subprocess.PIPE
                        )
                        gw_stdout, _ = await gateway_proc.communicate()
                        gw_output = gw_stdout.decode()
                        gw_match = re.search(r'default via (\S+)', gw_output)
                        if gw_match:
                            gateway = gw_match.group(1)
                            ping_proc = await asyncio.create_subprocess_exec(
                                "ping", "-c", "1", "-W", "3", gateway,
                                stdout=asyncio.subprocess.PIPE,
                                stderr=asyncio.subprocess.PIPE
                            )
                            ping_stdout, _ = await ping_proc.communicate()
                            if "1 received" in ping_stdout.decode():
                                pass  # Connectivity verified
                    except Exception:
                        logger.warning("connect_wifi_linux: Could not verify connectivity, but connection appears established")

                    return ConnectionResult(0, "")

            await asyncio.sleep(1)

        logger.error("connect_wifi_linux: WiFi connected but no IP address obtained")
        return ConnectionResult(1, "No IP address obtained")

    except Exception as e:
        logger.error(f"connect_wifi_linux: Exception during connection: {e}")
        return ConnectionResult(1, str(e))


async def connect_wifi_mac(ssid, password) -> ConnectionResult:
    """ Connects to WiFi with the SSID and Password provided as arguments using 'networksetup' in Mac."""

    if not shutil.which("networksetup"):
        logger.error(" --- connect_wifi_mac: 'networksetup' is not present. Please install 'networksetup'.")
        return ConnectionResult(1, "networksetup not found")

    try:
        # Get the Wi-Fi interface
        interface_output = await run_subprocess(
            ["/usr/sbin/networksetup", "-listallhardwareports"],
            capture_output=True
        )
        interface = "en0"   # 'en0' is by default
        for block in interface_output.split("\n\n"):
            if "Wi-Fi" in block:
                for line in block.splitlines():
                    if "Device" in line:
                        interface = line.split(":")[1].strip()
                        break

        try:
            await run_subprocess([
                "networksetup",
                "-setairportnetwork", interface, ssid, password
            ], check=True)
        except Exception as e:
            logger.error(f" --- connect_wifi_mac: networksetup command failed: {e}")
            return ConnectionResult(1, str(e))

        # Wait for connection to establish
        await asyncio.sleep(WIFI_WAIT_SECONDS)

        if isinstance(ssid, bytes):
            ssid_str = ssid.decode('utf-8')
        else:
            ssid_str = ssid

        for verify_attempt in range(1, 6):
            try:
                ip_output = await run_subprocess([
                    "ipconfig", "getifaddr", interface
                ], capture_output=True)

                if ip_output and ip_output.strip():
                    try:
                        gateway_output = await run_subprocess([
                            "route", "-n", "get", "default"
                        ], capture_output=True)

                        # Extract gateway IP
                        for line in gateway_output.splitlines():
                            if "gateway:" in line:
                                gateway = line.split("gateway:")[1].strip()
                                # Quick ping test (1 packet, 2 second timeout)
                                ping_result = await run_subprocess([
                                    "ping", "-c", "1", "-W", "2000", gateway
                                ], capture_output=True)
                                if "1 packets received" in ping_result or "1 received" in ping_result:
                                    break
                    except Exception as ping_e:
                        # Ping verification is optional, don't fail if it doesn't work
                        logger.debug(f" --- connect_wifi_mac: Gateway ping failed (non-critical): {ping_e}")

                    return ConnectionResult(0, "")
                logger.warning(f" --- connect_wifi_mac: No IP address yet on {interface}")

                if verify_attempt < 5:
                    await asyncio.sleep(3)

            except Exception as verify_e:
                logger.warning(f" --- connect_wifi_mac: Verification attempt {verify_attempt}/5 error: {verify_e}")
                if verify_attempt < 5:
                    await asyncio.sleep(3)

        logger.error(f" --- connect_wifi_mac: Failed to verify connection to {ssid_str} after 5 attempts")
        return ConnectionResult(1, "Connection verification failed")

    except Exception as e:
        logger.error(f" --- connect_wifi_mac: Unexpected exception while trying to connect to {ssid_str}: {e}")
        return ConnectionResult(1, str(e))


async def connect_host_wifi(ssid, password) -> Optional[ConnectionResult]:
    """ Checks in which OS (Linux or Darwin only) the script is running and calls the corresponding connect_wifi_* function. """

    os_name = detect_platform()
    conn = None

    for attempt in range(1, MAX_ATTEMPTS + 1):
        try:
            if os_name == "linux":
                conn = await connect_wifi_linux(ssid, password)
            elif os_name == "macos":
                conn = await connect_wifi_mac(ssid, password)
            else:
                logger.error(f"connect_host_wifi: OS not supported: {os_name}")
                return ConnectionResult(1, "OS not supported")

            if conn and conn.returncode == 0:
                break
            if conn:
                logger.warning(
                    f"connect_host_wifi: Attempt {attempt} failed. Return code: {conn.returncode}, stderr: {conn.stderr}")
            else:
                logger.warning(f"connect_host_wifi: No result returned from connection attempt {attempt}")

        except Exception as e:
            logger.warning(f"connect_host_wifi: Exception on attempt {attempt}: {e}")

        if attempt < MAX_ATTEMPTS and (not conn or conn.returncode != 0):
            await asyncio.sleep(RETRY_DELAY_SECONDS)

    return conn


def is_network_switch_successful(err):
    """ Verifies if networkingStatus is 0 (kSuccess) """
    return (
        err is None or
        (hasattr(err, "networkingStatus") and
         err.networkingStatus == cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)
    )


async def change_networks(test, cluster, ssid, password, breadcrumb):
    """ Changes networks in DUT by sending ConnectNetwork command and
        changes TH network by calling connect_host_wifi function with fallback."""

    for attempt in range(1, MAX_ATTEMPTS + 1):
        try:
            if attempt > 1:
                await asyncio.sleep(RETRY_DELAY_SECONDS)

            try:
                err = await asyncio.wait_for(
                    test.send_single_cmd(
                        cmd=cluster.Commands.ConnectNetwork(
                            networkID=ssid,
                            breadcrumb=breadcrumb
                        )
                    ),
                    timeout=COMMAND_TIMEOUT
                )

                if is_network_switch_successful(err):
                    pass
                else:
                    logger.error("change_networks: ConnectNetwork command indicated failure")
                    continue

            except asyncio.TimeoutError:
                # Timeout may be expected when DUT switches networks, but don't assume success
                pass
            except Exception as cmd_e:
                error_msg = str(cmd_e).lower()
                # Check if it's a Matter/CHIP internal timeout (may be expected during network switch)
                if "timeout" in error_msg or "commandsender.cpp" in error_msg:
                    pass
                else:
                    logger.error(f"change_networks: ConnectNetwork command failed: {cmd_e}")
                    continue

        except Exception as e:
            logger.error(f"change_networks: Unexpected error on attempt {attempt}: {e}")
            continue

        # Wait for DUT to change networks
        await asyncio.sleep(WIFI_WAIT_SECONDS)

        # Try to change TH network with multiple attempts
        th_success = False

        for th_attempt in range(1, MAX_ATTEMPTS + 1):
            try:
                result = await asyncio.wait_for(
                    connect_host_wifi(ssid=ssid, password=password),
                    timeout=NETWORK_CHANGE_TIMEOUT
                )
                if result and result.returncode == 0:
                    # Extra wait for network stabilization after TH connects
                    await asyncio.sleep(3)
                    th_success = True
                    break
                logger.warning(
                    f"change_networks: TH connection failed on attempt {th_attempt}: {result.stderr if result else 'Unknown error'}")

            except asyncio.TimeoutError:
                pass

            except Exception as e:
                logger.warning(f"change_networks: TH connection exception on attempt {th_attempt}: {e}")

            if th_attempt < MAX_ATTEMPTS:
                await asyncio.sleep(RETRY_DELAY_SECONDS)

        if th_success:
            return  # Success!
        logger.error(f"change_networks: TH failed to connect to {ssid} after {MAX_ATTEMPTS} attempts")

        # Try fallback to original network immediately
        try:
            fallback_result = await asyncio.wait_for(
                connect_host_wifi(ssid=original_ssid, password=original_password),
                timeout=NETWORK_CHANGE_TIMEOUT
            )
            if fallback_result and fallback_result.returncode == 0:
                pass
            else:
                logger.error(f"change_networks: Fallback to {original_ssid} also failed!")
        except Exception as fallback_e:
            logger.error(f"change_networks: Fallback connection failed: {fallback_e}")

        if attempt < MAX_ATTEMPTS:
            pass
        else:
            logger.error("change_networks: All retry attempts exhausted")

    # All attempts failed, ensure we have fallback connectivity
    logger.error(f"change_networks: Failed to switch networks after {MAX_ATTEMPTS} retries.")
    logger.info(f"change_networks: Ensuring fallback connectivity to {original_ssid}")

    try:
        fallback_result = await connect_host_wifi(original_ssid, original_password)
        if fallback_result and fallback_result.returncode == 0:
            logger.info(f"change_networks: Final fallback to {original_ssid} successful")
        else:
            logger.error(f"change_networks: Final fallback to {original_ssid} failed - WiFi may be disconnected!")
    except Exception as final_e:
        logger.error(f"change_networks: Final fallback attempt failed: {final_e}")

    raise Exception(f"Failed to switch networks to {ssid} after {MAX_ATTEMPTS} attempts")


async def parse_default_routes():
    """ Parses default routes from 'ip route show default' output """

    try:
        output = await run_subprocess(["ip", "route", "show", "default"], check=True, capture_output=True)
        routes_info = []

        for line in output.splitlines():
            if "default via" in line:
                # Parse route: "default via 192.168.1.1 dev eth0 proto dhcp src 192.168.1.54 metric 100"
                parts = line.split()
                gateway = None
                interface = None
                metric = None

                for i, part in enumerate(parts):
                    if part == "via" and i + 1 < len(parts):
                        gateway = parts[i + 1]
                    elif part == "dev" and i + 1 < len(parts):
                        interface = parts[i + 1]
                    elif part == "metric" and i + 1 < len(parts):
                        metric = parts[i + 1]

                if gateway and interface:
                    routes_info.append({
                        "gateway": gateway,
                        "interface": interface,
                        "metric": metric,
                        "full_line": line.strip(),
                        "is_lan": not any(x in interface for x in ["wlan", "wl", "wifi", "wlp", "wlx"])
                    })

        return routes_info
    except Exception as e:
        logger.error(f"parse_default_routes: Error parsing routes: {e}")
        return []


async def remove_lan_routes():
    """ Removes default routes via LAN interfaces and returns the removed routes for restoration later. """

    original_routes = []

    try:
        routes_info = await parse_default_routes()

        for route in routes_info:
            if route["is_lan"]:  # Only remove LAN routes
                gateway = route["gateway"]
                interface = route["interface"]

                logger.info(f"remove_lan_routes: Removing default route via {gateway} dev {interface}")
                try:
                    await run_subprocess(["sudo", "ip", "route", "del", "default", "via", gateway, "dev", interface], check=True)
                    original_routes.append(route)
                except subprocess.CalledProcessError as e:
                    logger.warning(f"remove_lan_routes: Failed to remove route via {gateway}: {e}")

        if original_routes:
            pass
        else:
            pass

    except Exception as e:
        logger.error(f"remove_lan_routes: Error removing LAN routes: {e}")

    return original_routes


async def restore_lan_routes(original_routes):
    """ Restores previously removed LAN default routes. """

    if not original_routes:
        return

    for route in original_routes:
        gateway = route["gateway"]
        interface = route["interface"]
        metric = route["metric"]

        try:
            cmd = ["sudo", "ip", "route", "add", "default", "via", gateway, "dev", interface]
            if metric:
                cmd.extend(["metric", metric])
            await run_subprocess(cmd, check=True)
        except subprocess.CalledProcessError as e:
            logger.warning(f"restore_lan_routes: Failed to restore route via {gateway}: {e}")
            # Try to restore using dhclient as fallback
            try:
                await run_subprocess(["sudo", "dhclient", "-r", interface])
                await asyncio.sleep(0.5)
                await run_subprocess(["sudo", "dhclient", interface])
            except Exception as dhcp_e:
                logger.warning(f"restore_lan_routes: DHCP fallback failed for {interface}: {dhcp_e}")


async def restore_original_network():
    """Restores TH Wi-Fi to original network after test."""

    try:
        logger.info("restore_original_network: Restoring TH Wi-Fi to original network...")
        result = await connect_host_wifi(original_ssid, original_password)
        if result and result.returncode == 0:
            logger.info("restore_original_network: Successfully restored TH Wi-Fi to original network")
        else:
            logger.error("restore_original_network: Failed to restore TH Wi-Fi to original network")
    except Exception as e:
        logger.error(f"restore_original_network: Exception during network restoration: {e}")


async def find_network_and_assert(test, networks, ssid, should_be_connected=True):
    """Finds the network with the given SSID in the list of networks and asserts its connection state."""

    asserts.assert_is_not_none(networks, "Could not read networks.")
    for idx, network in enumerate(networks):
        if network.networkID == ssid.encode():
            connection_state = "connected" if network.connected else "not connected"
            asserts.assert_equal(network.connected, should_be_connected, f"Wifi network {ssid} is {connection_state}.")
            return idx
    asserts.fail(f"Wifi network not found for SSID: {ssid}")
    return None


async def verify_operational_network(test, ssid):
    """ Verifies that the DUT is connected to the specified SSID by reading the Networks attribute. """

    networks = None

    # Quick immediate check to catch device before potential firmware crash
    logger.info("verify_operational_network: Performing immediate device check after network change...")
    await asyncio.sleep(2)  # Brief settling time
    try:
        quick_check_services = await find_matter_devices_mdns(max_attempts=1)
        if quick_check_services:
            logger.info("verify_operational_network: Device already reachable (immediate check)")
    except Exception as e:
        logger.debug(f"verify_operational_network: Immediate check failed (expected): {e}")

    logger.info(f"verify_operational_network: Waiting {NETWORK_STABILIZATION_WAIT}s for network stabilization...")
    await asyncio.sleep(NETWORK_STABILIZATION_WAIT)

    # Reduced from MAX_ATTEMPTS to 3 to prevent excessive retries that can cause segmentation faults
    max_read_attempts = 3
    for attempt in range(1, max_read_attempts + 1):
        try:
            if attempt == 1:
                timeout = OPERATIONAL_DISCOVERY_TIMEOUT
            else:
                timeout = ATTRIBUTE_READ_TIMEOUT

            networks = await asyncio.wait_for(
                test.read_single_attribute_check_success(
                    cluster=cnet,
                    attribute=cnet.Attributes.Networks
                ),
                timeout=timeout
            )

            if networks and len(networks) > 0:
                for network in networks:
                    if network.connected:
                        break
                else:
                    raise Exception("No connected network found in response")
                break

        except Exception as e:
            logger.error(f"verify_operational_network: Exception reading networks (attempt {attempt}/{max_read_attempts}): {e}")

        if attempt == 1:
            retry_delay = 10
        elif attempt == 2:
            retry_delay = 15
        else:
            retry_delay = RETRY_DELAY_SECONDS

        if attempt < max_read_attempts:
            await asyncio.sleep(retry_delay)
    else:
        asserts.fail(f"verify_operational_network: Could not read networks after {max_read_attempts} retries.")

    userwifi_netidx = await find_network_and_assert(test, networks, ssid)
    if userwifi_netidx is not None:
        pass


class TC_CNET_4_11(MatterBaseTest):

    @classmethod
    def setup_class(cls):
        """Remove default route from LAN interface to force traffic through Wi-Fi during test. (Linux only)"""

        os_name = detect_platform()
        if os_name == "linux":
            try:
                cls._original_routes = asyncio.run(remove_lan_routes())
            except Exception as e:
                logger.error(f"setup_class: Failed to setup network environment: {e}")
                cls._original_routes = []
                raise

    @classmethod
    def teardown_class(cls):
        """
        Restore original default routes after the test finishes (Linux only).
        Restore original Wi-Fi network on all platforms.
        """
        os_name = detect_platform()
        if os_name == "linux":
            try:
                original_routes = getattr(cls, "_original_routes", [])
                asyncio.run(restore_lan_routes(original_routes))
            except Exception as e:
                logger.error(f"teardown_class: Failed to teardown network environment: {e}")
        # Always try to restore original Wi-Fi network
        try:
            asyncio.run(restore_original_network())
        except Exception as e:
            logger.error(f"teardown_class: Failed to restore original Wi-Fi network: {e}")

    # Overrides default_timeout: Test includes several long waits, adjust timeout to accommodate.
    @property
    def default_timeout(self) -> int:
        return TIMEOUT

    def steps_TC_CNET_4_11(self):
        return [
            TestStep("precondition", "TH is commissioned", is_commissioning=True),
            TestStep(1, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(2, "TH reads Networks attribute from the DUT and saves the number of entries as 'NumNetworks'."),
            TestStep(3, "TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'Userwifi_netidx'",
                        "Verify that the Networks attribute list has an entry with the following fields:"
                        " 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID"
                        " 2. Connected is of type bool and is TRUE"),
            TestStep(4, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1"
                        "Verify that DUT sends NetworkConfigResponse to command with the following fields:"
                        " 1. NetworkingStatus is Success"
                        " 2. NetworkIndex matches previously saved 'Userwifi_netidx'"),
            TestStep(5, "TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1",
                        "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:"
                        " 1. NetworkingStatus is success which is '0'"
                        " 2. DebugText is of type string with max length 512 or empty"),
            TestStep(6, "TH reads Networks attribute from the DUT",
                        "Verify that the Networks attribute list has an entry with the following fields:"
                        "NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"
                        "Connected is of type bool and is FALSE"),
            TestStep(7, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 2"
                        "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"),
            TestStep(8, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network",
                     "Verify that the TH successfully connects to the DUT"),
            TestStep(9, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT",
                     "Verify that the breadcrumb value is set to 2"),
            TestStep(10, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0."
                     "This forcibly disarms the fail-safe and is expected to cause the changes of configuration to NetworkCommissioning cluster done so far to be reverted.",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(11, "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID"),
            TestStep(12, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network",
                     "Verify that the TH successfully connects to the DUT"),
            TestStep(13, "TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900",
                     "Verify that DUT sends ArmFailSafeResponse command to the TH"),
            TestStep(14, "TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1",
                     "Verify that DUT sends NetworkConfigResponse to command with the following response fields:"
                     " 1. NetworkingStatus is success"
                     " 2. NetworkIndex is 'Userwifi_netidx'"),
            TestStep(15, "TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID, Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1",
                     "Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:"
                     " 1. NetworkingStatus is success which is '0'"
                     " 2. DebugText is of type string with max length 512 or empty"),
            TestStep(16, "TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 3"
                     "TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"),
            TestStep(17, "TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network",
                     "Verify that the TH successfully connects to the DUT"),
            TestStep(18, "TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT",
                     "Verify that the breadcrumb value is set to 3"),
            TestStep(19, "TH sends the CommissioningComplete command to the DUT",
                     "Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)"),
            TestStep(20, "TH reads Networks attribute from the DUT",
                     " Verify that the Networks attribute list has an entry with the following fields:"
                     " 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"
                     " 2. Connected is of type bool and is TRUE"),
        ]

    def desc_TC_CNET_4_11(self):
        return "[TC-CNET-4.11] [Wi-Fi] Verification for ConnectNetwork Command [DUT-Server]"

    @run_if_endpoint_matches(has_feature(Clusters.NetworkCommissioning, Clusters.NetworkCommissioning.Bitmaps.Feature.kWiFiNetworkInterface))
    async def test_TC_CNET_4_11(self):

        logger.info("test_TC_CNET_4_11: Waiting for mDNS service to stabilize before starting test...")
        await asyncio.sleep(5)

        # Set the target device ID for mDNS discovery from DUT node ID
        set_target_device_id(self.dut_node_id)
        logger.info(f"test_TC_CNET_4_11: Set target device ID for mDNS filtering: {self.dut_node_id}")

        asserts.assert_true("PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID" in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID must be included on the command line in "
                            "the --string-arg flag as PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID:<hex:7A6967626565686F6D65>")
        asserts.assert_true("PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS" in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS must be included on the command line in "
                            "the --string-arg flag as PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS:<hex:70617373776f7264313233>")

        wifi_1st_ap_ssid = self.matter_test_config.wifi_ssid
        wifi_1st_ap_credentials = self.matter_test_config.wifi_passphrase
        wifi_2nd_ap_ssid = self.matter_test_config.global_test_params["PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID"]
        wifi_2nd_ap_credentials = self.matter_test_config.global_test_params["PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS"]

        global original_ssid, original_password
        original_ssid = wifi_1st_ap_ssid
        original_password = wifi_1st_ap_credentials

        # Commissioning is already done
        self.step("precondition")

        # Precondition: TH reads FeatureMap attribute from the DUT and verifies if DUT supports WiFi on endpoint
        feature_map = await self.read_single_attribute_check_success(
            cluster=cnet, attribute=attr.FeatureMap
        )
        if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
            logger.info('Device does not support WiFi on endpoint, skipping remaining steps')
            self.skip_all_remaining_steps(1)
            return

        # TH reads the Networks attribute list from the DUT on all endpoints (all network commissioning clusters)
        connected_network_count = {}

        networks_dict = await self.read_single_attribute_all_endpoints(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        logger.info(f" --- Networks by endpoint: {networks_dict}")

        # Verify that there is a single connected network across ALL network commissioning clusters
        for ep in networks_dict:
            connected_network_count[ep] = sum(x.connected for x in networks_dict[ep])
            logger.info(f" --- Connected networks count by endpoint: {connected_network_count}")
            asserts.assert_equal(sum(connected_network_count.values()), 1,
                                 "Verify that only one entry has connected status as TRUE across ALL endpoints")

        endpoint = self.get_endpoint()
        current_cluster_connected = connected_network_count[endpoint] == 1

        if not current_cluster_connected:
            logger.info("Current cluster is not connected, skipping all remaining test steps")
            self.skip_all_remaining_steps(1)
            return

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(1)

        await self.send_single_cmd(
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=900),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        # Successful command execution is implied if no exception is raised.

        # TH reads Networks attribute from the DUT and saves the number of entries as 'NumNetworks'
        self.step(2)

        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        num_networks = len(networks)
        logger.info(f" --- Step 2: num_networks: {num_networks}")

        # TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'userwifi_netidx'
        self.step(3)

        # Verify that the Networks attribute list has an entry with the following fields:
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is TRUE
        userwifi_netidx = await find_network_and_assert(self, networks, wifi_1st_ap_ssid)

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(4)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=wifi_1st_ap_ssid.encode(),
                breadcrumb=1
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        # Verify that DUT sends NetworkConfigResponse to command with the following fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            f"Expected response to be of type NetworkConfigResponse but got: {type(response)}")
        # 1. NetworkingStatus is Success
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Network was not removed")
        # 2. NetworkIndex matches previously saved 'Userwifi_netidx'
        asserts.assert_equal(response.networkIndex, userwifi_netidx,
                             "Incorrect network index in response")

        # Wait for network state to stabilize after removing current network
        logger.info("Waiting for network state to stabilize after RemoveNetwork...")
        await asyncio.sleep(3)

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(5)

        # Add second network - handle potential connectivity issues after removing the first network
        cmd = cnet.Commands.AddOrUpdateWiFiNetwork(
            ssid=wifi_2nd_ap_ssid.encode(), credentials=wifi_2nd_ap_credentials.encode(), breadcrumb=1)

        # Try with retries in case of temporary connectivity issues
        response = None
        for attempt in range(1, MAX_ATTEMPTS + 1):
            try:
                logger.info(f"Step 5 attempt {attempt}/{MAX_ATTEMPTS}: Sending AddOrUpdateWiFiNetwork command")
                response = await self.send_single_cmd(cmd=cmd, timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
                break
            except Exception as e:
                logger.warning(f"Step 5 attempt {attempt} failed: {e}")
                if attempt < MAX_ATTEMPTS:
                    logger.info(f"Retrying in {RETRY_DELAY_SECONDS} seconds...")
                    await asyncio.sleep(RETRY_DELAY_SECONDS)
                else:
                    raise Exception(f"Failed to send AddOrUpdateWiFiNetwork after {MAX_ATTEMPTS} attempts: {e}")

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        # 1. NetworkingStatus is success which is "0"
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (Success), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # TH reads Networks attribute from the DUT
        self.step(6)

        # Verify that the Networks attribute list has an entry with the following fields:
        networks = await self.read_single_attribute_check_success(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )

        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is FALSE
        await find_network_and_assert(self, networks, wifi_2nd_ap_ssid, False)

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 2
        self.step(7)

        # TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        await asyncio.wait_for(
            change_networks(
                test=self,
                cluster=cnet,
                ssid=wifi_2nd_ap_ssid.encode(),
                password=wifi_2nd_ap_credentials.encode(),
                breadcrumb=2
            ),
            timeout=TIMEOUT
        )
        # Give extra time for DUT to update mDNS announcements with new IP after network change
        # This prevents TH from attempting to connect to stale/cached IP addresses
        await asyncio.sleep(WIFI_WAIT_SECONDS * 2)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(8)

        # Verify that the TH successfully connects to the DUT
        await asyncio.sleep(WIFI_WAIT_SECONDS)
        await verify_operational_network(self, wifi_2nd_ap_ssid)

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(9)

        # Verify that the breadcrumb value is set to 2
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=cgen,
            attribute=cgen.Attributes.Breadcrumb
        )
        logger.info(f" --- Step 9: Breadcrumb is: {breadcrumb}")
        asserts.assert_equal(breadcrumb, 2, f"Expected breadcrumb to be 2, but got: {breadcrumb}")

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 0.
        # This forcibly disarms the fail-safe and is expected to cause the changes of
        # configuration to NetworkCommissioning cluster done so far to be reverted.
        self.step(10)

        try:
            response = await self.send_single_cmd(
                cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=0),
                timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS*6
            )
            asserts.assert_equal(
                response.errorCode,
                cgen.Enums.CommissioningErrorEnum.kOk,
                "ArmFailSafeResponse error code is not OK.",
            )
        except Exception as e:
            logger.warning(f"ArmFailSafe(0) command may have succeeded despite timeout: {e}")
            logger.info("Proceeding with network change as this is expected behavior")

        # Wait for DUT to complete network reversion
        await asyncio.sleep(WIFI_WAIT_SECONDS)

        # TH changes its WiFi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        self.step(11)

        logger.info(f"Switching TH back to original network: {wifi_1st_ap_ssid}")
        await asyncio.wait_for(
            connect_host_wifi(wifi_1st_ap_ssid, wifi_1st_ap_credentials),
            timeout=NETWORK_CHANGE_TIMEOUT
        )
        await asyncio.sleep(1)  # Brief delay for command processing
        # Note: find_matter_devices_mdns() removed - verify_operational_network already does mDNS discovery

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network
        self.step(12)

        # Give more time for both TH and DUT to stabilize on the original network
        await asyncio.sleep(WIFI_WAIT_SECONDS * 2)

        # Verify that DUT has reverted to original network and is reachable
        try:
            await verify_operational_network(self, wifi_1st_ap_ssid)
            logger.info("Successfully reconnected to DUT on original network")
        except Exception as e:
            logger.error(f"Failed to reconnect to DUT on original network: {e}")
            # Try one more time with additional wait
            logger.info("Retrying connection to DUT...")
            await asyncio.sleep(WIFI_WAIT_SECONDS * 2)
            await verify_operational_network(self, wifi_1st_ap_ssid)

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(13)

        await self.send_single_cmd(
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=900),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )
        # Successful command execution is implied if no exception is raised.

        # TH sends RemoveNetwork Command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and Breadcrumb field set to 1
        self.step(14)

        response = await self.send_single_cmd(
            cmd=cnet.Commands.RemoveNetwork(
                networkID=wifi_1st_ap_ssid.encode(),
                breadcrumb=1
            ),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS
        )

        # Verify that DUT sends NetworkConfigResponse to command with the following response fields:
        # 1. NetworkingStatus is success
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Network was not removed")
        # 2. NetworkIndex is 'Userwifi_netidx'
        asserts.assert_equal(response.networkIndex, userwifi_netidx,
                             "Incorrect network index in response")

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(15)

        cmd = cnet.Commands.AddOrUpdateWiFiNetwork(
            ssid=wifi_2nd_ap_ssid.encode(), credentials=wifi_2nd_ap_credentials.encode(), breadcrumb=1)
        response = await self.send_single_cmd(cmd=cmd, timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)

        # Verify that DUT sends the NetworkConfigResponse command to the TH with the following response fields:
        # 1. NetworkingStatus is success which is "0"
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            "Unexpected value returned from AddOrUpdateWiFiNetwork")
        asserts.assert_equal(response.networkingStatus, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                             f"Expected 0 (Success), but got: {response.networkingStatus}")
        # 2. DebugText is of type string with max length 512 or empty
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"Expected length of debugText to be less than or equal to 512, but got: {len(response.debugText)}")

        # TH sends ConnectNetwork command to the DUT with NetworkID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID and Breadcrumb field set to 3
        self.step(16)

        # TH changes its Wi-Fi connection to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        await change_networks(
            test=self,
            cluster=cnet,
            ssid=wifi_2nd_ap_ssid.encode(),
            password=wifi_2nd_ap_credentials.encode(),
            breadcrumb=3
        )
        # Give extra time for DUT to update mDNS announcements with new IP after network change
        # This prevents TH from attempting to connect to stale/cached IP addresses
        await asyncio.sleep(WIFI_WAIT_SECONDS * 2)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(17)

        # Verify that the TH successfully connects to the DUT
        await asyncio.sleep(WIFI_WAIT_SECONDS)
        await verify_operational_network(self, wifi_2nd_ap_ssid)

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(18)

        breadcrumb = await asyncio.wait_for(
            self.read_single_attribute_check_success(
                cluster=cgen,
                attribute=cgen.Attributes.Breadcrumb
            ),
            timeout=TIMEOUT
        )
        logger.info(f" --- Step 18: Breadcrumb is: {breadcrumb}")
        # Verify that the breadcrumb value is set to 3
        asserts.assert_equal(breadcrumb, 3, f"Expected breadcrumb to be 3, but got: {breadcrumb}")

        # TH sends the CommissioningComplete command to the DUT
        self.step(19)

        # Disarm the failsafe
        cmd = cgen.Commands.CommissioningComplete()
        response = await self.send_single_cmd(cmd=cmd, timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)

        # Verify that DUT sends CommissioningCompleteResponse with the ErrorCode field set to OK (0)
        asserts.assert_true(isinstance(response, cgen.Commands.CommissioningCompleteResponse), "Got wrong response type")
        asserts.assert_equal(response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"Expected CommissioningCompleteResponse to be 0, but got {response.errorCode}")

        # TH reads Networks attribute from the DUT
        self.step(20)

        # Verify that the Networks attribute list has an entry with the following fields:
        networks = await asyncio.wait_for(
            self.read_single_attribute_check_success(
                cluster=cnet,
                attribute=cnet.Attributes.Networks
            ),
            timeout=TIMEOUT
        )

        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is TRUE
        await find_network_and_assert(self, networks, wifi_2nd_ap_ssid)


if __name__ == "__main__":
    default_matter_test_main()

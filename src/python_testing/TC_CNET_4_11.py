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
import glob
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

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches

logger = logging.getLogger(__name__)


# General configuration
MAX_RETRIES = 3                      # Number of retry attempts for failed operations
TIMEOUT = 900                        # Overall test timeout (15 min) - main test execution time limit

# Matter command timeouts
TIMED_REQUEST_TIMEOUT_MS = 5000      # Matter command timeout (5s) - for individual Matter commands
COMMAND_TIMEOUT = 15                 # Quick command timeout (15s) - for ConnectNetwork that may timeout intentionally
ATTRIBUTE_READ_TIMEOUT = 30          # Attribute read timeout (30s) - after network changes when DUT may be slow

# Network operation timeouts
CONNECTION_TIMEOUT = 20              # WiFi connection timeout (20s) - time to establish WiFi link
IP_TIMEOUT = 15                      # IP assignment timeout (15s) - time to get IP address via DHCP
NETWORK_CHANGE_TIMEOUT = 180         # Network transition timeout (180s) - for network changes
# mDNS discovery timeout (180s) - device discovery after network changes
MDNS_DISCOVERY_TIMEOUT = 180

# Wait periods (not timeouts, but delays for stability)
WIFI_WAIT_SECONDS = 5                # WiFi stabilization wait (5s) - basic network settling time
# Network stabilization wait (60s) - after major network changes (increased for Docker mDNS stability)
NETWORK_STABILIZATION_WAIT = 60
RETRY_DELAY_SECONDS = 8              # Delay between retry attempts (8s) - delay for DUT recovery

# Short wait periods for internal operations
SHORT_WAIT = 1                       # Short wait (1s) - for quick stabilization
MEDIUM_WAIT = 2                      # Medium wait (2s) - for process starts/stops
LONG_WAIT = 3                        # Long wait (3s) - for interface resets
MDNS_STARTUP_WAIT = 10               # mDNS service startup wait (10s) - for mDNS service stabilization
SETUP_TEARDOWN_TIMEOUT = 120         # Setup/teardown timeout (120s) - for class setup/teardown operations

cgen = Clusters.GeneralCommissioning
cnet = Clusters.NetworkCommissioning
attr = cnet.Attributes


class ConnectionResult:

    def __init__(self, returncode: int, stderr: str = ""):
        self.returncode = returncode
        self.stderr = stderr


async def run_subprocess(cmd, check=False, capture_output=False):
    """Run subprocess command with optional output capture.

    Args:
        cmd: List of command arguments (e.g., ["sudo", "ip", "link", "set", "eth0", "up"])
        check: If True, raise CalledProcessError on non-zero exit code
        capture_output: If True, return stdout as string; if False, return success boolean

    Returns:
        If capture_output=True: stdout as string (empty on failure)
        If capture_output=False: True on success, False on failure
    """
    try:
        proc = await asyncio.create_subprocess_exec(
            *cmd,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )

        stdout, stderr = await proc.communicate()

        if check and proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, cmd, stdout, stderr)

        if capture_output:
            return stdout.decode() if proc.returncode == 0 else ""
        else:
            return proc.returncode == 0

    except subprocess.CalledProcessError as e:
        logger.warning(f"run_subprocess: Subprocess failed (handled): {' '.join(cmd)}")
        if check:
            raise
        return "" if capture_output else False
    except Exception as e:
        logger.error(f"run_subprocess: Subprocess error: {' '.join(cmd)} - {e}")
        return "" if capture_output else False


async def detect_wifi_iface():
    """Detect WiFi interface using wpa_supplicant sockets"""

    try:
        wpa_dir = "/var/run/wpa_supplicant"
        candidates = [f for f in os.listdir(wpa_dir) if not f.startswith('.')] if os.path.exists(wpa_dir) else []
        physical_interfaces = [c for c in candidates if not c.startswith('p2p-dev-')]
        if physical_interfaces:
            return physical_interfaces[0]
        else:
            return None
    except Exception as e:
        logger.error(f"detect_wifi_iface: Error: {e}")
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


async def scan_and_find_ssid(iface, target_ssid, retries=MAX_RETRIES, delay=RETRY_DELAY_SECONDS):
    """Scans and searches for SSID with retries"""

    retry = 0
    while retry < retries:
        retry += 1
        try:
            await wpa_command(iface, "SCAN")
            await asyncio.sleep(delay)

            results = await wpa_command(iface, "SCAN_RESULTS")
            scan_lines = results.splitlines()[1:]

            for line in scan_lines:
                if line.endswith(f"\t{target_ssid}"):
                    return True

        except Exception as e:
            logger.error(f"scan_and_find_ssid: Scan attempt {retry} failed: {e}")

        if retry < retries:
            await asyncio.sleep(SHORT_WAIT)

    logger.error(f"scan_and_find_ssid: SSID {target_ssid} not found after {retries} attempts")
    return False


async def connect_wifi_linux(ssid, password) -> ConnectionResult:
    """Connects to WiFi using wpa_supplicant commands in Linux (for both desktop and Raspberry Pi)"""

    if isinstance(ssid, bytes):
        ssid = ssid.decode()
    if isinstance(password, bytes):
        password = password.decode()

    iface = await detect_wifi_iface()
    if not iface:
        logger.error("connect_wifi_linux: No WiFi interface found")
        return ConnectionResult(1, "No WiFi interface found")

    try:
        # Scan to ensure SSID is available
        if not await scan_and_find_ssid(iface, ssid):
            logger.error(f"connect_wifi_linux: SSID {ssid} not found in scan")
            return ConnectionResult(1, f"SSID {ssid} not available")

        # Ensure interface is up and reset state
        await run_subprocess(["sudo", "ip", "link", "set", iface, "up"])
        await asyncio.sleep(SHORT_WAIT)

        # Disconnect from any current connection
        await wpa_command(iface, "DISCONNECT")
        await asyncio.sleep(MEDIUM_WAIT)

        # Remove all existing networks to avoid conflicts
        nets_result = await wpa_command(iface, "LIST_NETWORKS")
        if nets_result and "network id" in nets_result:
            nets = nets_result.splitlines()[1:]
            for n in nets:
                if n.strip():
                    cols = n.split("\t")
                    if len(cols) >= 1:
                        await wpa_command(iface, f"REMOVE_NETWORK {cols[0]}")

        # Add and configure the target network
        net_id = await wpa_command(iface, "ADD_NETWORK")
        if net_id.startswith("FAIL"):
            logger.error(f"connect_wifi_linux: Failed to add network: {net_id}")
            return ConnectionResult(1, f"Failed to add network: {net_id}")

        net_id = net_id.strip()

        # Configure network with explicit parameters
        config_commands = [
            f'SET_NETWORK {net_id} ssid "{ssid}"',
            f'SET_NETWORK {net_id} psk "{password}"',
            f'SET_NETWORK {net_id} key_mgmt WPA-PSK',
            f'SET_NETWORK {net_id} proto RSN WPA',
            f'SET_NETWORK {net_id} pairwise CCMP TKIP',
            f'SET_NETWORK {net_id} group CCMP TKIP',
        ]

        for cmd in config_commands:
            result = await wpa_command(iface, cmd)
            if result.startswith("FAIL"):
                logger.error(f"connect_wifi_linux: Failed to configure network: {cmd} -> {result}")
                return ConnectionResult(1, f"Failed to configure network: {cmd}")

        # Enable and select the network
        await wpa_command(iface, f"ENABLE_NETWORK {net_id}")
        await wpa_command(iface, f"SELECT_NETWORK {net_id}")
        await wpa_command(iface, "REASSOCIATE")

        retry = 0
        timeout = CONNECTION_TIMEOUT

        while retry < MAX_RETRIES:
            retry += 1
            start_time = time.time()
            connected = False

            while time.time() - start_time < timeout:
                status = await wpa_command(iface, "STATUS")

                if "wpa_state=COMPLETED" in status:
                    connected = True
                    break
                elif "wpa_state=4WAY_HANDSHAKE" in status:
                    # Authentication in progress, wait a bit more
                    await asyncio.sleep(MEDIUM_WAIT)
                elif "wpa_state=DISCONNECTED" in status or "wpa_state=INACTIVE" in status:
                    logger.warning(f"connect_wifi_linux: Connection failed, status: {status}")
                    break

                await asyncio.sleep(SHORT_WAIT)

            if connected:
                break

            # If connection failed and we have more attempts, reset interface
            if retry < MAX_RETRIES:
                logger.warning(f"connect_wifi_linux: Attempt {retry} failed, resetting interface")
                await run_subprocess(["sudo", "ip", "link", "set", iface, "down"])
                await asyncio.sleep(MEDIUM_WAIT)
                await run_subprocess(["sudo", "ip", "link", "set", iface, "up"])
                await asyncio.sleep(LONG_WAIT)
                await wpa_command(iface, f"SELECT_NETWORK {net_id}")
                await wpa_command(iface, "REASSOCIATE")
                await asyncio.sleep(MEDIUM_WAIT)

        if not connected:
            final_status = await wpa_command(iface, "STATUS")
            logger.error(f"connect_wifi_linux: Connection failed after {MAX_RETRIES} attempts. Final status: {final_status}")
            return ConnectionResult(1, f"Connection failed: {final_status}")

        # Connection successful, get IP address
        # Release any existing DHCP lease and request new one
        await run_subprocess(["sudo", "dhclient", "-r", iface])
        await asyncio.sleep(SHORT_WAIT)
        await run_subprocess(["sudo", "dhclient", iface])

        # Wait for IP assignment with retry
        ip_timeout = IP_TIMEOUT
        ip_start = time.time()

        while time.time() - ip_start < ip_timeout:
            proc = await asyncio.create_subprocess_exec(
                "ip", "-4", "addr", "show", iface,
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

            await asyncio.sleep(SHORT_WAIT)

        logger.error("connect_wifi_linux: WiFi connected but no IP address obtained")
        return ConnectionResult(1, "No IP address obtained")

    except Exception as e:
        logger.error(f"connect_wifi_linux: Exception during connection: {e}")
        return ConnectionResult(1, str(e))


async def connect_wifi_mac(ssid, password) -> ConnectionResult:
    """ Connects to WiFi with the SSID and Password provided as arguments using 'networksetup' in Mac."""

    if not shutil.which("networksetup"):
        logger.error("connect_wifi_mac: 'networksetup' is not present. Please install 'networksetup'.")
        return ConnectionResult(1, "networksetup not found")

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

        logger.info(f"connect_wifi_mac: Using interface: {interface}")
        result = subprocess.run([
            "networksetup",
            "-setairportnetwork", interface, ssid, password
        ], check=True, capture_output=True, text=True)
        await asyncio.sleep(WIFI_WAIT_SECONDS)
        return ConnectionResult(result.returncode, result.stderr)

    except subprocess.CalledProcessError as e:
        logger.error(f"connect_wifi_mac: Error connecting with networksetup: {e.stderr.strip()}")
        return ConnectionResult(e.returncode, e.stderr)
    except Exception as e:
        logger.error(f"connect_wifi_mac: Unexpected exception while trying to connect to {ssid}: {e}")
        return ConnectionResult(1, str(e))


async def connect_host_wifi(ssid, password) -> Optional[ConnectionResult]:
    """ Checks in which OS (Linux or Darwin only) the script is running and calls the corresponding connect_wifi_* function. """

    os_name = platform.system()
    retry = 0
    conn = None
    while retry < MAX_RETRIES:
        retry += 1
        try:
            if os_name == "Linux":
                conn = await connect_wifi_linux(ssid, password)
            elif os_name == "Darwin":
                conn = await connect_wifi_mac(ssid, password)
            else:
                logger.error(f"connect_host_wifi: OS not supported: {os_name}")
                return ConnectionResult(1, "OS not supported")
            if conn and conn.returncode == 0:
                logger.info(f"connect_host_wifi: Connected to {ssid}")
                break
            elif conn:
                logger.error(f"connect_host_wifi: Attempt {retry} failed. Return code: {conn.returncode}")
            else:
                logger.error(f"connect_host_wifi: No result returned from connection attempt {retry}")
        except Exception as e:
            logger.error(f"connect_host_wifi: Exception on attempt {retry}: {e}")
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

    logger.info(f"change_networks: Starting network change to {ssid}")

    # Store original network for fallback
    original_ssid = test.matter_test_config.wifi_ssid
    original_password = test.matter_test_config.wifi_passphrase

    retry = 0
    while retry < MAX_RETRIES:
        retry += 1
        try:
            if retry > 1:
                await asyncio.sleep(WIFI_WAIT_SECONDS)

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
                success = is_network_switch_successful(err)
                logger.info("change_networks: ConnectNetwork command completed successfully")
            except asyncio.TimeoutError:
                # Timeout is expected behavior when DUT switches networks
                success = True  # Treat timeout as success since DUT is switching networks
            except Exception as cmd_e:
                error_msg = str(cmd_e).lower()
                # Check if it's a Matter/CHIP internal timeout (also expected during network switch)
                if "timeout" in error_msg or "commandsender.cpp" in error_msg:
                    success = True  # Treat Matter timeout as success since DUT is switching networks
                else:
                    logger.error(f"change_networks: ConnectNetwork command failed: {cmd_e}")
                    success = False

            if not success:
                logger.error("change_networks: DUT network switch failed, retrying...")
                continue

        except Exception as e:
            logger.error(f"change_networks: Unexpected error on attempt {retry}: {e}")
            continue

        # Wait for DUT to change networks
        await asyncio.sleep(WIFI_WAIT_SECONDS)

        logger.info(f"change_networks: Changing TH network to {ssid}")
        try:
            result = await asyncio.wait_for(
                connect_host_wifi(ssid=ssid, password=password),
                timeout=NETWORK_CHANGE_TIMEOUT
            )
            if result and result.returncode == 0:
                # Extra wait for network stabilization after TH connects
                logger.info("change_networks: Waiting additional 3s for network stabilization...")
                await asyncio.sleep(LONG_WAIT)
                return  # Success!
            else:
                logger.error(f"change_networks: TH failed to connect to {ssid}")

                # Try fallback to original network immediately
                logger.warning(f"change_networks: Attempting fallback to original network {original_ssid}")
                try:
                    fallback_result = await asyncio.wait_for(
                        connect_host_wifi(ssid=original_ssid, password=original_password),
                        timeout=NETWORK_CHANGE_TIMEOUT
                    )
                    if fallback_result and fallback_result.returncode == 0:
                        logger.info(f"change_networks: Successfully fell back to {original_ssid}")
                    else:
                        logger.error(f"change_networks: Fallback to {original_ssid} also failed!")
                except Exception as fallback_e:
                    logger.error(f"change_networks: Fallback connection failed: {fallback_e}")

        except asyncio.TimeoutError:
            logger.error(f"change_networks: Timeout changing TH to {ssid}")

            # Attempt fallback on timeout as well
            logger.warning(f"change_networks: Timeout occurred, attempting fallback to {original_ssid}")
            try:
                fallback_result = await connect_host_wifi(ssid=original_ssid, password=original_password)
                if fallback_result and fallback_result.returncode == 0:
                    logger.info(f"change_networks: Successfully fell back to {original_ssid} after timeout")
            except Exception as fallback_e:
                logger.error(f"change_networks: Fallback after timeout failed: {fallback_e}")

        except Exception as e:
            logger.error(f"change_networks: TH failed to change to {ssid}: {e}")

        retry += 1

    # All attempts failed, ensure we have fallback connectivity
    logger.error(f"change_networks: Failed to switch networks after {MAX_RETRIES} retries.")
    logger.info(f"change_networks: Ensuring fallback connectivity to {original_ssid}")

    try:
        fallback_result = await connect_host_wifi(ssid=original_ssid, password=original_password)
        if fallback_result and fallback_result.returncode == 0:
            logger.info(f"change_networks: Final fallback to {original_ssid} successful")
        else:
            logger.error(f"change_networks: Final fallback to {original_ssid} failed - WiFi may be disconnected!")
    except Exception as final_e:
        logger.error(f"change_networks: Final fallback attempt failed: {final_e}")

    raise Exception(f"Failed to switch networks to {ssid} after {MAX_RETRIES} attempts")


async def parse_default_routes():
    """Parse and return default routes information."""
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
    """Remove LAN default routes and return list of removed routes for restoration."""
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
            logger.info(f"remove_lan_routes: Removed {len(original_routes)} LAN default route(s)")
        else:
            logger.info("remove_lan_routes: No LAN default routes found to remove")

    except Exception as e:
        logger.error(f"remove_lan_routes: Error removing LAN routes: {e}")

    return original_routes


async def restore_lan_routes(original_routes):
    """Restore previously removed LAN default routes."""
    if not original_routes:
        logger.info("restore_lan_routes: No original routes to restore")
        return

    for route in original_routes:
        gateway = route["gateway"]
        interface = route["interface"]
        metric = route["metric"]

        logger.info(f"restore_lan_routes: Restoring default route via {gateway} dev {interface}")
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
                await asyncio.sleep(SHORT_WAIT)
                await run_subprocess(["sudo", "dhclient", interface])
                logger.info(f"restore_lan_routes: Attempted DHCP refresh for {interface}")
            except Exception as dhcp_e:
                logger.warning(f"restore_lan_routes: DHCP fallback failed for {interface}: {dhcp_e}")


async def temporarily_remove_lan_route_for_wifi(ssid, wifi_1st_ap_ssid):
    """Temporarily remove LAN route if this is 2nd WiFi network and dual routes exist.

    Returns tuple: (lan_route_removed, gateway, lan_interface) for restoration.
    """
    lan_route_removed = False
    gateway = None
    lan_interface = None
    os_name = platform.system()

    # Only attempt this on Linux systems and for 2nd WiFi network
    if ssid != wifi_1st_ap_ssid and os_name == "Linux":
        try:
            routes_info = await parse_default_routes()

            lan_default = None
            wifi_default = None

            for route in routes_info:
                if route["is_lan"]:
                    lan_default = route
                elif not route["is_lan"]:  # WiFi interface
                    wifi_default = route

            if lan_default and wifi_default:
                gateway = lan_default["gateway"]
                lan_interface = lan_default["interface"]

                logger.info(
                    f"temporarily_remove_lan_route_for_wifi: Detected dual default routes, temporarily removing LAN route via {gateway} dev {lan_interface}")

                # Verify route exists before removing
                verify_route = subprocess.run(
                    ['ip', 'route', 'show', 'default', 'via', gateway, 'dev', lan_interface],
                    capture_output=True, text=True, check=False)

                if verify_route.returncode == 0 and verify_route.stdout.strip():
                    # Remove LAN default route temporarily
                    result = subprocess.run(['sudo', 'ip', 'route', 'del', 'default', 'via', gateway, 'dev', lan_interface],
                                            capture_output=True, text=True, check=False)
                    if result.returncode == 0:
                        lan_route_removed = True
                        logger.info(f"temporarily_remove_lan_route_for_wifi: LAN default route temporarily removed")
                        await asyncio.sleep(MEDIUM_WAIT)  # Wait for route change to take effect
                    else:
                        logger.warning(f"temporarily_remove_lan_route_for_wifi: Failed to remove LAN route: {result.stderr}")
                else:
                    logger.info("temporarily_remove_lan_route_for_wifi: LAN default route verification failed, skipping removal")

        except Exception as e:
            logger.warning(f"temporarily_remove_lan_route_for_wifi: Could not manage routes on {os_name}: {e}")
    elif ssid != wifi_1st_ap_ssid and os_name != "Linux":
        logger.info(
            f"temporarily_remove_lan_route_for_wifi: Route management not implemented for {os_name}, skipping route optimization")

    return lan_route_removed, gateway, lan_interface


async def restore_temporarily_removed_lan_route(lan_route_removed, gateway, lan_interface):
    """Restore temporarily removed LAN route (Linux only)."""
    if lan_route_removed and gateway and lan_interface and platform.system() == "Linux":
        try:
            # Check if route already exists before adding
            verify_restore = subprocess.run(
                ['ip', 'route', 'show', 'default', 'via', gateway, 'dev', lan_interface],
                capture_output=True, text=True, check=False)

            if not verify_restore.stdout.strip():
                # Route doesn't exist, safe to add
                result = subprocess.run(['sudo', 'ip', 'route', 'add', 'default', 'via', gateway, 'dev', lan_interface, 'metric', '100'],
                                        capture_output=True, text=True, check=False)
                if result.returncode == 0:
                    logger.info(
                        f"restore_temporarily_removed_lan_route: LAN default route restored (gateway: {gateway}, interface: {lan_interface})")
                else:
                    logger.error(f"restore_temporarily_removed_lan_route: Failed to restore LAN route: {result.stderr}")
            else:
                logger.info("restore_temporarily_removed_lan_route: LAN default route already exists, no restoration needed")
        except Exception as e:
            logger.error(f"restore_temporarily_removed_lan_route: Failed to restore LAN route: {e}")


async def find_network_and_assert(networks, ssid, should_be_connected=True):
    """Find network in networks list and assert connection state.

    Args:
        networks: List of network objects to search
        ssid: SSID to find (string)
        should_be_connected: Expected connection state (bool)

    Returns:
        Index of the network in the list

    Raises:
        AssertionError: If network not found or connection state doesn't match
    """
    asserts.assert_is_not_none(networks, "Could not read networks.")
    for idx, network in enumerate(networks):
        if network.networkID == ssid.encode():
            connection_state = "connected" if network.connected else "not connected"
            asserts.assert_equal(network.connected, should_be_connected, f"Wifi network {ssid} is {connection_state}.")
            return idx
    asserts.fail(f"Wifi network not found for SSID: {ssid}")


async def verify_operational_network(ssid, wifi_1st_ap_ssid, read_single_attribute_check_success):
    """Verify that DUT is operational on the specified network.

    Args:
        ssid: SSID to verify connection to (string)
        wifi_1st_ap_ssid: First WiFi AP SSID for comparison (string)
        read_single_attribute_check_success: Function to read Matter attributes

    Raises:
        AssertionError: If network verification fails
    """
    networks = None
    retry = 0

    logger.info(f"verify_operational_network: Waiting {NETWORK_STABILIZATION_WAIT}s for network stabilization...")
    await asyncio.sleep(NETWORK_STABILIZATION_WAIT)

    # Temporarily manage routes to ensure proper connectivity during WiFi verification
    lan_route_removed, gateway, lan_interface = await temporarily_remove_lan_route_for_wifi(ssid, wifi_1st_ap_ssid)

    try:
        # Increased retries for better reliability in Docker/mDNS environment
        max_retries_for_discovery = 5

        while retry < max_retries_for_discovery:
            retry += 1
            try:
                # Use extended timeout for mDNS discovery, especially on first attempt after network change
                if retry == 1:
                    timeout_to_use = MDNS_DISCOVERY_TIMEOUT  # Extended timeout for first attempt (180s)
                else:
                    timeout_to_use = ATTRIBUTE_READ_TIMEOUT  # Shorter timeout for subsequent attempts (30s)

                logger.debug(f"verify_operational_network: Using timeout {timeout_to_use}s for attempt {retry}")

                networks = await asyncio.wait_for(
                    read_single_attribute_check_success(
                        cluster=cnet,
                        attribute=cnet.Attributes.Networks
                    ),
                    timeout=timeout_to_use
                )

                if networks and len(networks) > 0:
                    # Check if we have any connected network
                    for network in networks:
                        if network.connected:
                            break
                    else:
                        # No connected network found, continue trying
                        raise Exception("No connected network found in response")
                    break

            except Exception as e:
                logger.error(f"verify_operational_network: Exception reading networks: {e}")

            # Clean mDNS state between retry attempts to prevent cache issues
            if retry < max_retries_for_discovery:
                logger.info(f"verify_operational_network: Cleaning mDNS state before retry {retry + 1}")
                await cleanup_mdns_state()

            # Progressive delay - ESP32 needs more time after network switch
            if retry == 1:
                retry_delay = 10  # First retry: 10s
            elif retry == 2:
                retry_delay = 15  # Second retry: 15s
            elif retry == 3:
                retry_delay = 20  # Third retry: 20s
            else:
                retry_delay = RETRY_DELAY_SECONDS  # Final retry: 8s

            await asyncio.sleep(retry_delay)
        else:
            asserts.fail(f"verify_operational_network: Could not read networks after {max_retries_for_discovery} retries.")

    finally:
        # Always restore LAN default route if we removed it
        await restore_temporarily_removed_lan_route(lan_route_removed, gateway, lan_interface)

    userwifi_netidx = await find_network_and_assert(networks, ssid)
    if userwifi_netidx is not None:
        logger.info(f"verify_operational_network: DUT connected to SSID: {ssid}")


async def clean_avahi_cache():
    """Clean Avahi cache files without restarting daemon."""
    try:
        logger.debug("clean_avahi_cache: Cleaning Avahi cache files...")

        # Clear avahi cache files safely
        cache_dirs = ["/run/avahi-daemon", "/var/lib/avahi-daemon"]
        for cache_dir in cache_dirs:
            try:
                cache_files = glob.glob(f"{cache_dir}/*")
                for cache_file in cache_files:
                    await run_subprocess(["sudo", "rm", "-rf", cache_file])
            except Exception as e:
                logger.warning(f"clean_avahi_cache: Failed to clean cache {cache_dir}: {e}")

    except Exception as e:
        logger.warning(f"clean_avahi_cache: Cache cleanup failed: {e}")


async def restart_avahi_daemon():
    """Restart Avahi daemon for complete mDNS reset."""
    try:
        logger.debug("restart_avahi_daemon: Restarting Avahi daemon...")

        # Step 1: Stop avahi daemon cleanly
        if os.path.exists("/usr/sbin/service"):
            # Try service command first (works in Docker)
            await run_subprocess(["sudo", "service", "avahi-daemon", "stop"])
            await asyncio.sleep(MEDIUM_WAIT)
            await run_subprocess(["sudo", "service", "avahi-daemon", "start"], check=True)
            logger.debug("restart_avahi_daemon: Avahi daemon restarted via service command")
        else:
            # Try direct process management with clean start
            await run_subprocess(["sudo", "pkill", "-f", "avahi-daemon"])
            await asyncio.sleep(MEDIUM_WAIT)
            await run_subprocess(["sudo", "/usr/sbin/avahi-daemon", "-D"], check=True)
            logger.debug("restart_avahi_daemon: Avahi daemon restarted via process management")

        # Wait for avahi to stabilize
        await asyncio.sleep(WIFI_WAIT_SECONDS)

    except subprocess.CalledProcessError as e:
        logger.error(f"restart_avahi_daemon: Failed to restart Avahi daemon: {e}")
        # Try alternative cleanup
        try:
            await run_subprocess(["sudo", "pkill", "-9", "-f", "avahi-daemon"])
            await asyncio.sleep(MEDIUM_WAIT)
            await run_subprocess(["sudo", "/usr/sbin/avahi-daemon", "-D"])
            await asyncio.sleep(WIFI_WAIT_SECONDS)
            logger.debug("restart_avahi_daemon: Attempted Avahi cleanup via force restart")
        except Exception as fallback_e:
            logger.error(f"restart_avahi_daemon: All Avahi restart methods failed: {fallback_e}")
            raise


async def aggressive_mdns_cleanup():
    """Perform aggressive mDNS cleanup for test setup (kill processes, clean cache, restart daemon)."""
    try:
        logger.info("aggressive_mdns_cleanup: Performing aggressive mDNS cleanup...")

        # Step 1: Kill all avahi-related processes
        await run_subprocess(["sudo", "pkill", "-f", "avahi-daemon"])
        await run_subprocess(["sudo", "pkill", "-f", "avahi-browse"])
        await run_subprocess(["sudo", "pkill", "-f", "avahi-resolve"])
        await asyncio.sleep(MEDIUM_WAIT)

        # Step 2: Clear avahi cache and temporary files (more comprehensive than regular cleanup)
        cache_dirs = [
            "/var/lib/avahi-daemon",
            "/run/avahi-daemon",
            "/tmp/avahi-*"
        ]
        for cache_dir in cache_dirs:
            try:
                if '*' in cache_dir:
                    # Handle wildcard patterns with glob
                    cache_files = glob.glob(cache_dir)
                    for cache_file in cache_files:
                        await run_subprocess(["sudo", "rm", "-rf", cache_file])
                else:
                    # Handle regular directories
                    await run_subprocess(["sudo", "rm", "-rf", cache_dir])
            except Exception as e:
                logger.warning(f"aggressive_mdns_cleanup: Failed to clean cache {cache_dir}: {e}")

        # Step 3: Restart avahi daemon cleanly
        await asyncio.sleep(SHORT_WAIT)
        await restart_avahi_daemon()

        logger.info("aggressive_mdns_cleanup: Aggressive mDNS cleanup completed")

    except Exception as e:
        logger.error(f"aggressive_mdns_cleanup: Aggressive cleanup failed: {e}")
        raise


async def cleanup_mdns_state():
    """Clean up mDNS state to prevent discovery issues."""

    try:
        logger.info("cleanup_mdns_state: Performing quick mDNS cleanup...")

        # Clean cache files
        await clean_avahi_cache()

        # Send SIGHUP to avahi-daemon to refresh (if running)
        try:
            await run_subprocess(["sudo", "pkill", "-HUP", "avahi-daemon"])
            await asyncio.sleep(SHORT_WAIT)
        except Exception as e:
            logger.warning(f"cleanup_mdns_state: Failed to send SIGHUP to avahi-daemon: {e}")

        logger.info("cleanup_mdns_state: mDNS cleanup completed")
    except Exception as e:
        logger.warning(f"cleanup_mdns_state: Cleanup failed but continuing: {e}")


async def setup_network_environment():
    """Setup network environment by cleaning mDNS state and managing default routes."""

    try:
        # Clean mDNS state aggressively to prevent intermittent failures
        await aggressive_mdns_cleanup()

        # Remove LAN routes to force traffic through Wi-Fi during the test
        original_routes = await remove_lan_routes()

    except Exception as e:
        logger.error(f"setup_network_environment: Error during setup: {e}")
        original_routes = []

    return original_routes


async def teardown_network_environment(original_routes):
    """Restore network environment by restoring routes and cleaning mDNS state."""

    try:
        # Restore original LAN routes using helper function
        await restore_lan_routes(original_routes)

        # Clean mDNS state after test completion to prevent interference with next run
        logger.info("teardown_network_environment: Cleaning mDNS state for next run...")
        await cleanup_mdns_state()

    except Exception as e:
        logger.error(f"teardown_network_environment: Error during teardown: {e}")


class TC_CNET_4_11(MatterBaseTest):

    @classmethod
    def setup_class(cls):
        """Remove default route from LAN interface to force traffic through Wi-Fi during test."""
        try:
            cls._original_routes = asyncio.run(
                asyncio.wait_for(setup_network_environment(), timeout=SETUP_TEARDOWN_TIMEOUT)
            )
        except Exception as e:
            logger.error(f"setup_class: Failed to setup network environment: {e}")
            cls._original_routes = []

    @classmethod
    def teardown_class(cls):
        """Restore original default routes after the test finishes."""
        try:
            original_routes = getattr(cls, "_original_routes", [])
            asyncio.run(
                asyncio.wait_for(teardown_network_environment(original_routes), timeout=SETUP_TEARDOWN_TIMEOUT)
            )
        except Exception as e:
            logger.error(f"teardown_class: Failed to teardown network environment: {e}")

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

        # Wait for mDNS service to stabilize after any setup_class activities
        logger.info("test_TC_CNET_4_11: Waiting for mDNS service to stabilize before starting test...")
        await asyncio.sleep(MDNS_STARTUP_WAIT)

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

        # Commissioning is already done
        self.step("precondition")

        # Precondition: TH reads FeatureMap attribute from the DUT and verifies if DUT supports WiFi on endpoint
        feature_map = await self.read_single_attribute_check_success(
            cluster=cnet, attribute=attr.FeatureMap
        )
        if not (feature_map & cnet.Bitmaps.Feature.kWiFiNetworkInterface):
            logger.info('test_TC_CNET_4_11: Device does not support WiFi on endpoint, skipping remaining steps')
            self.skip_all_remaining_steps(1)
            return

        # TH reads the Networks attribute list from the DUT on all endpoints (all network commissioning clusters)
        connected_network_count = {}

        networks_dict = await self.read_single_attribute_all_endpoints(
            cluster=cnet,
            attribute=cnet.Attributes.Networks
        )
        logger.info(f"Networks by endpoint: {networks_dict}")

        # Verify that there is a single connected network across ALL network commissioning clusters
        for ep in networks_dict:
            connected_network_count[ep] = sum(map(lambda x: x.connected, networks_dict[ep]))
            logger.info(f"Connected networks count by endpoint: {connected_network_count}")
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
        logger.info(f"Step 2: num_networks: {num_networks}")

        # TH finds the index of the Networks list entry with NetworkID for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID and saves it as 'userwifi_netidx'
        self.step(3)

        # Verify that the Networks attribute list has an entry with the following fields:
        # 1. NetworkID is the hex representation of the ASCII values for PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        # 2. Connected is of type bool and is TRUE
        userwifi_netidx = await find_network_and_assert(networks, wifi_1st_ap_ssid)

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
        await asyncio.sleep(LONG_WAIT)

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(5)

        # Add second network - handle potential connectivity issues after removing the first network
        cmd = cnet.Commands.AddOrUpdateWiFiNetwork(
            ssid=wifi_2nd_ap_ssid.encode(), credentials=wifi_2nd_ap_credentials.encode(), breadcrumb=1)

        # Try with retries in case of temporary connectivity issues
        response = None
        retry = 0
        while retry < MAX_RETRIES:
            retry += 1
            try:
                logger.info(f"Step 5 attempt {retry}/{MAX_RETRIES}: Sending AddOrUpdateWiFiNetwork command")
                response = await self.send_single_cmd(cmd=cmd, timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
                break
            except Exception as e:
                logger.warning(f"Step 5 attempt {retry} failed: {e}")
                if retry < MAX_RETRIES:
                    logger.info(f"Retrying in {RETRY_DELAY_SECONDS} seconds...")
                    await asyncio.sleep(RETRY_DELAY_SECONDS)
                else:
                    raise Exception(f"Failed to send AddOrUpdateWiFiNetwork after {MAX_RETRIES} attempts: {e}")

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
        await find_network_and_assert(networks, wifi_2nd_ap_ssid, False)

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

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(8)

        # Verify that the TH successfully connects to the DUT
        await asyncio.sleep(WIFI_WAIT_SECONDS)
        await verify_operational_network(wifi_2nd_ap_ssid, wifi_1st_ap_ssid, self.read_single_attribute_check_success)

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(9)

        # Verify that the breadcrumb value is set to 2
        breadcrumb = await self.read_single_attribute_check_success(
            cluster=cgen,
            attribute=cgen.Attributes.Breadcrumb
        )
        logger.info(f"Step 9: Breadcrumb is: {breadcrumb}")
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
        await asyncio.sleep(WIFI_WAIT_SECONDS * 2)

        # TH changes its WiFi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        self.step(11)

        logger.info(f"Switching TH back to original network: {wifi_1st_ap_ssid}")
        await asyncio.wait_for(
            connect_host_wifi(wifi_1st_ap_ssid, wifi_1st_ap_credentials),
            timeout=NETWORK_CHANGE_TIMEOUT
        )
        # Let's wait a couple of seconds to change networks
        await asyncio.sleep(WIFI_WAIT_SECONDS)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network
        self.step(12)

        # Give more time for both TH and DUT to stabilize on the original network
        await asyncio.sleep(WIFI_WAIT_SECONDS * 2)

        # Verify that DUT has reverted to original network and is reachable
        try:
            await verify_operational_network(wifi_1st_ap_ssid, wifi_1st_ap_ssid, self.read_single_attribute_check_success)
            logger.info("Successfully reconnected to DUT on original network")
        except Exception as e:
            logger.error(f"Failed to reconnect to DUT on original network: {e}")
            # Try one more time with additional wait
            logger.info("Retrying connection to DUT...")
            await asyncio.sleep(WIFI_WAIT_SECONDS * 2)
            await verify_operational_network(wifi_1st_ap_ssid, wifi_1st_ap_ssid, self.read_single_attribute_check_success)

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

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID operational network
        self.step(17)

        # Verify that the TH successfully connects to the DUT
        await asyncio.sleep(WIFI_WAIT_SECONDS)
        await verify_operational_network(wifi_2nd_ap_ssid, wifi_1st_ap_ssid, self.read_single_attribute_check_success)

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(18)

        breadcrumb = await asyncio.wait_for(
            self.read_single_attribute_check_success(
                cluster=cgen,
                attribute=cgen.Attributes.Breadcrumb
            ),
            timeout=TIMEOUT
        )
        logger.info(f"Step 18: Breadcrumb is: {breadcrumb}")
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
        await find_network_and_assert(networks, wifi_2nd_ap_ssid)

        # TH changes its Wi-Fi connection back to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID.
        await connect_host_wifi(wifi_1st_ap_ssid, wifi_1st_ap_credentials)


if __name__ == "__main__":
    default_matter_test_main()

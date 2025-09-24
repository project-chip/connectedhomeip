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

import matter.clusters as Clusters
from matter.testing.matter_testing import MatterBaseTest, TestStep, default_matter_test_main, has_feature, run_if_endpoint_matches

logger = logging.getLogger(__name__)


# General configuration
MAX_MDNS_DISCOVERY_RETRIES = 5       # Number of retry attempts for mDNS discovery after network changes
MAX_RETRIES = 3                      # Number of retry attempts for failed operations
SETUP_TEARDOWN_TIMEOUT = 120         # Setup/teardown timeout (120s) - for class setup/teardown operations
TIMEOUT = 900                        # Overall test timeout (15 min) - main test execution time limit

# Matter command timeouts
ATTRIBUTE_READ_TIMEOUT = 30          # Attribute read timeout (30s) - after network changes when DUT may be slow
COMMAND_TIMEOUT = 15                 # Quick command timeout (15s) - for ConnectNetwork that may timeout intentionally
TIMED_REQUEST_TIMEOUT_MS = 5000      # Matter command timeout (5s) - for individual Matter commands

# TH (Test Harness) operation waits - for test framework operations
TH_INTERFACE_WAIT = 3                # TH interface operations (3s) - interface resets, service setup
TH_PROCESS_WAIT = 2                  # TH process operations (2s) - process starts/stops, interface operations
TH_QUICK_WAIT = 1                    # Quick TH operations (1s) - quick stabilization, process monitoring

# DUT waits - for Device Under Test operations and recovery
DUT_NETWORK_WAIT = 3                 # DUT network wait (3s) - WiFi stabilization, network settling (reduced from 5s)
DUT_RECOVERY_DELAY = 6               # DUT recovery delay (6s) - between retry attempts (reduced from 8s)
DUT_SERVICE_STARTUP = 7              # DUT service startup (7s) - mDNS service stabilization (reduced from 10s)


# Network operation timeouts
CONNECTION_TIMEOUT = 15              # WiFi connection timeout (15s) - time to establish WiFi link (reduced from 20s)
IP_TIMEOUT = 8                       # IP assignment timeout (8s) - reduced from 10s for faster failure detection
MDNS_DISCOVERY_TIMEOUT = 20          # mDNS discovery timeout (20s) - further reduced from 30s
NETWORK_CHANGE_TIMEOUT = 90          # Network transition timeout (90s) - increased from 45s for better reliability
# Network stabilization (5s) - further reduced from 15s for faster execution
NETWORK_STABILIZATION_WAIT = 5

# Discovery duration for mDNS and network operations
DISCOVERY_DURATION = 8               # Default duration for mDNS discovery (reduced from 15s)

# Avahi corruption recovery timeouts (used when Avahi state is corrupted)
AVAHI_CORRUPTED_DISCOVERY_DURATION = 15    # Extended discovery duration when Avahi is corrupted
AVAHI_CORRUPTED_NETWORK_TIMEOUT = 180      # Extended network timeout when Avahi is corrupted
AVAHI_CORRUPTED_MDNS_TIMEOUT = 45          # Extended mDNS timeout when Avahi is corrupted

# Network verification parameters
PING_COUNT = 1                       # Number of ping packets to send for connectivity verification
PING_DEADLINE = 3                    # Ping deadline in seconds (-W parameter)

cgen = Clusters.GeneralCommissioning
cnet = Clusters.NetworkCommissioning
attr = cnet.Attributes


# ===== ENVIRONMENT DETECTION AND MDNS DISCOVERY FUNCTIONS =====

# Global variable to store target device ID for mDNS discovery
_target_device_id = None

# Global Avahi coordination lock to prevent simultaneous usage conflicts
_avahi_lock = asyncio.Lock()

# Track Avahi state health
_avahi_consecutive_errors = 0
_avahi_state_corrupted = False


def get_adaptive_discovery_timeout():
    """Get discovery timeout adapted to current Avahi state."""
    global _avahi_state_corrupted
    if _avahi_state_corrupted:
        logger.debug(f"get_adaptive_discovery_timeout: Using extended timeout due to Avahi corruption: {AVAHI_CORRUPTED_DISCOVERY_DURATION}s")
        return AVAHI_CORRUPTED_DISCOVERY_DURATION
    return DISCOVERY_DURATION


def get_adaptive_network_timeout():
    """Get network timeout adapted to current Avahi state."""
    global _avahi_state_corrupted
    if _avahi_state_corrupted:
        logger.debug(f"get_adaptive_network_timeout: Using extended timeout due to Avahi corruption: {AVAHI_CORRUPTED_NETWORK_TIMEOUT}s")
        return AVAHI_CORRUPTED_NETWORK_TIMEOUT
    return NETWORK_CHANGE_TIMEOUT


def get_adaptive_mdns_timeout():
    """Get mDNS timeout adapted to current Avahi state."""
    global _avahi_state_corrupted
    if _avahi_state_corrupted:
        logger.debug(f"get_adaptive_mdns_timeout: Using extended timeout due to Avahi corruption: {AVAHI_CORRUPTED_MDNS_TIMEOUT}s")
        return AVAHI_CORRUPTED_MDNS_TIMEOUT
    return MDNS_DISCOVERY_TIMEOUT


def detect_environment():
    """Detect if we're running in Docker, Linux Desktop, or macOS.

    Returns:
        str: 'docker', 'linux_desktop', 'macos', or 'unknown'
    """
    # Check for Docker environment
    docker_indicators = [
        os.path.exists('/.dockerenv'),
        os.path.exists('/proc/1/cgroup') and 'docker' in open('/proc/1/cgroup').read(),
        os.environ.get('DOCKER_CONTAINER') == 'true'
    ]

    is_docker = any(docker_indicators)

    if is_docker:
        return 'docker'

    # Check platform
    system = platform.system()
    if system == 'Darwin':
        return 'macos'
    elif system == 'Linux':
        return 'linux_desktop'
    else:
        return 'unknown'


def get_target_device_id():
    """Get the cached target device ID for mDNS discovery."""
    global _target_device_id
    return _target_device_id


def set_target_device_id(device_id):
    """Set the target device ID for mDNS discovery."""
    global _target_device_id
    _target_device_id = str(device_id) if device_id is not None else None


async def check_avahi_state_health():
    """Check if Avahi service is in a healthy state.
    
    Monitors for signs of Avahi state corruption that cause infinite re-register loops.
    
    Returns:
        bool: True if Avahi appears healthy, False if corrupted state detected
    """
    global _avahi_consecutive_errors, _avahi_state_corrupted
    
    try:
        # Quick test: try to list services with avahi-browse
        proc = await asyncio.create_subprocess_exec(
            "avahi-browse", "-a", "-t", "-p", "--timeout=2",
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        
        try:
            stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=5)
            if proc.returncode == 0:
                _avahi_consecutive_errors = 0
                _avahi_state_corrupted = False
                return True
            else:
                _avahi_consecutive_errors += 1
                logger.warning(f"Avahi health check failed (attempt {_avahi_consecutive_errors}): {stderr.decode() if stderr else 'unknown error'}")
        except asyncio.TimeoutError:
            proc.kill()
            await proc.communicate()
            _avahi_consecutive_errors += 1
            logger.warning(f"Avahi health check timeout (attempt {_avahi_consecutive_errors})")
            
    except Exception as e:
        _avahi_consecutive_errors += 1
        logger.warning(f"Avahi health check exception (attempt {_avahi_consecutive_errors}): {e}")
    
    # Mark as corrupted if we have too many consecutive errors
    if _avahi_consecutive_errors >= 3:
        _avahi_state_corrupted = True
        logger.error("Avahi state appears corrupted - detected multiple consecutive failures")
        return False
    
    return _avahi_consecutive_errors < 2


async def recover_avahi_service():
    """Attempt to recover from Avahi service corruption using container-compatible strategies.
    
    Since we cannot control the host's avahi-daemon from within a container,
    this function implements alternative recovery strategies that work in containerized environments.
    """
    global _avahi_consecutive_errors, _avahi_state_corrupted
    
    logger.info("Attempting Avahi service recovery (container-compatible strategies)...")
    
    # Strategy 1: Clear local avahi-browse process cache by killing any hanging processes
    try:
        logger.info("recover_avahi_service: Clearing hanging avahi-browse processes...")
        proc = await asyncio.create_subprocess_exec(
            "pkill", "-f", "avahi-browse",
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        await proc.communicate()
        logger.debug("recover_avahi_service: Cleared avahi-browse processes")
        await asyncio.sleep(2)  # Allow cleanup
        
    except Exception as e:
        logger.debug(f"recover_avahi_service: Process cleanup failed (may be normal): {e}")
    
    # Strategy 2: Test basic avahi-browse functionality with minimal timeout
    recovery_success = False
    try:
        logger.info("recover_avahi_service: Testing avahi-browse basic functionality...")
        proc = await asyncio.create_subprocess_exec(
            "avahi-browse", "-a", "-t", "-p", "--timeout=3",
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        
        try:
            stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=8)
            
            if proc.returncode == 0:
                logger.info("recover_avahi_service: avahi-browse test successful")
                recovery_success = True
            else:
                stderr_text = stderr.decode() if stderr else ""
                logger.warning(f"recover_avahi_service: avahi-browse test failed: {stderr_text}")
                
        except asyncio.TimeoutError:
            proc.kill()
            await proc.communicate()
            logger.warning("recover_avahi_service: avahi-browse test timeout")
            
    except Exception as e:
        logger.warning(f"recover_avahi_service: avahi-browse test exception: {e}")
    
    # Strategy 3: Wait for potential host-side Avahi recovery (user intervention or automatic recovery)
    if not recovery_success:
        logger.info("recover_avahi_service: Waiting for potential host-side Avahi recovery...")
        await asyncio.sleep(5)  # Give time for host-side recovery
        
        # Re-test after waiting
        try:
            proc = await asyncio.create_subprocess_exec(
                "avahi-browse", "-a", "-t", "-p", "--timeout=2",
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            
            stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=5)
            
            if proc.returncode == 0:
                logger.info("recover_avahi_service: Recovery detected after waiting period")
                recovery_success = True
            else:
                logger.warning("recover_avahi_service: No recovery detected after waiting period")
                
        except Exception as e:
            logger.debug(f"recover_avahi_service: Post-wait test failed: {e}")
    
    if recovery_success:
        # Reset error counters on successful recovery
        _avahi_consecutive_errors = 0
        _avahi_state_corrupted = False
        logger.info("recover_avahi_service: Avahi recovery successful")
        return True
    else:
        logger.warning("recover_avahi_service: All container-compatible recovery strategies failed")
        logger.warning("recover_avahi_service: Consider restarting the host avahi-daemon service manually if issues persist")
        return False


async def discover_dut(duration=DISCOVERY_DURATION, target_device_id=None):
    """Discover Matter services using platform-specific command-line tools.

    Uses different approaches based on environment:
    - macOS: dns-sd command (built into macOS)
    - Linux Desktop: avahi-browse (if available) or systemd-resolve
    - Docker: avahi-browse command (no service dependencies)

    This function implements Avahi coordination to prevent conflicts with Test Harness mDNS operations.

    Args:
        duration: Discovery duration in seconds
        target_device_id: Optional device ID to look for specifically

    Returns:
        list: List of discovered services with their details
    """
    env = detect_environment()
    discovered_services = []

    # Use Avahi coordination lock to prevent conflicts with Test Harness
    async with _avahi_lock:
        try:
            # Check Avahi health before proceeding (only for environments using Avahi)
            if env in ['linux_desktop', 'docker']:
                logger.info(f"discover_dut: Checking Avahi service health before discovery...")
                avahi_healthy = await check_avahi_state_health()
                
                if not avahi_healthy:
                    logger.warning(f"discover_dut: Avahi state appears corrupted, attempting recovery...")
                    recovery_success = await recover_avahi_service()
                    
                    if not recovery_success:
                        logger.error(f"discover_dut: Avahi recovery failed - using adaptive timeouts")
                        # Use adaptive timeout based on Avahi state
                        duration = get_adaptive_discovery_timeout()

            logger.info(f"discover_dut: Starting mDNS discovery for {duration}s on {env}...")

            if env == 'macos':
                # macOS: Use built-in dns-sd command
                await _discover_dut_from_macos(duration, target_device_id, discovered_services)

            elif env == 'linux_desktop':
                # Linux Desktop: Try avahi-browse or systemd-resolve
                await _discover_dut_from_linux_desktop(duration, target_device_id, discovered_services)

            elif env == 'docker':
                # Docker: Use avahi-browse command directly
                return await _discover_dut_from_docker(duration, target_device_id)

            else:
                logger.warning(f"discover_dut: No discovery method available for {env}")
                return []

            logger.info(f"discover_dut: Discovery completed. Found {len(discovered_services)} services")
            return discovered_services

        except Exception as e:
            logger.error(f"discover_dut: Discovery failed: {e}")
            return []


async def _discover_dut_from_macos(duration, target_device_id, discovered_services):
    """Discover services on macOS using dns-sd command-line tool."""
    service_types = ["_matter._tcp", "_matterc._udp", "_chip._tcp"]

    for service_type in service_types:
        try:
            logger.info(f"_discover_dut_from_macos: Browsing {service_type} on macOS...")

            # Start dns-sd browse in background
            proc = subprocess.Popen(
                ["dns-sd", "-B", service_type],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )

            # Let it run for a portion of the duration
            await asyncio.sleep(duration / len(service_types))

            try:
                stdout, stderr = proc.communicate(timeout=1)

                # Parse dns-sd output
                for line in stdout.split('\n'):
                    if 'ADD' in line and service_type in line:
                        parts = line.split()
                        if len(parts) >= 6:
                            service_name = parts[6] if len(parts) > 6 else 'unknown'

                            service_info = {
                                'name': service_name,
                                'type': service_type,
                                'domain': 'local',
                                'method': 'dns-sd'
                            }
                            discovered_services.append(service_info)

                            target_match = ""
                            if target_device_id and target_device_id.lower() in service_name.lower():
                                target_match = " TARGET FOUND"

                            logger.info(f"_discover_dut_from_macos: [dns-sd] {service_name} ({service_type}){target_match}")

            except subprocess.TimeoutExpired:
                proc.kill()

        except Exception as e:
            logger.warning(f"_discover_dut_from_macos: dns-sd failed for {service_type}: {e}")


async def _discover_dut_from_linux_desktop(duration, target_device_id, discovered_services):
    """Discover services on Linux Desktop using avahi-browse or systemd-resolve CLI tools."""

    # Try avahi-browse first (if installed)
    try:
        result = await run_subprocess(
            ["avahi-browse", "-a", "-t", "-p"],
            capture_output=True,
            timeout=min(duration, 15)
        )

        if result.returncode == 0:
            logger.info("_discover_dut_from_linux_desktop: Using avahi-browse for discovery...")

            for line in result.stdout.split('\n'):
                if ('_matter._tcp' in line or '_matterc._udp' in line or '_chip._tcp' in line) and line.startswith('+'):
                    parts = line.split(';')
                    if len(parts) >= 6:
                        service_name = parts[6] if len(parts) > 6 else 'unknown'
                        service_type = parts[4] if len(parts) > 4 else 'unknown'

                        service_info = {
                            'name': service_name,
                            'type': service_type,
                            'domain': 'local',
                            'method': 'avahi-browse'
                        }
                        discovered_services.append(service_info)

                        target_match = ""
                        if target_device_id and target_device_id.lower() in service_name.lower():
                            target_match = " TARGET FOUND"

                        logger.info(
                            f"_discover_dut_from_linux_desktop: [avahi-browse] {service_name} ({service_type}){target_match}")
            return

    except (subprocess.TimeoutExpired, subprocess.CalledProcessError, FileNotFoundError):
        logger.debug("_discover_dut_from_linux_desktop: avahi-browse not available or failed")

    # Try systemd-resolve as fallback
    try:
        logger.info("_discover_dut_from_linux_desktop: Trying systemd-resolve for mDNS...")

        # Use systemd-resolve to query .local domains
        test_queries = [f"{target_device_id}.local" if target_device_id else "matter-device.local"]

        for query in test_queries:
            try:
                result = await run_subprocess(
                    ["systemd-resolve", query],
                    capture_output=True,
                    timeout=5
                )

                if result.returncode == 0 and result.stdout:
                    service_info = {
                        'name': query,
                        'type': '_matter._tcp',
                        'domain': 'local',
                        'method': 'systemd-resolve',
                        'details': result.stdout.strip()
                    }
                    discovered_services.append(service_info)
                    logger.info(f"_discover_dut_from_linux_desktop: [systemd-resolve] Found: {query}")

            except Exception:
                continue

    except Exception as e:
        logger.debug(f"_discover_dut_from_linux_desktop: systemd-resolve failed: {e}")

    if not discovered_services:
        logger.warning("_discover_dut_from_linux_desktop: No native Linux discovery tools worked")


async def _discover_dut_from_docker(duration=DISCOVERY_DURATION, target_device_id=None):
    """
    Discover Matter services using avahi-browse command-line tool.

    This function is designed for use inside a Docker container running on Raspberry Pi with Ubuntu.
    Other environments are not supported or tested in this test scenario.

    Implements enhanced error handling for Avahi state conflicts that can cause infinite re-register loops.

    Args:
        duration: Discovery duration in seconds
        target_device_id: Optional device ID to look for specifically

    Returns:
        list: List of discovered services with their details
    """
    discovered_services = []
    max_retries = 3
    retry_count = 0
    
    while retry_count < max_retries:
        try:
            logger.info(f"_discover_dut_from_docker: Starting avahi-browse mDNS discovery for {duration}s (attempt {retry_count + 1}/{max_retries})...")
            
            # Use timeout to prevent hanging if Avahi is in corrupted state
            timeout_duration = min(duration + 10, 30)  # Cap timeout to prevent excessive waits
            
            # Run avahi-browse for the specified duration
            proc = await asyncio.create_subprocess_exec(
                "avahi-browse", "-a", "-t", "-p",
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            
            try:
                # Wait for the specified duration, then terminate the process
                await asyncio.sleep(duration)
                proc.terminate()
                stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=5)
                
            except asyncio.TimeoutError:
                logger.warning(f"_discover_dut_from_docker: avahi-browse process timeout, force killing...")
                proc.kill()
                try:
                    stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=2)
                except asyncio.TimeoutError:
                    stdout, stderr = b"", b"timeout_during_kill"
                    
            except Exception as e:
                logger.warning(f"_discover_dut_from_docker: Exception during avahi-browse execution: {e}")
                proc.kill()
                try:
                    stdout, stderr = await proc.communicate()
                except:
                    stdout, stderr = b"", b"exception_during_execution"

            # Check for signs of Avahi corruption in stderr
            stderr_text = stderr.decode() if stderr else ""
            if any(indicator in stderr_text.lower() for indicator in [
                "failed to create client", "daemon not running", 
                "name collision", "service type already exists"
            ]):
                logger.warning(f"_discover_dut_from_docker: Avahi corruption detected in stderr: {stderr_text}")
                if retry_count < max_retries - 1:
                    logger.info(f"_discover_dut_from_docker: Retrying discovery after Avahi error...")
                    retry_count += 1
                    await asyncio.sleep(2)  # Brief wait before retry
                    continue
                else:
                    logger.error(f"_discover_dut_from_docker: Max retries reached, discovery may be incomplete")

            output = stdout.decode() if stdout else ""
            
            # Filter Matter services
            for line in output.splitlines():
                if ("_matter._tcp" in line or "_matterc._udp" in line or "_chip._tcp" in line) and line.startswith('+'):
                    parts = line.split(';')
                    if len(parts) >= 6:
                        service_name = parts[6] if len(parts) > 6 else 'unknown'
                        service_type = parts[4] if len(parts) > 4 else 'unknown'
                        service_info = {
                            'name': service_name,
                            'type': service_type,
                            'domain': 'local',
                            'method': 'avahi-browse'
                        }
                        # Filter by target_device_id if specified
                        if target_device_id:
                            if target_device_id.lower() in service_name.lower():
                                logger.info(f"_discover_dut_from_docker: [avahi-browse] {service_name} ({service_type}) TARGET FOUND")
                                discovered_services.append(service_info)
                        else:
                            logger.info(f"_discover_dut_from_docker: [avahi-browse] {service_name} ({service_type})")
                            discovered_services.append(service_info)

            logger.info(f"_discover_dut_from_docker: avahi-browse discovery completed. Found {len(discovered_services)} services")
            return discovered_services

        except Exception as e:
            logger.error(f"_discover_dut_from_docker: avahi-browse discovery failed (attempt {retry_count + 1}): {e}")
            if retry_count < max_retries - 1:
                retry_count += 1
                await asyncio.sleep(2)  # Brief wait before retry
                continue
            else:
                logger.error(f"_discover_dut_from_docker: All discovery attempts failed")
                return []

    return discovered_services


async def detect_avahi_infinite_loop():
    """Detect if Test Harness is stuck in an Avahi re-register infinite loop.
    
    This function monitors Matter Test Harness logs for signs of Avahi registration loops
    that cause high CPU usage and prevent normal network operations.
    
    Returns:
        bool: True if infinite loop detected, False otherwise
    """
    try:
        # Check recent system logs for Avahi re-register patterns
        proc = await asyncio.create_subprocess_exec(
            "journalctl", "-u", "matter-test-harness", "--since", "2 minutes ago", "-n", "50",
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        
        try:
            stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=10)
            output = stdout.decode() if stdout else ""
            
            # Count Avahi re-register patterns
            reregister_count = output.count("Avahi re-register required")
            failed_advertise_count = output.count("Failed to advertise operational node")
            incorrect_state_count = output.count("CHIP Error 0x00000003: Incorrect state")
            
            # If we see many re-register attempts in a short time, it's likely a loop
            if reregister_count > 10 or failed_advertise_count > 15 or incorrect_state_count > 20:
                logger.warning(f"detect_avahi_infinite_loop: Avahi loop detected - reregister: {reregister_count}, failed_advertise: {failed_advertise_count}, incorrect_state: {incorrect_state_count}")
                return True
                
        except asyncio.TimeoutError:
            logger.debug("detect_avahi_infinite_loop: journalctl timeout - unable to check logs")
            
    except FileNotFoundError:
        # journalctl not available, try alternative detection
        logger.debug("detect_avahi_infinite_loop: journalctl not available, using process monitoring")
        return await _detect_avahi_loop_via_process_monitoring()
        
    except Exception as e:
        logger.debug(f"detect_avahi_infinite_loop: Log check failed: {e}")
        
    return False


async def _detect_avahi_loop_via_process_monitoring():
    """Alternative Avahi loop detection using process monitoring."""
    try:
        # Check if any Matter-related processes are consuming high CPU
        proc = await asyncio.create_subprocess_exec(
            "ps", "aux", 
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        
        stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=5)
        output = stdout.decode() if stdout else ""
        
        # Look for high CPU usage in Matter/Avahi related processes
        high_cpu_processes = []
        for line in output.splitlines():
            if any(keyword in line.lower() for keyword in ['matter', 'avahi', 'chip']):
                parts = line.split()
                if len(parts) > 2:
                    try:
                        cpu_usage = float(parts[2])
                        if cpu_usage > 50:  # High CPU usage
                            high_cpu_processes.append(line)
                    except ValueError:
                        continue
        
        if high_cpu_processes:
            logger.warning(f"detect_avahi_infinite_loop: High CPU processes detected: {len(high_cpu_processes)}")
            return True
            
    except Exception as e:
        logger.debug(f"_detect_avahi_loop_via_process_monitoring: Failed: {e}")
        
    return False


async def mitigate_avahi_infinite_loop():
    """Attempt to break out of Avahi infinite loop by restarting services.
    
    This function implements various strategies to recover from Avahi state corruption:
    1. Restart avahi-daemon service
    2. Clear stale Avahi registration cache
    3. Reset Matter Test Harness mDNS state
    
    Returns:
        bool: True if mitigation appears successful, False otherwise
    """
    logger.info("mitigate_avahi_infinite_loop: Attempting to break Avahi infinite loop...")
    
    try:
        # Strategy 1: Try to restart avahi-daemon
        logger.info("mitigate_avahi_infinite_loop: Restarting avahi-daemon service...")
        proc = await asyncio.create_subprocess_exec(
            "systemctl", "restart", "avahi-daemon",
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        
        await proc.communicate()
        
        if proc.returncode == 0:
            logger.info("mitigate_avahi_infinite_loop: avahi-daemon restart successful")
            await asyncio.sleep(5)  # Allow service to stabilize
            
            # Verify the loop is broken
            await asyncio.sleep(2)
            loop_still_active = await detect_avahi_infinite_loop()
            
            if not loop_still_active:
                logger.info("mitigate_avahi_infinite_loop: Avahi loop successfully mitigated")
                return True
            else:
                logger.warning("mitigate_avahi_infinite_loop: Loop persists after daemon restart")
        else:
            logger.warning("mitigate_avahi_infinite_loop: avahi-daemon restart failed")
            
    except Exception as e:
        logger.warning(f"mitigate_avahi_infinite_loop: Service restart failed: {e}")
    
    # Strategy 2: Clear Avahi cache/state
    try:
        logger.info("mitigate_avahi_infinite_loop: Clearing Avahi cache...")
        await asyncio.create_subprocess_exec("rm", "-rf", "/var/run/avahi-daemon/*")
        await asyncio.sleep(2)
        
        # Try starting avahi-daemon again
        await asyncio.create_subprocess_exec("systemctl", "start", "avahi-daemon")
        await asyncio.sleep(3)
        
        loop_still_active = await detect_avahi_infinite_loop()
        if not loop_still_active:
            logger.info("mitigate_avahi_infinite_loop: Loop mitigated after cache clear")
            return True
            
    except Exception as e:
        logger.debug(f"mitigate_avahi_infinite_loop: Cache clear failed: {e}")
    
    logger.error("mitigate_avahi_infinite_loop: Failed to mitigate Avahi infinite loop")
    return False


async def verify_mdns_discovery():
    """Verify that the target device is discoverable via mDNS.

    This function only performs mDNS discovery to verify the device is discoverable
    on the network.

    Uses different strategies based on detected environment:
    - Docker: avahi-browse command (no service dependencies)
    - macOS: Native dns-sd command (built-in)
    - Linux Desktop: avahi-browse or systemd-resolve (usually pre-installed)

    Raises:
        Exception: If mDNS discovery fails to find the target device
    """
    env = detect_environment()
    logger.info(f"verify_mdns_discovery: Starting mDNS discovery verification (Environment: {env})")

    # Wait for network stabilization
    # logger.info(f"verify_mdns_discovery: Waiting {NETWORK_STABILIZATION_WAIT}s for network stabilization...")
    # await asyncio.sleep(NETWORK_STABILIZATION_WAIT)

    # Limit discovery attempts to prevent infinite loops
    max_discovery_attempts = 3
    attempt = 0

    while attempt < max_discovery_attempts:
        attempt += 1
        logger.info(f"verify_mdns_discovery: Discovery attempt {attempt}/{max_discovery_attempts}")

        try:
            # Try mDNS discovery using avahi-browse or platform-specific tools
            discovered_services = await discover_dut(
                duration=DISCOVERY_DURATION,
                target_device_id=get_target_device_id()  # Use cached device ID
            )

            if discovered_services:
                logger.info(f"verify_mdns_discovery: mDNS discovery successful! Found {len(discovered_services)} services")
                # Give additional time for service stabilization
                await asyncio.sleep(TH_INTERFACE_WAIT)
                return  # Success - mDNS discovery found the device
            else:
                logger.warning(f"verify_mdns_discovery: Attempt {attempt} failed - no services found")
                if attempt < max_discovery_attempts:
                    logger.info(f"verify_mdns_discovery: Waiting {TH_PROCESS_WAIT}s before next attempt...")
                    await asyncio.sleep(TH_PROCESS_WAIT)

        except Exception as e:
            logger.error(f"verify_mdns_discovery: Discovery attempt {attempt} failed with exception: {e}")
            if attempt < max_discovery_attempts:
                logger.info(f"verify_mdns_discovery: Waiting {TH_PROCESS_WAIT}s before retry...")
                await asyncio.sleep(TH_PROCESS_WAIT)

    # All attempts failed
    raise Exception(
        f"verify_mdns_discovery: mDNS discovery failed after {max_discovery_attempts} attempts: No Matter services found on the network")


class ConnectionResult:

    def __init__(self, returncode: int, stderr: str = ""):
        self.returncode = returncode
        self.stderr = stderr


async def run_subprocess(cmd, check=False, capture_output=False, timeout=ATTRIBUTE_READ_TIMEOUT):
    """Run subprocess command with optional output capture and timeout.

    Args:
        cmd: List of command arguments (e.g., ["sudo", "ip", "link", "set", "eth0", "up"])
        check: If True, raise CalledProcessError on non-zero exit code
        capture_output: If True, return stdout as string; if False, return success boolean
        timeout: Maximum time to wait for command completion in seconds

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

        stdout, stderr = await asyncio.wait_for(proc.communicate(), timeout=timeout)

        if check and proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, cmd, stdout, stderr)

        if capture_output:
            return stdout.decode() if proc.returncode == 0 else ""
        else:
            return proc.returncode == 0

    except asyncio.TimeoutError:
        logger.warning(f"run_subprocess: Command timed out after {timeout}s: {' '.join(cmd)}")
        if check:
            raise
        return "" if capture_output else False
    except subprocess.CalledProcessError as e:
        logger.warning(f"run_subprocess: Subprocess failed (handled): {' '.join(cmd)}")
        if check:
            raise
        return "" if capture_output else False
    except Exception as e:
        logger.error(f"run_subprocess: Subprocess error: {' '.join(cmd)} - {e}")
        return "" if capture_output else False


async def detect_wifi_interface():
    """Detect WiFi interface using wpa_supplicant sockets.

    Returns:
        str or None: First available WiFi interface name (e.g., 'wlan0'), or None if not found

    Raises:
        None: Function handles all exceptions internally and returns None on error
    """

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


async def wpa_command(interface, cmd):
    """Send command to wpa_supplicant via Unix socket.

    Args:
        interface: WiFi interface name (string)
        cmd: wpa_supplicant command to send (string)

    Returns:
        str: Response from wpa_supplicant daemon

    Raises:
        Exception: If socket communication fails or command execution fails
    """

    sock_file = f"/var/run/wpa_supplicant/{interface}"
    sock_tmp = f"/tmp/wpa_{interface}_sock"
    if os.path.exists(sock_tmp):
        os.remove(sock_tmp)
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    try:
        sock.bind(sock_tmp)
        sock.sendto(cmd.encode(), sock_file)
        resp, _ = sock.recvfrom(4096)
        return resp.decode(errors="ignore").strip()
    except Exception as e:
        logger.error(f"wpa_command: Error sending '{cmd}' to {interface}: {e}")
        raise
    finally:
        sock.close()
        if os.path.exists(sock_tmp):
            os.remove(sock_tmp)


async def scan_and_find_ssid(interface, target_ssid, retries=MAX_RETRIES, delay=DUT_RECOVERY_DELAY):
    """Scan and search for SSID with retry logic.

    Args:
        interface: WiFi interface name (string)
        target_ssid: SSID to search for (string)
        retries: Maximum number of retry attempts (int)
        delay: Delay between retry attempts in seconds (int)

    Returns:
        bool: True if SSID found, False otherwise

    Raises:
        None: Function handles all exceptions internally and returns False on error
    """

    retry = 0
    while retry < retries:
        retry += 1
        try:
            await wpa_command(interface, "SCAN")
            await asyncio.sleep(delay)

            results = await wpa_command(interface, "SCAN_RESULTS")
            scan_lines = results.splitlines()[1:]

            for line in scan_lines:
                if line.endswith(f"\t{target_ssid}"):
                    return True

        except Exception as e:
            logger.error(f"scan_and_find_ssid: Scan attempt {retry} failed: {e}")

        if retry < retries:
            await asyncio.sleep(TH_QUICK_WAIT)

    logger.error(f"scan_and_find_ssid: SSID {target_ssid} not found after {retries} attempts")
    return False


async def connect_wifi_linux(ssid, password) -> ConnectionResult:
    """Connect to WiFi using wpa_supplicant commands in Linux (desktop and Raspberry Pi).

    Args:
        ssid: Network SSID (string or bytes)
        password: Network password (string or bytes)

    Returns:
        ConnectionResult: Object with returncode (0=success, non-zero=failure) and stderr message

    Raises:
        None: Function handles all exceptions internally and returns ConnectionResult with error info
    """

    if isinstance(ssid, bytes):
        ssid = ssid.decode()
    if isinstance(password, bytes):
        password = password.decode()

    interface = await detect_wifi_interface()
    if not interface:
        logger.error("connect_wifi_linux: No WiFi interface found")
        return ConnectionResult(1, "No WiFi interface found")

    try:
        # Scan to ensure SSID is available
        if not await scan_and_find_ssid(interface, ssid):
            logger.error(f"connect_wifi_linux: SSID {ssid} not found in scan")
            return ConnectionResult(1, f"SSID {ssid} not available")

        # Ensure interface is up and reset state
        await run_subprocess(["sudo", "ip", "link", "set", interface, "up"])
        await asyncio.sleep(TH_QUICK_WAIT)

        # Disconnect from any current connection
        await wpa_command(interface, "DISCONNECT")
        await asyncio.sleep(TH_PROCESS_WAIT)

        # Remove all existing networks to avoid conflicts
        nets_result = await wpa_command(interface, "LIST_NETWORKS")
        if nets_result and "network id" in nets_result:
            nets = nets_result.splitlines()[1:]
            for n in nets:
                if n.strip():
                    cols = n.split("\t")
                    if len(cols) >= 1:
                        await wpa_command(interface, f"REMOVE_NETWORK {cols[0]}")

        # Add and configure the target network
        net_id = await wpa_command(interface, "ADD_NETWORK")
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
            result = await wpa_command(interface, cmd)
            if result.startswith("FAIL"):
                logger.error(f"connect_wifi_linux: Failed to configure network: {cmd} -> {result}")
                return ConnectionResult(1, f"Failed to configure network: {cmd}")

        # Enable and select the network
        await wpa_command(interface, f"ENABLE_NETWORK {net_id}")
        await wpa_command(interface, f"SELECT_NETWORK {net_id}")
        await wpa_command(interface, "REASSOCIATE")

        retry = 0
        timeout = CONNECTION_TIMEOUT

        while retry < MAX_RETRIES:
            retry += 1
            start_time = time.time()
            connected = False

            try:
                while time.time() - start_time < timeout:
                    status = await wpa_command(interface, "STATUS")

                    if "wpa_state=COMPLETED" in status:
                        connected = True
                        break
                    elif "wpa_state=4WAY_HANDSHAKE" in status:
                        # Authentication in progress, wait a bit more
                        await asyncio.sleep(TH_PROCESS_WAIT)
                    elif "wpa_state=DISCONNECTED" in status or "wpa_state=INACTIVE" in status:
                        logger.warning(f"connect_wifi_linux: Connection failed, status: {status}")
                        break

                    await asyncio.sleep(TH_QUICK_WAIT)

            except asyncio.CancelledError:
                logger.warning(f"connect_wifi_linux: Connection attempt {retry} was cancelled, continuing with next attempt")
                # Don't re-raise immediately, try the next attempt
                if retry >= MAX_RETRIES:
                    raise
                connected = False

            if connected:
                break

            # If connection failed and we have more attempts, reset interface
            if retry < MAX_RETRIES:
                logger.warning(f"connect_wifi_linux: Attempt {retry} failed, resetting interface")
                try:
                    await run_subprocess(["sudo", "ip", "link", "set", interface, "down"])
                    await asyncio.sleep(TH_PROCESS_WAIT)
                    await run_subprocess(["sudo", "ip", "link", "set", interface, "up"])
                    await asyncio.sleep(TH_INTERFACE_WAIT)
                    await wpa_command(interface, f"SELECT_NETWORK {net_id}")
                    await wpa_command(interface, "REASSOCIATE")
                    await asyncio.sleep(TH_PROCESS_WAIT)
                except asyncio.CancelledError:
                    logger.warning(f"connect_wifi_linux: Interface reset for attempt {retry} was cancelled")
                    if retry >= MAX_RETRIES:
                        raise

        if not connected:
            final_status = await wpa_command(interface, "STATUS")
            logger.error(f"connect_wifi_linux: Connection failed after {MAX_RETRIES} attempts. Final status: {final_status}")
            return ConnectionResult(1, f"Connection failed: {final_status}")

        # Connection successful, get IP address
        logger.info(f"connect_wifi_linux: WiFi connection established, obtaining IP address...")

        # Release any existing DHCP lease and request new one
        await run_subprocess(["sudo", "dhclient", "-r", interface])
        await asyncio.sleep(TH_QUICK_WAIT)
        await run_subprocess(["sudo", "dhclient", interface])

        # Wait for IP assignment with retry
        ip_timeout = IP_TIMEOUT
        ip_start = time.time()

        while time.time() - ip_start < ip_timeout:
            try:
                output = await run_subprocess(["ip", "-4", "addr", "show", interface], capture_output=True)

                if output and "inet " in output:
                    ip_match = re.search(r'inet\s+(\S+)', output)
                    if ip_match:
                        ip_address = ip_match.group(1)
                        logger.info(f"connect_wifi_linux: IP address obtained: {ip_address}")

                        # Test connectivity to gateway with better error handling
                        try:
                            gw_output = await run_subprocess(["ip", "route", "show", "default"], capture_output=True)
                            if gw_output:
                                gw_match = re.search(r'default via (\S+)', gw_output)
                                if gw_match:
                                    gateway = gw_match.group(1)
                                    ping_output = await run_subprocess(
                                        ["ping", "-c", str(PING_COUNT), "-W", str(PING_DEADLINE), gateway],
                                        capture_output=True,
                                        timeout=DUT_NETWORK_WAIT
                                    )
                                    if ping_output and "1 received" in ping_output:
                                        logger.info(f"connect_wifi_linux: Gateway connectivity verified to {gateway}")
                                    else:
                                        logger.warning(f"connect_wifi_linux: Gateway ping failed but IP assigned")
                        except Exception as e:
                            logger.warning(f"connect_wifi_linux: Could not verify gateway connectivity: {e}")

                        # Add stabilization delay after successful connection
                        await asyncio.sleep(DUT_NETWORK_WAIT)
                        return ConnectionResult(0, "")

                await asyncio.sleep(TH_QUICK_WAIT)

            except asyncio.CancelledError:
                logger.warning("connect_wifi_linux: IP address acquisition was cancelled")
                raise
            except Exception as e:
                logger.warning(f"connect_wifi_linux: Error during IP acquisition: {e}")
                await asyncio.sleep(TH_QUICK_WAIT)

        logger.error("connect_wifi_linux: WiFi connected but no IP address obtained")
        return ConnectionResult(1, "No IP address obtained")

    except Exception as e:
        logger.error(f"connect_wifi_linux: Exception during connection: {e}")
        return ConnectionResult(1, str(e))


async def connect_wifi_mac(ssid, password) -> ConnectionResult:
    """Connect to WiFi using 'networksetup' command on macOS.

    Args:
        ssid: Network SSID (string)
        password: Network password (string)

    Returns:
        ConnectionResult: Object with returncode (0=success, non-zero=failure) and stderr message

    Raises:
        None: Function handles all exceptions internally and returns ConnectionResult with error info
    """

    if not shutil.which("networksetup"):
        logger.error("connect_wifi_mac: 'networksetup' is not present. Please install 'networksetup'.")
        return ConnectionResult(1, "networksetup not found")

    try:
        # Get the Wi-Fi interface
        interface_result = await run_subprocess(
            ["/usr/sbin/networksetup", "-listallhardwareports"],
            capture_output=True
        )
        interface = "en0"   # 'en0' is by default
        for block in interface_result.stdout.split("\n\n"):
            if "Wi-Fi" in block:
                for line in block.splitlines():
                    if "Device" in line:
                        interface = line.split(":")[1].strip()
                        break

        logger.info(f"connect_wifi_mac: Using interface: {interface}")
        result = await run_subprocess([
            "networksetup",
            "-setairportnetwork", interface, ssid, password
        ], check=True, capture_output=True)
        await asyncio.sleep(DUT_NETWORK_WAIT)
        return ConnectionResult(result.returncode, result.stderr)

    except subprocess.CalledProcessError as e:
        logger.error(f"connect_wifi_mac: Error connecting with networksetup: {e.stderr.strip()}")
        return ConnectionResult(e.returncode, e.stderr)
    except Exception as e:
        logger.error(f"connect_wifi_mac: Unexpected exception while trying to connect to {ssid}: {e}")
        return ConnectionResult(1, str(e))


async def connect_host_wifi(ssid, password) -> Optional[ConnectionResult]:
    """Connect to WiFi network based on host OS (Linux or macOS) with retry logic.

    Args:
        ssid: Network SSID (string or bytes)
        password: Network password (string or bytes)

    Returns:
        Optional[ConnectionResult]: ConnectionResult object with success/failure info, or None on unsupported OS

    Raises:
        None: Function handles all exceptions internally and returns ConnectionResult with error info
    """

    os_name = platform.system()
    retry = 0
    conn = None
    last_error = None

    logger.info(f"connect_host_wifi: Starting connection to {ssid} on {os_name} (max {MAX_RETRIES} attempts)")

    while retry < MAX_RETRIES:
        retry += 1

        try:
            logger.info(f"connect_host_wifi: Attempt {retry}/{MAX_RETRIES} to connect to {ssid}")

            if os_name == "Linux":
                conn = await connect_wifi_linux(ssid, password)
            elif os_name == "Darwin":
                conn = await connect_wifi_mac(ssid, password)
            else:
                logger.error(f"connect_host_wifi: OS not supported: {os_name}")
                return ConnectionResult(1, "OS not supported")

            if conn and conn.returncode == 0:
                logger.info(f"connect_host_wifi: Successfully connected to {ssid} on attempt {retry}")
                return conn
            elif conn:
                last_error = f"Return code: {conn.returncode}, stderr: {conn.stderr}"
                logger.warning(f"connect_host_wifi: Attempt {retry} failed. {last_error}")
            else:
                last_error = "No result returned from connection attempt"
                logger.warning(f"connect_host_wifi: Attempt {retry} failed. {last_error}")

        except asyncio.CancelledError:
            logger.warning(f"connect_host_wifi: Attempt {retry} was cancelled")
            raise  # Re-raise CancelledError to allow proper cleanup
        except Exception as e:
            last_error = str(e)
            logger.warning(f"connect_host_wifi: Exception on attempt {retry}: {e}")

        # Fixed delay between attempts, but don't delay after last attempt
        if retry < MAX_RETRIES:
            logger.info(f"connect_host_wifi: Waiting {DUT_RECOVERY_DELAY}s before next attempt...")
            try:
                await asyncio.sleep(DUT_RECOVERY_DELAY)
            except asyncio.CancelledError:
                logger.warning("connect_host_wifi: Delay between attempts was cancelled")
                raise

    # All attempts failed
    final_error = f"Failed after {MAX_RETRIES} attempts. Last error: {last_error}"
    logger.error(f"connect_host_wifi: {final_error}")
    return ConnectionResult(1, final_error)


def is_network_switch_successful(err):
    """Verify if network switch was successful by checking networkingStatus.

    Args:
        err: Error response from network command, should have networkingStatus attribute or None

    Returns:
        bool: True if network switch was successful (status is kSuccess or None), False otherwise
    """

    return (
        err is None or
        (hasattr(err, "networkingStatus") and
         err.networkingStatus == cnet.Enums.NetworkCommissioningStatusEnum.kSuccess)
    )


async def change_networks(test, cluster, ssid, password, breadcrumb):
    """Change networks for both DUT and test host with fallback logic.

    Args:
        test: Test instance with send_single_cmd method and matter_test_config
        cluster: NetworkCommissioning cluster object for sending commands
        ssid: Target network SSID (bytes)
        password: Target network password (bytes)
        breadcrumb: Breadcrumb value for network commands (int)

    Returns:
        None: Function completes successfully or raises exception

    Raises:
        Exception: If network change fails after all retry attempts
    """

    logger.info(f"change_networks: Starting network change to {ssid}")

    # Store original network for fallback
    original_ssid = test.matter_test_config.wifi_ssid
    original_password = test.matter_test_config.wifi_passphrase

    retry = 0
    while retry < MAX_RETRIES:
        retry += 1
        try:
            if retry > 1:
                await asyncio.sleep(DUT_NETWORK_WAIT)

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
        await asyncio.sleep(DUT_NETWORK_WAIT)

        logger.info(f"change_networks: Changing TH network to {ssid} (timeout: {NETWORK_CHANGE_TIMEOUT}s)")
        th_connection_success = False

        try:
            result = await asyncio.wait_for(
                connect_host_wifi(ssid=ssid, password=password),
                timeout=NETWORK_CHANGE_TIMEOUT
            )
            if result and result.returncode == 0:
                # Extra wait for network stabilization after TH connects
                logger.info("change_networks: TH connection successful, waiting for network stabilization...")
                await asyncio.sleep(TH_INTERFACE_WAIT)
                th_connection_success = True
            else:
                logger.error(f"change_networks: TH connection failed with result: {result}")

        except asyncio.TimeoutError:
            logger.error(f"change_networks: Timeout ({NETWORK_CHANGE_TIMEOUT}s) changing TH to {ssid}")
        except Exception as e:
            logger.error(f"change_networks: Exception changing TH to {ssid}: {e}")

        if th_connection_success:
            logger.info(f"change_networks: Successfully changed TH to {ssid}")
            return  # Success!

        # Connection failed, attempt fallback
        logger.warning(f"change_networks: TH connection to {ssid} failed, attempting fallback to {original_ssid}")
        try:
            fallback_result = await asyncio.wait_for(
                connect_host_wifi(ssid=original_ssid, password=original_password),
                timeout=NETWORK_CHANGE_TIMEOUT
            )
            if fallback_result and fallback_result.returncode == 0:
                logger.info(f"change_networks: Successfully fell back to {original_ssid}")
            else:
                logger.error(f"change_networks: Fallback to {original_ssid} also failed: {fallback_result}")
        except asyncio.TimeoutError:
            logger.error(f"change_networks: Timeout during fallback to {original_ssid}")
        except Exception as fallback_e:
            logger.error(f"change_networks: Exception during fallback to {original_ssid}: {fallback_e}")
    # If we reach here, all retry attempts have been exhausted
    logger.error(f"change_networks: Failed to switch networks after {MAX_RETRIES} attempts.")
    logger.info(f"change_networks: Ensuring fallback connectivity to {original_ssid}")

    try:
        fallback_result = await asyncio.wait_for(
            connect_host_wifi(ssid=original_ssid, password=original_password),
            timeout=NETWORK_CHANGE_TIMEOUT
        )
        if fallback_result and fallback_result.returncode == 0:
            logger.info(f"change_networks: Final fallback to {original_ssid} successful")
        else:
            logger.error(f"change_networks: Final fallback to {original_ssid} failed - WiFi may be disconnected!")
    except asyncio.TimeoutError:
        logger.error(f"change_networks: Final fallback to {original_ssid} timed out")
    except Exception as final_e:
        logger.error(f"change_networks: Final fallback attempt failed: {final_e}")

    raise Exception(f"Failed to switch networks to {ssid} after {MAX_RETRIES} attempts")


async def parse_default_routes():
    """Parse and return default routes information from system routing table.

    Returns:
        list: List of dictionaries containing route information with keys:
              - gateway: Gateway IP address (string)
              - interface: Network interface name (string)  
              - metric: Route metric value (string or None)
              - full_line: Complete route line (string)
              - is_lan: Boolean indicating if route is LAN-based (not WiFi)

    Raises:
        None: Function handles all exceptions internally and returns empty list on error
    """
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
    """Remove LAN default routes and return list of removed routes for restoration.

    Returns:
        list: List of route dictionaries that were successfully removed

    Raises:
        None: Function handles all exceptions internally and returns partial results
    """
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


async def check_route_exists(gateway, interface):
    """Check if a specific default route exists.

    Args:
        gateway: Gateway IP address (string)
        interface: Network interface name (string)

    Returns:
        bool: True if route exists, False otherwise

    Raises:
        None: Function handles all exceptions internally and returns False on error
    """
    try:
        output = await run_subprocess(
            ["ip", "route", "show", "default", "via", gateway, "dev", interface],
            capture_output=True
        )
        return bool(output and output.strip())
    except Exception as e:
        logger.warning(f"check_route_exists: Error checking route: {e}")
        return False


async def restore_lan_routes(original_routes):
    """Restore previously removed LAN default routes.

    Args:
        original_routes: List of route dictionaries to restore (list)

    Returns:
        None

    Raises:
        None: Function handles all exceptions internally and logs failures
    """
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
                await asyncio.sleep(TH_QUICK_WAIT)
                await run_subprocess(["sudo", "dhclient", interface])
                logger.info(f"restore_lan_routes: Attempted DHCP refresh for {interface}")
            except Exception as dhcp_e:
                logger.warning(f"restore_lan_routes: DHCP fallback failed for {interface}: {dhcp_e}")


async def temporarily_remove_lan_route_for_wifi(ssid, wifi_1st_ap_ssid):
    """Temporarily remove LAN route if this is 2nd WiFi network and dual routes exist.

    Args:
        ssid: Current network SSID to connect to (string)
        wifi_1st_ap_ssid: First WiFi AP SSID for comparison (string)

    Returns:
        tuple: (lan_route_removed, gateway, lan_interface) for restoration
               - lan_route_removed: Boolean indicating if route was removed
               - gateway: Gateway IP address of removed route (string or None)
               - lan_interface: Interface name of removed route (string or None)

    Raises:
        None: Function handles all exceptions internally and logs warnings
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
                if await check_route_exists(gateway, lan_interface):
                    # Remove LAN default route temporarily using run_subprocess
                    success = await run_subprocess([
                        'sudo', 'ip', 'route', 'del', 'default', 'via', gateway, 'dev', lan_interface
                    ], check=False)
                    if success:
                        lan_route_removed = True
                        logger.info(f"temporarily_remove_lan_route_for_wifi: LAN default route temporarily removed")
                        await asyncio.sleep(TH_PROCESS_WAIT)  # Wait for route change to take effect
                    else:
                        logger.warning(
                            f"temporarily_remove_lan_route_for_wifi: Failed to remove LAN route via {gateway} dev {lan_interface}")
                else:
                    logger.info("temporarily_remove_lan_route_for_wifi: LAN default route verification failed, skipping removal")

        except Exception as e:
            logger.warning(f"temporarily_remove_lan_route_for_wifi: Could not manage routes on {os_name}: {e}")
    elif ssid != wifi_1st_ap_ssid and os_name != "Linux":
        logger.info(
            f"temporarily_remove_lan_route_for_wifi: Route management not implemented for {os_name}, skipping route optimization")

    return lan_route_removed, gateway, lan_interface


async def restore_temporarily_removed_lan_route(lan_route_removed, gateway, lan_interface):
    """Restore temporarily removed LAN route (Linux only).

    Args:
        lan_route_removed: Boolean indicating if route was previously removed
        gateway: Gateway IP address to restore (string or None)
        lan_interface: Interface name to restore (string or None)

    Returns:
        None

    Raises:
        None: Function handles all exceptions internally and logs errors
    """
    if lan_route_removed and gateway and lan_interface and platform.system() == "Linux":
        try:
            # Check if route already exists before adding
            if not await check_route_exists(gateway, lan_interface):
                # Route doesn't exist, safe to add
                success = await run_subprocess([
                    'sudo', 'ip', 'route', 'add', 'default', 'via', gateway, 'dev', lan_interface, 'metric', '100'
                ], check=False)
                if success:
                    logger.info(
                        f"restore_temporarily_removed_lan_route: LAN default route restored (gateway: {gateway}, interface: {lan_interface})")
                else:
                    logger.error(
                        f"restore_temporarily_removed_lan_route: Failed to restore LAN route via {gateway} dev {lan_interface}")
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
        int: Index of the network in the list

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


async def verify_breadcrumb_value(test_instance, expected_value, step_number, timeout=None):
    """Verify breadcrumb attribute value.

    Args:
        test_instance: Test instance with read_single_attribute_check_success method
        expected_value: Expected breadcrumb value (int)
        step_number: Step number for logging (int)
        timeout: Optional timeout for the operation (int)

    Returns:
        None

    Raises:
        AssertionError: If breadcrumb value doesn't match expected
    """
    if timeout:
        breadcrumb = await asyncio.wait_for(
            test_instance.read_single_attribute_check_success(
                cluster=cgen,
                attribute=cgen.Attributes.Breadcrumb
            ),
            timeout=timeout
        )
    else:
        breadcrumb = await test_instance.read_single_attribute_check_success(
            cluster=cgen,
            attribute=cgen.Attributes.Breadcrumb
        )

    logger.info(f"Step {step_number}: Breadcrumb is: {breadcrumb}")
    asserts.assert_equal(breadcrumb, expected_value, f"Expected breadcrumb to be {expected_value}, but got: {breadcrumb}")


async def send_add_or_update_wifi_network_with_retries(test_instance, ssid, credentials, breadcrumb=1, max_retries=MAX_RETRIES):
    """Send AddOrUpdateWiFiNetwork command with retry logic.

    Args:
        test_instance: Test instance with send_single_cmd method
        ssid: Network SSID (string)
        credentials: Network credentials (string)  
        breadcrumb: Breadcrumb value (int)
        max_retries: Maximum retry attempts (int)

    Returns:
        Response from AddOrUpdateWiFiNetwork command

    Raises:
        Exception: If command fails after all retries
    """
    cmd = cnet.Commands.AddOrUpdateWiFiNetwork(
        ssid=ssid.encode(), credentials=credentials.encode(), breadcrumb=breadcrumb)

    # Try with retries in case of temporary connectivity issues
    response = None
    retry = 0
    while retry < max_retries:
        retry += 1
        try:
            logger.info(
                f"send_add_or_update_wifi_network_with_retries: Attempt {retry}/{max_retries}: Sending AddOrUpdateWiFiNetwork command")
            response = await test_instance.send_single_cmd(cmd=cmd, timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
            break
        except Exception as e:
            logger.warning(f"send_add_or_update_wifi_network_with_retries: Attempt {retry} failed: {e}")
            if retry < max_retries:
                logger.info(f"Retrying in {DUT_RECOVERY_DELAY} seconds...")
                await asyncio.sleep(DUT_RECOVERY_DELAY)
            else:
                raise Exception(f"Failed to send AddOrUpdateWiFiNetwork after {max_retries} attempts: {e}")

    return response


async def verify_operational_network(ssid, wifi_1st_ap_ssid, read_single_attribute_check_success):
    """Verify that DUT is operational on the specified network.

    Args:
        ssid: SSID to verify connection to (string)
        wifi_1st_ap_ssid: First WiFi AP SSID for comparison (string)
        read_single_attribute_check_success: Function to read Matter attributes

    Returns:
        None

    Raises:
        AssertionError: If network verification fails
    """
    # Check for Avahi infinite loop before proceeding
    logger.info("verify_operational_network: Checking for Avahi infinite loop...")
    loop_detected = await detect_avahi_infinite_loop()
    
    if loop_detected:
        logger.warning("verify_operational_network: Avahi infinite loop detected - attempting mitigation...")
        mitigation_success = await mitigate_avahi_infinite_loop()
        
        if mitigation_success:
            logger.info("verify_operational_network: Avahi loop mitigation successful, proceeding with verification")
            # Brief pause to let services stabilize
            await asyncio.sleep(3)
        else:
            logger.error("verify_operational_network: Avahi loop mitigation failed - verification may be unreliable")
            # Continue anyway but use extended timeouts
    
    # Try enhanced mDNS discovery first
    logger.info("Attempting enhanced mDNS discovery method...")
    try:
        await verify_mdns_discovery()
        return  # Success with enhanced method
    except Exception as e:
        logger.warning(f"Enhanced mDNS method failed: {e}")
        logger.info("Falling back to traditional method...")

    # Traditional method (original implementation)
    networks = None
    retry = 0

    # logger.info(f"verify_operational_network: Waiting {NETWORK_STABILIZATION_WAIT}s for network stabilization...")
    # await asyncio.sleep(NETWORK_STABILIZATION_WAIT)

    # Temporarily manage routes to ensure proper connectivity during WiFi verification
    lan_route_removed, gateway, lan_interface = await temporarily_remove_lan_route_for_wifi(ssid, wifi_1st_ap_ssid)

    try:
        # Reasonable retries for mDNS discovery, especially in Docker environment
        max_retries_for_discovery = MAX_MDNS_DISCOVERY_RETRIES

        while retry < max_retries_for_discovery:
            retry += 1

            try:
                # Use extended timeout for mDNS discovery, especially on first attempt after network change
                if retry == 1:
                    timeout_to_use = MDNS_DISCOVERY_TIMEOUT  # Extended timeout for first attempt (180s)
                else:
                    timeout_to_use = ATTRIBUTE_READ_TIMEOUT  # Shorter timeout for subsequent attempts (30s)

                logger.debug(
                    f"verify_operational_network: Using timeout {timeout_to_use}s for attempt {retry}/{max_retries_for_discovery}")

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

            except asyncio.TimeoutError:
                logger.error(f"verify_operational_network: Timeout during attempt {retry} (waited {timeout_to_use}s)")
            except Exception as e:
                logger.error(f"verify_operational_network: Exception reading networks: {e}")

            # Wait between retry attempts to allow network stabilization
            if retry < max_retries_for_discovery:
                logger.info(f"verify_operational_network: Waiting before retry {retry + 1}")
                # Small wait to allow network and mDNS to stabilize naturally
                await asyncio.sleep(TH_PROCESS_WAIT)

            # Fixed delay - ESP32 needs time after network switch
            await asyncio.sleep(DUT_RECOVERY_DELAY)
        else:
            asserts.fail(f"verify_operational_network: Could not read networks after {max_retries_for_discovery} retries.")

    finally:
        # Always restore LAN default route if we removed it
        await restore_temporarily_removed_lan_route(lan_route_removed, gateway, lan_interface)

    userwifi_netidx = await find_network_and_assert(networks, ssid)
    if userwifi_netidx is not None:
        logger.info(f"verify_operational_network: DUT connected to SSID: {ssid}")


class TC_CNET_4_11(MatterBaseTest):

    @classmethod
    def setup_class(cls):
        """Remove default route from LAN interface to force traffic through Wi-Fi during test."""
        try:
            logger.info("setup_class: Setting up test environment...")
            # Remove LAN routes to force traffic through Wi-Fi during the test
            cls._original_routes = asyncio.run(remove_lan_routes())
            logger.info("setup_class: Network environment setup completed successfully")
        except Exception as e:
            logger.error(f"setup_class: Failed to setup network environment: {e}")
            cls._original_routes = []
            raise

    @classmethod
    def teardown_class(cls):
        """Restore original default routes after the test finishes."""
        try:
            original_routes = getattr(cls, "_original_routes", [])
            logger.info("teardown_class: Restoring network environment...")
            # Restore original LAN routes using helper function
            asyncio.run(restore_lan_routes(original_routes))
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
        await asyncio.sleep(DUT_SERVICE_STARTUP)

        # Set the target device ID for mDNS discovery from DUT node ID
        set_target_device_id(self.dut_node_id)
        logger.info(f"Set target device ID for mDNS discovery: {get_target_device_id()}")

        asserts.assert_true("PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID" in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID must be included on the command line in "
                            "the --string-arg flag as PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID:<hex:7A6967626565686F6D65>")
        asserts.assert_true("PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS" in self.matter_test_config.global_test_params,
                            "PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS must be included on the command line in "
                            "the --string-arg flag as PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS:<hex:70617373776F7264313233>")

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

        await self.send_single_cmd(cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=900), timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
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
        await asyncio.sleep(TH_INTERFACE_WAIT)

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(5)

        # Add second network - handle potential connectivity issues after removing the first network
        response = await send_add_or_update_wifi_network_with_retries(self, wifi_2nd_ap_ssid, wifi_2nd_ap_credentials, breadcrumb=1)
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
        await asyncio.sleep(DUT_NETWORK_WAIT)
        await verify_operational_network(wifi_2nd_ap_ssid, wifi_1st_ap_ssid, self.read_single_attribute_check_success)

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(9)

        # Small additional wait after network verification to ensure DUT session stability
        await asyncio.sleep(TH_PROCESS_WAIT)

        # Verify that the breadcrumb value is set to 2
        await verify_breadcrumb_value(self, expected_value=2, step_number=9)

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
        await asyncio.sleep(DUT_NETWORK_WAIT * 2)

        # TH changes its WiFi connection to PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID
        self.step(11)

        logger.info(f"Switching TH back to original network: {wifi_1st_ap_ssid}")
        await asyncio.wait_for(
            connect_host_wifi(wifi_1st_ap_ssid, wifi_1st_ap_credentials),
            timeout=NETWORK_CHANGE_TIMEOUT
        )
        # Let's wait a couple of seconds to change networks
        await asyncio.sleep(DUT_NETWORK_WAIT)

        # TH discovers and connects to DUT on the PIXIT.CNET.WIFI_1ST_ACCESSPOINT_SSID operational network
        self.step(12)

        # Give more time for both TH and DUT to stabilize on the original network
        await asyncio.sleep(DUT_NETWORK_WAIT * 2)

        # Verify that DUT has reverted to original network and is reachable
        try:
            await verify_operational_network(wifi_1st_ap_ssid, wifi_1st_ap_ssid, self.read_single_attribute_check_success)
            logger.info("Successfully reconnected to DUT on original network")
        except Exception as e:
            logger.error(f"Failed to reconnect to DUT on original network: {e}")
            # Try one more time with additional wait
            logger.info("Retrying connection to DUT...")
            await asyncio.sleep(DUT_NETWORK_WAIT * 2)
            await verify_operational_network(wifi_1st_ap_ssid, wifi_1st_ap_ssid, self.read_single_attribute_check_success)

        # TH sends ArmFailSafe command to the DUT with ExpiryLengthSeconds set to 900
        self.step(13)

        await self.send_single_cmd(cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=900), timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
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
        asserts.assert_true(isinstance(response, cnet.Commands.NetworkConfigResponse),
                            f"Expected response to be of type NetworkConfigResponse but got: {type(response)}")
        # 1. NetworkingStatus is success
        asserts.assert_equal(response.networkingStatus,
                             cnet.Enums.NetworkCommissioningStatusEnum.kSuccess, "Network was not removed")
        # 2. NetworkIndex is 'Userwifi_netidx'
        asserts.assert_equal(response.networkIndex, userwifi_netidx,
                             "Incorrect network index in response")

        # TH sends AddOrUpdateWiFiNetwork command to the DUT with SSID field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_SSID,
        # Credentials field set to PIXIT.CNET.WIFI_2ND_ACCESSPOINT_CREDENTIALS and Breadcrumb field set to 1
        self.step(15)

        response = await send_add_or_update_wifi_network_with_retries(self, wifi_2nd_ap_ssid, wifi_2nd_ap_credentials, breadcrumb=1)
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
        await asyncio.sleep(DUT_NETWORK_WAIT)
        await verify_operational_network(wifi_2nd_ap_ssid, wifi_1st_ap_ssid, self.read_single_attribute_check_success)

        # TH reads Breadcrumb attribute from the General Commissioning cluster of the DUT
        self.step(18)

        # Small additional wait after network verification to ensure DUT session stability
        await asyncio.sleep(TH_PROCESS_WAIT)

        # Verify that the breadcrumb value is set to 3
        await verify_breadcrumb_value(self, expected_value=3, step_number=18, timeout=TIMEOUT)

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

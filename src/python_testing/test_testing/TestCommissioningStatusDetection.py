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

"""
Unit tests for commissioning status detection functions in commissioning.py.

Tests cover:
- DNS-SD discovery (_is_device_operational_via_dnssd)
- Parallel session establishment (_establish_pase_or_case_session)
- is_commissioned() integration scenarios
- get_commissioned_fabric_count() integration scenarios

Scenarios include factory fresh devices, devices commissioned on the current fabric,
and devices commissioned on other fabrics.
"""

import asyncio
import sys
from dataclasses import dataclass
from pathlib import Path
from unittest.mock import AsyncMock, MagicMock, patch

# Add the python_testing directory to path so mdns_discovery module can be found
PYTHON_TESTING_DIR = Path(__file__).parent.parent
if str(PYTHON_TESTING_DIR) not in sys.path:
    sys.path.insert(0, str(PYTHON_TESTING_DIR))

# Test constants
TEST_NODE_ID = 1234
TEST_COMPRESSED_FABRIC_ID = 0x1234567890ABCDEF
TEST_ENDPOINT = 0
TEST_DISCRIMINATOR = 3840
TEST_PASSCODE = 20202021


@dataclass
class MockMdnsServiceInfo:
    """Mock MdnsServiceInfo for testing DNS-SD discovery."""
    instance_name: str


class MockDeviceController:
    """Mock ChipDeviceController for testing."""

    def __init__(self, compressed_fabric_id: int = TEST_COMPRESSED_FABRIC_ID):
        self._compressed_fabric_id = compressed_fabric_id
        # Mock methods that will be configured per-test
        self.EstablishPASESession = AsyncMock()
        self.GetConnectedDevice = AsyncMock()
        self.ReadAttribute = AsyncMock()
        self.CreateManualCode = MagicMock(return_value="MT:YNJV7VSC00KA0648G00")
        self.FindOrEstablishPASESession = AsyncMock()

    def GetCompressedFabricId(self) -> int:
        return self._compressed_fabric_id


def build_expected_instance_name(compressed_fabric_id: int, node_id: int) -> str:
    """Build the expected DNS-SD instance name for a device on a fabric."""
    return f'{compressed_fabric_id:016X}-{node_id:016X}'


def create_mock_attribute_result(endpoint: int, root_cert_count: int):
    """Create a mock ReadAttribute result with specified number of root certificates."""
    # Import here to avoid issues if module not available
    try:
        import matter.clusters as Clusters
        mock_certs = [b'cert_data'] * root_cert_count
        return {
            endpoint: {
                Clusters.OperationalCredentials: {
                    Clusters.OperationalCredentials.Attributes.TrustedRootCertificates: mock_certs
                }
            }
        }
    except ImportError:
        # Fallback for environments without the full Matter SDK
        mock_certs = [b'cert_data'] * root_cert_count
        mock_opcreds = MagicMock()
        mock_opcreds_attrs = MagicMock()
        mock_opcreds_attrs.TrustedRootCertificates = mock_certs
        return {
            endpoint: {
                mock_opcreds: {
                    mock_opcreds_attrs: mock_certs
                }
            }
        }


# =============================================================================
# CATEGORY A: DNS-SD Discovery Tests
# =============================================================================

async def test_dnssd_device_found_on_this_fabric():
    """
    Test: Device found operational on THIS fabric via DNS-SD.

    Scenario: Device is commissioned to this fabric and advertising.
    Expected: Returns True - device is operational on our fabric.

    Value: This is the fast path. When DNS-SD finds the device, we know
    immediately it's commissioned to this fabric without needing CASE/PASE.
    """
    from matter.testing.commissioning import _is_device_operational_via_dnssd

    mock_controller = MockDeviceController()
    expected_instance_name = build_expected_instance_name(TEST_COMPRESSED_FABRIC_ID, TEST_NODE_ID)

    with patch('mdns_discovery.mdns_discovery.MdnsDiscovery') as MockMdnsDiscovery:
        mock_mdns = MagicMock()
        mock_mdns.get_operational_services = AsyncMock(return_value=[
            MockMdnsServiceInfo(instance_name=expected_instance_name)
        ])
        MockMdnsDiscovery.return_value = mock_mdns

        result = await _is_device_operational_via_dnssd(mock_controller, TEST_NODE_ID)

        if not result:
            return "Expected True when device is operational on this fabric"
    return None


async def test_dnssd_device_on_different_fabric():
    """
    Test: Device advertising on different fabric (Scenario 3).

    Scenario: Device is commissioned to another fabric but not ours.
    Expected: Returns False - device not operational on OUR fabric.

    Value: Verifies we don't falsely detect devices on other fabrics as ours.
    The fabric ID in the instance name must match our compressed fabric ID.
    """
    from matter.testing.commissioning import _is_device_operational_via_dnssd

    mock_controller = MockDeviceController()

    with patch('mdns_discovery.mdns_discovery.MdnsDiscovery') as MockMdnsDiscovery:
        mock_mdns = MagicMock()
        # Device advertising on different fabric (different compressed fabric ID)
        mock_mdns.get_operational_services = AsyncMock(return_value=[
            MockMdnsServiceInfo(instance_name="AAAAAAAAAAAAAAAA-00000000000004D2")  # Different fabric, same node
        ])
        MockMdnsDiscovery.return_value = mock_mdns

        result = await _is_device_operational_via_dnssd(mock_controller, TEST_NODE_ID)

        if result:
            return "Expected False when device is on different fabric"
    return None


async def test_dnssd_no_services_found():
    """
    Test: No operational services found via DNS-SD.

    Scenario: Factory fresh device or device not advertising.
    Expected: Returns False - no operational advertisements found.

    Value: Verifies correct behavior when DNS-SD finds nothing,
    which triggers the parallel PASE/CASE fallback.
    """
    from matter.testing.commissioning import _is_device_operational_via_dnssd

    mock_controller = MockDeviceController()

    with patch('mdns_discovery.mdns_discovery.MdnsDiscovery') as MockMdnsDiscovery:
        mock_mdns = MagicMock()
        mock_mdns.get_operational_services = AsyncMock(return_value=[])
        MockMdnsDiscovery.return_value = mock_mdns

        result = await _is_device_operational_via_dnssd(mock_controller, TEST_NODE_ID)

        if result:
            return "Expected False when no operational services found"
    return None


async def test_dnssd_exception_handling():
    """
    Test: DNS-SD discovery raises exception.

    Scenario: Network error, timeout, or other DNS-SD failure.
    Expected: Returns False gracefully (triggers fallback to PASE/CASE).

    Value: Ensures DNS-SD failures don't crash the detection flow.
    The parallel PASE/CASE approach will handle the actual connection.
    """
    from matter.testing.commissioning import _is_device_operational_via_dnssd

    mock_controller = MockDeviceController()

    with patch('mdns_discovery.mdns_discovery.MdnsDiscovery') as MockMdnsDiscovery:
        mock_mdns = MagicMock()
        mock_mdns.get_operational_services = AsyncMock(side_effect=Exception("DNS-SD network error"))
        MockMdnsDiscovery.return_value = mock_mdns

        result = await _is_device_operational_via_dnssd(mock_controller, TEST_NODE_ID)

        if result:
            return "Expected False when DNS-SD raises exception"
    return None


async def test_dnssd_import_error_handling():
    """
    Test: mdns_discovery module not available.

    Scenario: Running in environment without mdns_discovery installed.
    Expected: Returns False gracefully (falls back to PASE/CASE).

    Value: Ensures the code works even if optional DNS-SD module is missing.
    """
    from matter.testing.commissioning import _is_device_operational_via_dnssd

    mock_controller = MockDeviceController()

    # Simulate ImportError by patching the import inside the function
    with patch.dict('sys.modules', {'mdns_discovery': None, 'mdns_discovery.mdns_discovery': None}):
        # The function catches ImportError and returns False
        result = await _is_device_operational_via_dnssd(mock_controller, TEST_NODE_ID)
        if result:
            return "Expected False when mdns_discovery module not available"
    return None


async def test_dnssd_multiple_services_finds_correct_one():
    """
    Test: Multiple services found, one matches our fabric+node.

    Scenario: Network has multiple Matter devices, we find ours.
    Expected: Returns True when our specific device is in the list.

    Value: Verifies correct filtering when multiple devices advertise.
    """
    from matter.testing.commissioning import _is_device_operational_via_dnssd

    mock_controller = MockDeviceController()
    expected_instance_name = build_expected_instance_name(TEST_COMPRESSED_FABRIC_ID, TEST_NODE_ID)

    with patch('mdns_discovery.mdns_discovery.MdnsDiscovery') as MockMdnsDiscovery:
        mock_mdns = MagicMock()
        mock_mdns.get_operational_services = AsyncMock(return_value=[
            MockMdnsServiceInfo(instance_name="OTHERFABRICID00000-0000000000005678"),
            MockMdnsServiceInfo(instance_name=expected_instance_name),  # Our device
            MockMdnsServiceInfo(instance_name="ANOTHERFABRIC0000-0000000000009999"),
        ])
        MockMdnsDiscovery.return_value = mock_mdns

        result = await _is_device_operational_via_dnssd(mock_controller, TEST_NODE_ID)

        if not result:
            return "Expected True when this fabric's service is in the list"
    return None


async def test_dnssd_same_fabric_different_node():
    """
    Test: Service for same fabric but different node ID.

    Scenario: Another device on our fabric, but not the one we're looking for.
    Expected: Returns False - wrong node ID.

    Value: Verifies both fabric AND node ID must match.
    """
    from matter.testing.commissioning import _is_device_operational_via_dnssd

    mock_controller = MockDeviceController()
    # Same fabric, different node
    different_node_instance = build_expected_instance_name(TEST_COMPRESSED_FABRIC_ID, 9999)

    with patch('mdns_discovery.mdns_discovery.MdnsDiscovery') as MockMdnsDiscovery:
        mock_mdns = MagicMock()
        mock_mdns.get_operational_services = AsyncMock(return_value=[
            MockMdnsServiceInfo(instance_name=different_node_instance)
        ])
        MockMdnsDiscovery.return_value = mock_mdns

        result = await _is_device_operational_via_dnssd(mock_controller, TEST_NODE_ID)

        if result:
            return "Expected False when service is for different node ID"
    return None


# =============================================================================
# CATEGORY B: Parallel Session Establishment Tests
# =============================================================================

async def test_parallel_session_pase_wins():
    """
    Test: PASE completes before CASE in parallel attempt.

    Scenario: Factory fresh device - PASE succeeds, CASE fails.
    Expected: Session established via PASE, CASE cancelled.

    Value: Verifies PASE path works for uncommissioned devices.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    # PASE succeeds immediately
    async def pase_success(*args, **kwargs):
        return None

    # CASE fails after a delay (simulating real network behavior)
    async def case_fail_slow(*args, **kwargs):
        await asyncio.sleep(0.1)  # Delay so PASE wins the race
        raise Exception("CASE failed - no fabric")

    mock_controller.EstablishPASESession = pase_success
    mock_controller.GetConnectedDevice = case_fail_slow

    pase_params = {'discriminator': TEST_DISCRIMINATOR, 'passcode': TEST_PASSCODE}

    try:
        await commissioning._establish_pase_or_case_session(mock_controller, TEST_NODE_ID, pase_params)
        # Should succeed via PASE
    except Exception as e:
        return f"Should have succeeded via PASE: {e}"

    return None


async def test_parallel_session_case_wins():
    """
    Test: CASE completes before PASE in parallel attempt.

    Scenario: Device commissioned to this fabric, CASE faster than PASE.
    Expected: Session established via CASE, PASE cancelled.

    Value: Verifies CASE path works when device is on our fabric.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    # PASE is slow (will be cancelled)
    async def pase_slow(*args, **kwargs):
        await asyncio.sleep(10)  # Will be cancelled when CASE succeeds
        return

    # CASE succeeds immediately
    async def case_success(*args, **kwargs):
        return MagicMock()

    mock_controller.EstablishPASESession = pase_slow
    mock_controller.GetConnectedDevice = case_success

    pase_params = {'discriminator': TEST_DISCRIMINATOR, 'passcode': TEST_PASSCODE}

    try:
        await commissioning._establish_pase_or_case_session(mock_controller, TEST_NODE_ID, pase_params)
        # Should succeed via CASE
    except Exception as e:
        return f"Should have succeeded via CASE: {e}"

    return None


async def test_parallel_session_first_fails_second_succeeds():
    """
    Test: First connection attempt fails, second succeeds.

    Scenario: CASE fails quickly, PASE succeeds after.
    Expected: Session established via PASE after CASE failure.

    Value: Verifies fallback to second attempt when first fails.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    # Track call order
    call_order = []

    async def pase_success(*args, **kwargs):
        call_order.append('pase')
        await asyncio.sleep(0.1)  # Slight delay
        return

    async def case_fail(*args, **kwargs):
        call_order.append('case')
        raise Exception("CASE failed immediately")

    mock_controller.EstablishPASESession = pase_success
    mock_controller.GetConnectedDevice = case_fail

    pase_params = {'discriminator': TEST_DISCRIMINATOR, 'passcode': TEST_PASSCODE}

    try:
        await commissioning._establish_pase_or_case_session(mock_controller, TEST_NODE_ID, pase_params)
        # Should succeed eventually via PASE
    except Exception as e:
        return f"Should have succeeded via PASE fallback: {e}"

    return None


async def test_parallel_session_both_fail():
    """
    Test: Both PASE and CASE connection attempts fail.

    Scenario: Device unreachable or invalid credentials.
    Expected: RuntimeError raised with both error messages.

    Value: Verifies proper error handling when device can't be reached.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    # First task fails quickly, second task fails after delay
    # This ensures there's a pending task when first one fails
    async def pase_fail_fast(*args, **kwargs):
        raise Exception("PASE failed")

    async def case_fail_slow(*args, **kwargs):
        await asyncio.sleep(0.1)  # Delay so PASE fails first
        raise Exception("CASE failed")

    mock_controller.EstablishPASESession = pase_fail_fast
    mock_controller.GetConnectedDevice = case_fail_slow

    pase_params = {'discriminator': TEST_DISCRIMINATOR, 'passcode': TEST_PASSCODE}

    try:
        await commissioning._establish_pase_or_case_session(mock_controller, TEST_NODE_ID, pase_params)
        return "Should have raised RuntimeError when both fail"
    except RuntimeError as e:
        error_msg = str(e)
        if "PASE" not in error_msg or "CASE" not in error_msg:
            return f"Error should mention both PASE and CASE failures: {e}"
    except Exception as e:
        return f"Expected RuntimeError, got {type(e).__name__}: {e}"

    return None


async def test_parallel_session_no_pase_params():
    """
    Test: No PASE params provided, only CASE attempted.

    Scenario: Caller doesn't have commissioning credentials.
    Expected: Only CASE is attempted (may succeed or fail).

    Value: Verifies graceful handling when PASE not possible.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    case_called = False

    async def case_success(*args, **kwargs):
        nonlocal case_called
        case_called = True
        return MagicMock()

    mock_controller.GetConnectedDevice = case_success

    try:
        await commissioning._establish_pase_or_case_session(mock_controller, TEST_NODE_ID, pase_params=None)
        # Should succeed via CASE only
        if not case_called:
            return "CASE should have been attempted"
    except Exception as e:
        return f"Should succeed with CASE-only: {e}"

    return None


# =============================================================================
# CATEGORY C: is_commissioned() Tests
# =============================================================================

async def test_is_commissioned_scenario2_operational_shortcircuit():
    """
    Test: SCENARIO 2 - Device commissioned on THIS fabric.

    Behavior: DNS-SD finds device operational -> return True immediately.
    No PASE/CASE needed since DNS-SD confirms commissioning.

    Value: This is the FAST PATH optimization. Avoids connection overhead
    when DNS-SD already confirms the device is on our fabric.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    with patch.object(commissioning, '_is_device_operational_via_dnssd', new_callable=AsyncMock) as mock_dnssd:
        mock_dnssd.return_value = True

        result = await commissioning.is_commissioned(mock_controller, TEST_NODE_ID)

        if not result:
            return "Expected True when DNS-SD confirms operational"
        if not mock_dnssd.called:
            return "DNS-SD check should have been called"
    return None


async def test_is_commissioned_scenario1_factory_fresh():
    """
    Test: SCENARIO 1 - Device NOT commissioned on ANY fabric (factory fresh).

    Behavior:
    - DNS-SD returns False (no operational advertisement)
    - Parallel PASE/CASE attempted (PASE wins for factory fresh)
    - ReadAttribute returns 0 root certificates
    - Returns False

    Value: Correctly identifies factory-fresh devices that need commissioning.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    with patch.object(commissioning, '_is_device_operational_via_dnssd', new_callable=AsyncMock) as mock_dnssd, \
            patch.object(commissioning, '_establish_pase_or_case_session', new_callable=AsyncMock) as mock_parallel, \
            patch('matter.clusters.OperationalCredentials') as MockOpCreds:

        mock_dnssd.return_value = False
        mock_parallel.return_value = None

        # Factory fresh: 0 root certificates
        mock_controller.ReadAttribute = AsyncMock(return_value={
            TEST_ENDPOINT: {
                MockOpCreds: {
                    MockOpCreds.Attributes.TrustedRootCertificates: []  # Empty = factory fresh
                }
            }
        })

        pase_params = {'discriminator': TEST_DISCRIMINATOR, 'passcode': TEST_PASSCODE}
        result = await commissioning.is_commissioned(mock_controller, TEST_NODE_ID, pase_params=pase_params)

        if result:
            return "Expected False for factory-fresh device (0 root certs)"
        if not mock_parallel.called:
            return "Parallel session should have been attempted"
    return None


async def test_is_commissioned_scenario3_other_fabric():
    """
    Test: SCENARIO 3 - Device commissioned on ANOTHER fabric (not ours).

    Behavior:
    - DNS-SD returns False (device not on OUR fabric)
    - Parallel PASE/CASE attempted (PASE wins if commissioning window open)
    - ReadAttribute returns >= 1 root certificates (other fabric's certs)
    - Returns True (device IS commissioned, just not to us)

    Value: Correctly identifies that device has fabrics, even if not ours.
    Test scripts may need to handle this case differently.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    with patch.object(commissioning, '_is_device_operational_via_dnssd', new_callable=AsyncMock) as mock_dnssd, \
            patch.object(commissioning, '_establish_pase_or_case_session', new_callable=AsyncMock) as mock_parallel, \
            patch('matter.clusters.OperationalCredentials') as MockOpCreds:

        mock_dnssd.return_value = False
        mock_parallel.return_value = None

        # Commissioned to other fabric: 1 root certificate (other fabric's)
        mock_controller.ReadAttribute = AsyncMock(return_value={
            TEST_ENDPOINT: {
                MockOpCreds: {
                    MockOpCreds.Attributes.TrustedRootCertificates: [b'other_fabric_cert']
                }
            }
        })

        pase_params = {'discriminator': TEST_DISCRIMINATOR, 'passcode': TEST_PASSCODE}
        result = await commissioning.is_commissioned(mock_controller, TEST_NODE_ID, pase_params=pase_params)

        if not result:
            return "Expected True for device commissioned to other fabric"
    return None


async def test_is_commissioned_not_operational_no_pase_params():
    """
    Test: Device not operational via DNS-SD and no PASE params provided.

    Behavior: Raises ValueError - can't safely check without risking timeout.

    Value: Prevents callers from accidentally causing long CASE timeouts
    when device might not be commissioned.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    with patch.object(commissioning, '_is_device_operational_via_dnssd', new_callable=AsyncMock) as mock_dnssd:
        mock_dnssd.return_value = False

        try:
            await commissioning.is_commissioned(mock_controller, TEST_NODE_ID, pase_params=None)
            return "Expected ValueError when not operational and no PASE params"
        except ValueError as e:
            if "not operational on this fabric" not in str(e):
                return f"Unexpected error message: {e}"
    return None


# =============================================================================
# CATEGORY D: get_commissioned_fabric_count() Tests
# =============================================================================

async def test_get_fabric_count_scenario2_operational():
    """
    Test: SCENARIO 2 - Device operational on this fabric, get count via CASE.

    Value: Verifies correct count retrieval when device is on our fabric.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    with patch.object(commissioning, '_is_device_operational_via_dnssd', new_callable=AsyncMock) as mock_dnssd, \
            patch('matter.clusters.OperationalCredentials') as MockOpCreds:

        mock_dnssd.return_value = True

        # Device has 2 fabrics
        mock_controller.ReadAttribute = AsyncMock(return_value={
            TEST_ENDPOINT: {
                MockOpCreds: {
                    MockOpCreds.Attributes.TrustedRootCertificates: [b'cert1', b'cert2']
                }
            }
        })

        result = await commissioning.get_commissioned_fabric_count(mock_controller, TEST_NODE_ID)

        if result != 2:
            return f"Expected fabric count 2, got {result}"
    return None


async def test_get_fabric_count_scenario1_factory_fresh():
    """
    Test: SCENARIO 1 - Factory fresh device, should return 0.

    Value: Verifies 0 count for uncommissioned devices.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    with patch.object(commissioning, '_is_device_operational_via_dnssd', new_callable=AsyncMock) as mock_dnssd, \
            patch.object(commissioning, '_establish_pase_or_case_session', new_callable=AsyncMock) as mock_parallel, \
            patch('matter.clusters.OperationalCredentials') as MockOpCreds:

        mock_dnssd.return_value = False
        mock_parallel.return_value = None

        # Factory fresh: 0 certificates
        mock_controller.ReadAttribute = AsyncMock(return_value={
            TEST_ENDPOINT: {
                MockOpCreds: {
                    MockOpCreds.Attributes.TrustedRootCertificates: []
                }
            }
        })

        pase_params = {'discriminator': TEST_DISCRIMINATOR, 'passcode': TEST_PASSCODE}
        result = await commissioning.get_commissioned_fabric_count(
            mock_controller, TEST_NODE_ID, pase_params=pase_params
        )

        if result != 0:
            return f"Expected fabric count 0 for factory fresh, got {result}"
    return None


async def test_get_fabric_count_not_operational_no_pase_params():
    """
    Test: Device not operational via DNS-SD and no PASE params provided.

    Value: Ensures ValueError is raised to prevent timeout.
    """
    from matter.testing import commissioning

    mock_controller = MockDeviceController()

    with patch.object(commissioning, '_is_device_operational_via_dnssd', new_callable=AsyncMock) as mock_dnssd:
        mock_dnssd.return_value = False

        try:
            await commissioning.get_commissioned_fabric_count(mock_controller, TEST_NODE_ID, pase_params=None)
            return "Expected ValueError when not operational and no PASE params"
        except ValueError as e:
            if "not operational on this fabric" not in str(e):
                return f"Unexpected error message: {e}"
    return None


# =============================================================================
# TEST RUNNER
# =============================================================================

def main():
    """Run all tests and report results."""
    tests = [
        # Category A: DNS-SD Discovery Tests
        ("A1. DNS-SD: device found on THIS fabric", test_dnssd_device_found_on_this_fabric),
        ("A2. DNS-SD: device on DIFFERENT fabric", test_dnssd_device_on_different_fabric),
        ("A3. DNS-SD: no services found", test_dnssd_no_services_found),
        ("A4. DNS-SD: exception handling", test_dnssd_exception_handling),
        ("A5. DNS-SD: import error handling", test_dnssd_import_error_handling),
        ("A6. DNS-SD: multiple services finds correct one", test_dnssd_multiple_services_finds_correct_one),
        ("A7. DNS-SD: same fabric different node", test_dnssd_same_fabric_different_node),

        # Category B: Parallel Session Establishment Tests
        ("B1. Parallel: PASE wins race", test_parallel_session_pase_wins),
        ("B2. Parallel: CASE wins race", test_parallel_session_case_wins),
        ("B3. Parallel: first fails, second succeeds", test_parallel_session_first_fails_second_succeeds),
        ("B4. Parallel: both fail", test_parallel_session_both_fail),
        ("B5. Parallel: no PASE params (CASE only)", test_parallel_session_no_pase_params),

        # Category C: is_commissioned() Tests
        ("C1. is_commissioned: SCENARIO 2 - operational shortcircuit", test_is_commissioned_scenario2_operational_shortcircuit),
        ("C2. is_commissioned: SCENARIO 1 - factory fresh", test_is_commissioned_scenario1_factory_fresh),
        ("C3. is_commissioned: SCENARIO 3 - other fabric", test_is_commissioned_scenario3_other_fabric),
        ("C4. is_commissioned: not operational, no PASE params", test_is_commissioned_not_operational_no_pase_params),

        # Category D: get_commissioned_fabric_count() Tests
        ("D1. get_fabric_count: SCENARIO 2 - operational", test_get_fabric_count_scenario2_operational),
        ("D2. get_fabric_count: SCENARIO 1 - factory fresh", test_get_fabric_count_scenario1_factory_fresh),
        ("D3. get_fabric_count: not operational, no PASE params", test_get_fabric_count_not_operational_no_pase_params),
    ]

    print("\n" + "=" * 70)
    print("COMMISSIONING STATUS DETECTION - UNIT TESTS")
    print("=" * 70)

    failures = []
    passed = 0
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)

    for test_name, test_func in tests:
        try:
            error = loop.run_until_complete(test_func())
            if error:
                failures.append(f"{test_name}: {error}")
                print(f"  FAIL: {test_name}")
                print(f"        {error}")
            else:
                print(f"  PASS: {test_name}")
                passed += 1
        except Exception as e:
            failures.append(f"{test_name}: Exception - {e}")
            print(f"  CRASH: {test_name}")
            print(f"         {type(e).__name__}: {e}")

    loop.close()

    # Summary
    print("\n" + "-" * 70)
    print(f"SUMMARY: {passed}/{len(tests)} tests passed, {len(failures)} failures")
    print("-" * 70)

    if failures:
        print("\nFailed tests:")
        for f in failures:
            print(f"  - {f}")

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())

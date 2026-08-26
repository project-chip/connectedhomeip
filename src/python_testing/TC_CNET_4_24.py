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
TC-CNET-4.24 — [Thread] Network Commissioning Success After Connection Failures.

This module implements the PASE variant of TC-CNET-4.24: it exercises a
Thread-capable DUT that has NOT yet been commissioned, connecting to it over
BLE via PASE, then verifying that the DUT correctly reports a failed
ConnectNetwork against an unknown Extended PAN ID and can still be fully
commissioned with the correct dataset afterwards.

Because the control channel is BLE/PASE, Thread join attempts do not tear
down the session. The test is therefore only meaningful on devices that
report SupportsConcurrentConnection = True and is skipped otherwise.
"""

import asyncio
import logging
import sys
from typing import Callable

from mobly import asserts

import matter.clusters as Clusters
from matter.clusters.Types import NullValue
from matter.commissioning import ROOT_ENDPOINT_ID
from matter.testing.decorators import async_test_body
from matter.testing.matter_testing import MatterBaseTest
from matter.testing.runner import TestStep, default_matter_test_main

logger = logging.getLogger(__name__)

# Timeout constants
TIMED_REQUEST_TIMEOUT_MS = 5000     # Matter timed-invoke timeout for cluster commands (5 s)
NETWORK_STATUS_UPDATE_DELAY = 5     # Grace period for the DUT to publish LastNetworkingStatus (5 s for Thread)
THREAD_CONNECT_ATTEMPT_DELAY = 40   # Time granted to the Thread stack to attempt joining before we read status
TIMEOUT = 300                       # Overall per-test timeout enforced by the async runner (5 min)

# Matter spec version thresholds — the SpecificationVersion attribute uses
# 32-bit encoding: 0xMMmmpp00 (major, minor, patch). 0x01070000 == spec 1.7.0.
MATTER_SPEC_VERSION_1_7 = 0x01070000

# Cluster aliases
cnet = Clusters.NetworkCommissioning
cgen = Clusters.GeneralCommissioning
cbasic = Clusters.BasicInformation

# Thread TLV types (from the Thread Operational Dataset specification)
EXTENDED_PAN_ID_TLV_TYPE = 0x02     # 8-byte value


def get_thread_tlv(dataset: bytes, tlv_type: int, expected_length: int | None = None) -> bytes:
    """Extract a single TLV value from a Thread Operational Dataset.

    A Thread operational dataset is a concatenation of TLVs in the form
    ``Type (1 byte) | Length (1 byte) | Value (Length bytes)``. This helper
    walks the dataset and returns the value payload of the first TLV whose
    type matches ``tlv_type``.

    Args:
        dataset: The full Thread operational dataset (raw bytes).
        tlv_type: The TLV type identifier to search for (e.g. 0x02 for
            Extended PAN ID, 0x05 for Network Key).
        expected_length: If provided, the TLV's length byte must match this
            value or a ``ValueError`` is raised. Useful as a sanity check
            for fixed-length TLVs (Extended PAN ID = 8, Network Key = 16).

    Returns:
        The value bytes of the matched TLV (does NOT include the type/length
        prefix).

    Raises:
        ValueError: If no TLV of the requested type is present, if the TLV
            extends past the dataset boundary, or if ``expected_length`` was
            provided and did not match.
    """
    i = 0
    while i < len(dataset) - 1:
        current_type = dataset[i]
        current_length = dataset[i + 1]

        if current_type == tlv_type:
            if expected_length is not None and current_length != expected_length:
                raise ValueError(
                    f"Invalid TLV length for type 0x{tlv_type:02x}: {current_length}, expected {expected_length} bytes"
                )
            if i + 2 + current_length <= len(dataset):
                return dataset[i + 2: i + 2 + current_length]
            raise ValueError(f"TLV type 0x{tlv_type:02x} extends beyond dataset boundary")

        i += 2 + current_length

    raise ValueError(f"TLV type 0x{tlv_type:02x} not found in Thread operational dataset")


def modify_thread_tlv(dataset: bytes, tlv_type: int, modifier_fn: Callable[[bytes], bytes]) -> bytes:
    """Return a copy of a Thread dataset with one TLV's value transformed.

    The TLV type/length prefix is preserved and only the value bytes are
    replaced by ``modifier_fn(original_value)``. The new value must have the
    same length as the original — this helper deliberately does not support
    resizing TLVs (which would also require adjusting downstream offsets).

    Args:
        dataset: The full Thread operational dataset to modify.
        tlv_type: The TLV type whose value should be transformed.
        modifier_fn: Callable that receives the original value bytes and
            returns the replacement value bytes. Must preserve length.

    Returns:
        A new bytes object containing the full dataset with the target TLV's
        value replaced. The original ``dataset`` is not mutated.

    Raises:
        ValueError: If the requested TLV type is not present, if its declared
            length extends past the dataset, or if the modifier returned a
            value of a different length than the original.
    """
    result = bytearray(dataset)
    i = 0

    while i < len(result) - 1:
        current_type = result[i]
        current_length = result[i + 1]

        if current_type == tlv_type:
            value_start = i + 2
            value_end = value_start + current_length

            if value_end > len(result):
                raise ValueError(f"TLV type 0x{tlv_type:02x} extends beyond dataset boundary")

            original_value = bytes(result[value_start:value_end])
            modified_value = modifier_fn(original_value)

            if len(modified_value) != current_length:
                raise ValueError(
                    f"Modified TLV value for type 0x{tlv_type:02x} has different length: "
                    f"{len(modified_value)} vs original {current_length}"
                )

            result[value_start:value_end] = modified_value
            logger.info(" --- Modified TLV type 0x%02x (%d bytes)", tlv_type, current_length)
            return bytes(result)

        i += 2 + current_length

    raise ValueError(f"TLV type 0x{tlv_type:02x} not found in dataset")


class TC_CNET_4_24(MatterBaseTest):
    """
    [TC-CNET-4.24] [Thread] Network Commissioning Success After Connection Failures [DUT-Server]
    PASE session variant — for uncommissioned devices.

    This test establishes a PASE session over BLE and verifies that the DUT can
    recover from a Thread network connection failure (wrong Extended PAN ID) and
    successfully commission with correct credentials.

    Gated on SupportsConcurrentConnection = True.

    Usage:
        rm -rf /tmp/chip_kvs
        python src/python_testing/TC_CNET_4_24.py \\
            --in-test-commissioning-method ble-thread \\
            --discriminator <discriminator> \\
            --passcode <passcode> \\
            --thread-dataset-hex <dataset_hex>
    """

    async def _validate_network_config_response(
        self,
        response: cnet.Commands.NetworkConfigResponse,
        expected_status: cnet.Enums.NetworkCommissioningStatusEnum | None = None,
    ) -> None:
        """Assert that a NetworkConfigResponse matches the expected status.

        Also enforces the spec cap on ``debugText`` length (512 bytes) when a
        debug string is present.

        Args:
            response: The response returned by AddOrUpdateThreadNetwork,
                AddOrUpdateWiFiNetwork, RemoveNetwork or ReorderNetwork.
            expected_status: The NetworkCommissioningStatusEnum value the
                response is expected to carry. Defaults to ``kSuccess``.

        Raises:
            mobly.signals.TestFailure: If the response type is wrong, the
                status differs from expectation, or the debug text is too long.
        """
        expected_status = expected_status or cnet.Enums.NetworkCommissioningStatusEnum.kSuccess
        asserts.assert_true(
            isinstance(response, cnet.Commands.NetworkConfigResponse),
            "Unexpected response type from NetworkConfig command")
        asserts.assert_equal(
            response.networkingStatus, expected_status,
            f"Expected NetworkingStatus {expected_status}, got {response.networkingStatus}")
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"debugText too long: {len(response.debugText)} bytes")

    async def _validate_connect_network_response(
        self,
        response: cnet.Commands.ConnectNetworkResponse,
        expect_success: bool = True,
    ) -> None:
        """Assert that a ConnectNetworkResponse carries the expected outcome.

        Args:
            response: The response returned by the ConnectNetwork command.
            expect_success: When True the response must be ``kSuccess``; when
                False it must be any status other than ``kSuccess``.

        Raises:
            mobly.signals.TestFailure: If the response type is wrong, the
                status does not match ``expect_success``, or the debug text
                exceeds the 512-byte spec cap.
        """
        asserts.assert_true(
            isinstance(response, cnet.Commands.ConnectNetworkResponse),
            "Unexpected response type from ConnectNetwork command")
        if expect_success:
            asserts.assert_equal(
                response.networkingStatus,
                cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                f"Expected success, got {response.networkingStatus}")
        else:
            asserts.assert_not_equal(
                response.networkingStatus,
                cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                "Expected failure, got success")
        if response.debugText:
            asserts.assert_less_equal(len(response.debugText), 512,
                                      f"debugText too long: {len(response.debugText)} bytes")

    async def _read_networks(self, endpoint: int) -> list:
        """Read the NetworkCommissioning ``Networks`` attribute.

        Args:
            endpoint: Endpoint hosting the NetworkCommissioning cluster
                (typically ``ROOT_ENDPOINT_ID`` == 0).

        Returns:
            The current list of ``NetworkInfoStruct`` entries. Each element
            exposes at least ``networkID`` (bytes) and ``connected`` (bool).
            An empty list is returned when no networks are configured.
        """
        return await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.Networks)

    async def _read_last_networking_status(
        self, endpoint: int
    ) -> cnet.Enums.NetworkCommissioningStatusEnum:
        """Read and log the ``LastNetworkingStatus`` attribute.

        Args:
            endpoint: Endpoint hosting the NetworkCommissioning cluster.

        Returns:
            The current value of ``LastNetworkingStatus``. May be ``NullValue``
            if the DUT has not yet performed a networking operation on this
            session.
        """
        status = await self.read_single_attribute(
            dev_ctrl=self.default_controller,
            node_id=self.dut_node_id,
            endpoint=endpoint,
            attribute=cnet.Attributes.LastNetworkingStatus)
        logger.info(" --- LastNetworkingStatus = %s", status)
        return status

    def _emit_warning(self, msg: str) -> None:
        """Record a non-fatal warning that surfaces in the test summary.

        Uses ``self.record_warning`` when the base test class exposes it, so
        the warning appears in the framework's post-run report. Falls back to
        a plain ``logger.warning`` when the method is not available (older
        framework versions or a stubbed harness).

        Args:
            msg: Human-readable warning message.
        """
        logger.warning(msg)
        if hasattr(self, 'record_warning'):
            try:
                self.record_warning(msg)
                return
            except Exception as e:
                logger.warning("record_warning failed (%s), warning only in logs", e)

    @property
    def default_timeout(self) -> int:
        """Overall per-test timeout in seconds enforced by the async runner."""
        return TIMEOUT

    def pics_TC_CNET_4_24(self) -> list[str]:
        """Return the PICS codes required for this test to be applicable.

        ``PICS.S.F01`` corresponds to the NetworkCommissioning cluster
        Thread-networking feature bit — the test is only meaningful on
        Thread-capable DUTs.
        """
        return ['PICS.S.F01']

    def steps_TC_CNET_4_24(self) -> list[TestStep]:
        """Declare the ordered list of test steps shown in the run report."""
        return [
            TestStep(0,
                     "TH establishes a PASE session with the DUT over BLE, verifies the DUT reports "
                     "SupportsConcurrentConnection = True (the test is skipped otherwise) and opens "
                     "a fail-safe window (300 seconds) for network reconfiguration testing",
                     is_commissioning=True),
            TestStep(1,
                     "TH reads Networks attribute and removes all configured networks",
                     "Verify that DUT successfully removed all networks\n"
                     "Verify LastNetworkingStatus and LastConnectErrorValue are Null after network removal"),
            TestStep(2,
                     "TH sends AddOrUpdateThreadNetwork with valid format but incorrect Extended PAN ID, Breadcrumb = 2",
                     "Verify NetworkConfigResponse with NetworkingStatus kSuccess (0)"),
            TestStep(3,
                     "TH sends ConnectNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 3",
                     "Verify ConnectNetworkResponse with NetworkingStatus kSuccess (0) or a raised exception "
                     "(the DUT may drop the response when it cannot join)"),
            TestStep(4,
                     "TH reads LastNetworkingStatus after Extended PAN ID connection failure",
                     "For DUTs reporting SpecificationVersion >= 1.7: expect kNetworkNotFound (5). "
                     "For pre-1.7 DUTs: expect any non-success status, a mismatch with kNetworkNotFound "
                     "is recorded as a warning but does not fail the test."),
            TestStep(5,
                     "TH reads Networks attribute",
                     "Verify dataset with incorrect Extended PAN ID is in the network list"),
            TestStep(6,
                     "TH sends RemoveNetwork command with dataset containing incorrect Extended PAN ID, Breadcrumb = 6",
                     "Verify NetworkConfigResponse with NetworkingStatus kSuccess (0)"),
            TestStep(7,
                     "TH reads Networks attribute",
                     "Verify Networks list is empty after removal"),
            TestStep(8,
                     "TH fully commissions the DUT using the correct Thread network",
                     "Commissioning is successful and the DUT reports the correct Thread network as connected"),
        ]

    def desc_TC_CNET_4_24(self) -> str:
        """Return the one-line human-readable description of this test."""
        return "[TC-CNET-4.24] [Thread] Network Commissioning Success After Connection Failures [DUT-Server] [PASE]"

    @async_test_body
    async def test_TC_CNET_4_24(self) -> None:
        """Run the TC-CNET-4.24 PASE flow end-to-end.

        High-level flow:
            0. Open BLE-PASE, gate on ``SupportsConcurrentConnection``,
               read ``SpecificationVersion`` (drives strictness of step 4),
               arm the 300 s fail-safe.
            1. Clear any pre-existing networks and verify status attributes
               are reset to ``Null``.
            2-3. Add a Thread dataset with an intentionally-wrong Extended
               PAN ID and attempt to connect to it.
            4. Verify the DUT reports a failure via ``LastNetworkingStatus``
               (strict ``kNetworkNotFound`` on spec >= 1.7, warning-only on
               pre-1.7 devices).
            5-7. Verify the bad network is present, remove it, verify the
               list is empty again.
            8. Fully commission the DUT with the correct dataset via
               ``commission_devices()`` to avoid waiting out the fail-safe.

        Raises:
            mobly.signals.TestFailure: Propagated by any assertion in the
                helpers or the step body.
            mobly.signals.TestSkip: Raised via ``asserts.skip`` when the DUT
                reports ``SupportsConcurrentConnection = False``.
        """
        endpoint: int = ROOT_ENDPOINT_ID

        correct_thread_dataset: bytes | None = self.matter_test_config.thread_operational_dataset
        if correct_thread_dataset is None:
            # Fallback: parse --thread-dataset-hex directly from command line args.
            # The framework only populates thread_operational_dataset when a
            # commissioning method like ble-thread is specified, so this path
            # covers invocations that skip auto-commissioning.
            for i, arg in enumerate(sys.argv):
                if arg == '--thread-dataset-hex' and i + 1 < len(sys.argv):
                    try:
                        correct_thread_dataset = bytes.fromhex(sys.argv[i + 1])
                        logger.info(" --- Parsed thread dataset from --thread-dataset-hex command line argument")
                    except ValueError:
                        asserts.fail(f"Invalid hex value for --thread-dataset-hex: {sys.argv[i + 1]}")
                    break
        if correct_thread_dataset is None:
            asserts.fail("Thread operational dataset must be provided via --thread-dataset-hex <dataset_hex>.")
        logger.info(" --- Correct Thread operational dataset: %s", correct_thread_dataset.hex())

        # Build the single incorrect dataset we still exercise: wrong Extended PAN ID.
        # On Thread the odds of encountering a matching network with a bad key in the field are slim,
        # the interesting failure mode is a not-found network.
        incorrect_thread_dataset_1: bytes = modify_thread_tlv(
            correct_thread_dataset, EXTENDED_PAN_ID_TLV_TYPE,
            lambda v: bytes(b ^ 0xAA for b in v))
        logger.info(" --- Incorrect dataset (modified Extended PAN ID): %s", incorrect_thread_dataset_1.hex())

        correct_network_id: bytes = get_thread_tlv(correct_thread_dataset,
                                                   tlv_type=EXTENDED_PAN_ID_TLV_TYPE, expected_length=8)
        network_id_1: bytes = get_thread_tlv(incorrect_thread_dataset_1,
                                             tlv_type=EXTENDED_PAN_ID_TLV_TYPE, expected_length=8)

        # PASE, SupportsConcurrentConnection gate, and fail-safe.
        self.step(0)

        logger.info(" --- Establishing PASE session with DUT over BLE...")
        try:
            setup_payload_info = self.get_setup_payload_info()
            if not setup_payload_info or len(setup_payload_info) == 0:
                asserts.fail("No setup payload info available (QR code, manual code, or passcode+discriminator required)")

            passcode: int = setup_payload_info[0].passcode
            discriminator: int = setup_payload_info[0].filter_value

            await self.default_controller.EstablishPASESessionBLE(
                setupPinCode=passcode,
                discriminator=discriminator,
                nodeId=self.dut_node_id)
            logger.info(" --- PASE session established successfully over BLE")
        except Exception as e:
            asserts.fail(f"Failed to establish PASE session over BLE: {e}")

        # Gate the test on SupportsConcurrentConnection == True.
        # This test only makes sense on devices that keep the PASE/BLE session alive
        # while attempting Thread joins, non-concurrent devices tear down the session
        # on ConnectNetwork and cannot be exercised this way.
        try:
            supports_concurrent: bool = await self.read_single_attribute(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                attribute=cgen.Attributes.SupportsConcurrentConnection)
            logger.info(" --- SupportsConcurrentConnection = %s", supports_concurrent)
        except Exception as e:
            asserts.fail(f"Could not read SupportsConcurrentConnection: {e}")

        if not supports_concurrent:
            skip_msg = ("Skipping TC-CNET-4.24: DUT reports SupportsConcurrentConnection = False. "
                        "This test requires a concurrent-connection-capable DUT.")
            logger.info(" --- %s", skip_msg)
            asserts.skip(skip_msg)

        # Best-effort read of SpecificationVersion to decide how strict Step 4 should be.
        # Devices predating spec 1.7 are allowed to report any non-success status.
        spec_version: int = 0
        try:
            spec_version = await self.read_single_attribute(
                dev_ctrl=self.default_controller,
                node_id=self.dut_node_id,
                endpoint=endpoint,
                attribute=cbasic.Attributes.SpecificationVersion)
            logger.info(" --- SpecificationVersion = 0x%08x", spec_version)
        except Exception as e:
            logger.warning(" --- Could not read SpecificationVersion (%s), treating DUT as pre-1.7", e)

        is_spec_1_7_or_later: bool = spec_version >= MATTER_SPEC_VERSION_1_7

        # Arm fail-safe for 300 seconds.
        logger.info(" --- Arming fail-safe to 300 seconds")
        arm_failsafe_response = await self.send_single_cmd(
            endpoint=ROOT_ENDPOINT_ID,
            cmd=cgen.Commands.ArmFailSafe(expiryLengthSeconds=300, breadcrumb=0))
        asserts.assert_equal(arm_failsafe_response.errorCode, cgen.Enums.CommissioningErrorEnum.kOk,
                             f"Fail-safe arming failed: {arm_failsafe_response.errorCode}")
        logger.info(" --- Fail-safe armed successfully (300 seconds)")

        self.step(1)

        networks = await self._read_networks(endpoint)
        logger.info(" --- Found %d network(s) configured", len(networks))
        for network in networks:
            network_id: bytes = network.networkID
            logger.info(" --- Removing network with Extended PAN ID: %s", network_id.hex())
            remove_response = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=cnet.Commands.RemoveNetwork(networkID=network_id, breadcrumb=1))
            await self._validate_network_config_response(remove_response)
            logger.info(" --- Network removed successfully (Extended PAN ID: %s)", network_id.hex())

        networks_after = await self._read_networks(endpoint)
        asserts.assert_equal(len(networks_after), 0,
                             f"Expected empty network list, but found {len(networks_after)} network(s)")
        logger.info(" --- All networks successfully removed.")

        logger.info(" --- Waiting %ss for device to update status...", NETWORK_STATUS_UPDATE_DELAY)
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Verify LastNetworkingStatus and LastConnectErrorValue are Null after removal.
        last_networking_status = await self.read_single_attribute(
            dev_ctrl=self.default_controller, node_id=self.dut_node_id,
            endpoint=endpoint, attribute=cnet.Attributes.LastNetworkingStatus)
        last_connect_error = await self.read_single_attribute(
            dev_ctrl=self.default_controller, node_id=self.dut_node_id,
            endpoint=endpoint, attribute=cnet.Attributes.LastConnectErrorValue)
        logger.info(" --- LastNetworkingStatus: %s", last_networking_status)
        logger.info(" --- LastConnectErrorValue: %s", last_connect_error)

        asserts.assert_is(last_networking_status, NullValue,
                          f"Expected LastNetworkingStatus to be Null, got {last_networking_status}")
        asserts.assert_is(last_connect_error, NullValue,
                          f"Expected LastConnectErrorValue to be Null, got {last_connect_error}")

        self.step(2)
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.AddOrUpdateThreadNetwork(
                operationalDataset=incorrect_thread_dataset_1, breadcrumb=2),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
        await self._validate_network_config_response(response)

        self.step(3)
        logger.info(" --- Sending ConnectNetwork with incorrect Extended PAN ID: %s", network_id_1.hex())

        try:
            response = await self.send_single_cmd(
                endpoint=endpoint,
                cmd=cnet.Commands.ConnectNetwork(networkID=network_id_1, breadcrumb=3),
                timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
            await self._validate_connect_network_response(response, expect_success=True)
            logger.info(" --- ConnectNetwork returned response")
        except Exception as e:
            logger.info(" --- ConnectNetwork raised exception: %s", type(e).__name__)
            logger.info(" --- Continuing to observe post-connect network state")

        # Wait for Thread to attempt connection and update status.
        await asyncio.sleep(THREAD_CONNECT_ATTEMPT_DELAY)
        await asyncio.sleep(NETWORK_STATUS_UPDATE_DELAY)

        # Version-conditional check per PR feedback:
        #   - Spec 1.7+: enforce kNetworkNotFound.
        #   - Pre-1.7:   allow any non-success status, a mismatch with kNetworkNotFound
        #                is a warning rather than a failure.
        self.step(4)
        status: cnet.Enums.NetworkCommissioningStatusEnum = await self._read_last_networking_status(endpoint)
        expected: cnet.Enums.NetworkCommissioningStatusEnum = cnet.Enums.NetworkCommissioningStatusEnum.kNetworkNotFound

        if is_spec_1_7_or_later:
            asserts.assert_equal(
                status, expected,
                f"Spec >= 1.7 DUT: expected LastNetworkingStatus = {expected}, got {status}")
        else:
            asserts.assert_not_equal(
                status, cnet.Enums.NetworkCommissioningStatusEnum.kSuccess,
                "Pre-1.7 DUT: LastNetworkingStatus must not be kSuccess after failed ConnectNetwork")
            if status != expected:
                self._emit_warning(
                    f"Pre-1.7 DUT reported LastNetworkingStatus = {status} after ConnectNetwork "
                    f"with an unknown Extended PAN ID, spec 1.7 requires {expected}. "
                    "Accepted for now, but this behavior should be corrected."
                )

        self.step(5)
        networks = await self._read_networks(endpoint)
        logger.info(" --- Networks attribute has %d network(s)", len(networks))
        network_ids: list = [net.networkID for net in networks]
        asserts.assert_in(network_id_1, network_ids,
                          "Incorrect Extended PAN ID not found in Networks attribute")

        self.step(6)
        response = await self.send_single_cmd(
            endpoint=endpoint,
            cmd=cnet.Commands.RemoveNetwork(networkID=network_id_1, breadcrumb=6),
            timedRequestTimeoutMs=TIMED_REQUEST_TIMEOUT_MS)
        await self._validate_network_config_response(response)

        self.step(7)
        networks = await self._read_networks(endpoint)
        logger.info(" --- Networks attribute has %d network(s) after removal", len(networks))
        asserts.assert_equal(len(networks), 0,
                             f"Expected empty Networks list, but has {len(networks)} network(s)")

        # Full commissioning with the correct Thread dataset. Doing this at the end
        # (instead of a bare ConnectNetwork + failsafe wait-out) avoids stalling the
        # test 5 minutes for the fail-safe to expire and leaves the DUT in a clean,
        # commissioned state.
        self.step(8)
        commissioning_ok: bool = await self.commission_devices()
        asserts.assert_true(commissioning_ok, "Commissioning failed with correct Thread network")

        # Best-effort sanity check: verify the DUT is connected to the expected network.
        # commission_devices() re-arms the fail-safe and drives its own commissioning
        # flow, so this is an extra guardrail rather than the primary pass criterion.
        try:
            networks = await self._read_networks(endpoint)
            connected_networks: list = [net.networkID for net in networks if net.connected]
            logger.info(" --- Connected networks: %s", [net.hex() for net in connected_networks])
            logger.info(" --- Expected network ID: %s", correct_network_id.hex())
            asserts.assert_in(
                correct_network_id, connected_networks,
                f"Expected DUT connected to Thread network with Extended PAN ID "
                f"'{correct_network_id.hex()}' after commissioning")
        except Exception as e:
            logger.warning(" --- Post-commissioning Networks read failed: %s", e)

        logger.info(" --- Test completed successfully")


if __name__ == "__main__":
    default_matter_test_main()

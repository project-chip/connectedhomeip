#
#    Copyright (c) 2024 Project CHIP Authors
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
# === BEGIN CI TEST ARGUMENTS ===
# test-runner-runs:
#   run1:
#     app: ${ALL_CLUSTERS_APP}
#     app-args: --discriminator 1234 --KVS kvs1 --trace-to json:${TRACE_APP}.json
#     script-args: >
#       --storage-path admin_storage.json
#       --commissioning-method on-network
#       --discriminator 1234
#       --passcode 20202021
#       --trace-to json:${TRACE_TEST_JSON}.json
#       --trace-to perfetto:${TRACE_TEST_PERFETTO}.perfetto
#       --PICS src/app/tests/suites/certification/ci-pics-values
#     factory-reset: true
#     quiet: true
# === END CI TEST ARGUMENTS ===

import asyncio.exceptions as ae
import logging
from dataclasses import dataclass
from time import sleep
from typing import Optional

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.exceptions import ChipStackError
from chip.interaction_model import InteractionModelError as IME
from chip.testing.matter_testing import MatterBaseTest, TestStep, async_test_body, default_matter_test_main
from mdns_discovery import mdns_discovery
from mobly import asserts


class TC_CADMIN_1_5(MatterBaseTest):
    @dataclass
    class ParsedService:
        service: mdns_discovery.MdnsServiceInfo
        cm: Optional[int] = None
        d: Optional[int] = None

        def __post_init__(self):
            # Safely convert CM value to int if present
            cm_value = self.service.txt_record.get('CM')
            if cm_value is not None:
                try:
                    self.cm = int(cm_value)
                except (ValueError, TypeError):
                    logging.warning(f"Could not convert CM value '{cm_value}' to integer")
                    self.cm = None

            # Safely convert D value to int if present
            d_value = self.service.txt_record.get('D')
            if d_value is not None:
                try:
                    self.d = int(d_value)
                except (ValueError, TypeError):
                    logging.warning(f"Could not convert discriminator value '{d_value}' to integer")
                    self.d = None

        def __str__(self) -> str:
            return f"Service CM={self.cm}, D={self.d}"

        def matches(self, expected_cm: int, expected_d: int) -> bool:
            """Check if this service matches the expected CM and discriminator values."""
            cm_match = self.cm == expected_cm
            d_match = self.d == expected_d
            return cm_match and d_match

    async def get_all_txt_records(self):
        discovery = mdns_discovery.MdnsDiscovery(verbose_logging=True)
        discovery._service_types = [mdns_discovery.MdnsServiceType.COMMISSIONABLE.value]
        await discovery._discover(discovery_timeout_sec=240, log_output=False)

        if mdns_discovery.MdnsServiceType.COMMISSIONABLE.value in discovery._discovered_services:
            return discovery._discovered_services[mdns_discovery.MdnsServiceType.COMMISSIONABLE.value]
        return []

    async def wait_for_correct_cm_value(self, expected_cm_value: int, expected_discriminator: int, max_attempts: int = 5, delay_sec: int = 5):
        """Wait for the correct CM value and discriminator in DNS-SD with retries."""
        for attempt in range(max_attempts):
            raw_services = await self.get_all_txt_records()
            services = [self.ParsedService(service) for service in raw_services]

            # Look through all services for a match
            for parsed_service in services:
                if parsed_service.matches(expected_cm_value, expected_discriminator):
                    logging.info(f"Found matching service: {parsed_service}")
                    return parsed_service.service  # Return the original service object

            # Log what we found for debugging purposes
            logging.info(f"Found {len(services)} services, but none match CM={expected_cm_value}, D={expected_discriminator}")
            for service in services:
                logging.info(f"{service}")
            else:
                logging.info("No services found in this attempt")

            # Not on last attempt, wait and retry
            if attempt < max_attempts - 1:
                logging.info(f"Waiting for service with CM={expected_cm_value} and D={expected_discriminator}, "
                             f"attempt {attempt+1}/{max_attempts}")
                sleep(delay_sec)
            else:
                # Final retry attempt failed
                asserts.fail(f"Failed to find DNS-SD advertisement with CM={expected_cm_value} and "
                             f"discriminator={expected_discriminator} after {max_attempts} attempts. "
                             f"Found services: {[str(s) for s in services]}")

    async def commission_on_network(self, setup_code: int, discriminator: int, expected_error: int = 0):
        # This is expected to error as steps 4 and 7 expects timeout issue or pase connection error to occur due to commissioning window being closed already
        if expected_error == 50:
            try:
                await self.th2.CommissionOnNetwork(
                    nodeId=self.dut_node_id, setupPinCode=setup_code,
                    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)

            except ChipStackError as e:
                asserts.assert_true(int(e.code) == expected_error,
                                    'Unexpected error code returned from Commissioning Attempt')

        else:
            try:
                await self.th2.CommissionOnNetwork(
                    nodeId=self.dut_node_id, setupPinCode=setup_code,
                    filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=discriminator)

            except ae.CancelledError:
                # This is expected to fail due to timeout, however there is no code to validate here, so just passing since the correct exception was raised to get to this point
                pass

    def steps_TC_CADMIN_1_5(self) -> list[TestStep]:
        return [
            TestStep(1, "Commissioning, already done", is_commissioning=True),
            TestStep(2, "TH_CR1 opens a commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM"),
            TestStep(3, "TH_CR1 finds DUT_CE advertising as a commissionable node on DNS-SD",
                     "Verify that the DNS-SD advertisement TXT record shows CM=2"),
            TestStep(4, "TH_CR2 attempts to start a commissioning process with DUT_CE after 190 seconds",
                     "TH_CR2 should fail to commission the DUT since the window should be closed. This may be a failure to find the commissionable node or a failure to establish a PASE connection."),
            TestStep(5, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM",
                     "{resDutSuccess}"),
            TestStep(6, "TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command", "{resDutSuccess}"),
            TestStep(7, "TH_CR2 attempts to start a commissioning process with DUT_CE",
                     "TH_CR2 should fail to commission the DUT since the window should be closed. This may be a failure to find the commissionable node or a failure to establish a PASE connection."),
            TestStep(8, "TH_CR1 revokes the commissioning window on DUT_CE using RevokeCommissioning command.",
                     "Verify this command fails with the cluster specific status code of WindowNotOpen"),
            TestStep(9, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the iterations set to 999",
                     "Verify DUT_CE fails to open Commissioning window with status code 3 (PakeParameterError)"),
            TestStep(10, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the iterations set to 100001",
                     "Verify DUT_CE fails to open Commissioning window with status code 3 (PakeParameterError)"),
            TestStep(11, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the salt set to 'too_short'",
                     "Verify DUT_CE fails to open Commissioning window with status code 3 (PakeParameterError)"),
            TestStep(12, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of 180 seconds using ECM with the salt set to 'this pake salt very very very long'",
                     "Verify DUT_CE fails to open Commissioning window with status code 3 (PakeParameterError)"),
            TestStep(13, "TH_CR1 opens a new commissioning window on DUT_CE using a commissioning timeout of {PIXIT_CWDURATION} seconds using ECM",
                     "Verify DUT_CE opens its Commissioning window to allow a second commissioning"),
            TestStep(14, "TH_CR1 opens another commissioning window on DUT_CE using a commissioning timeout of {PIXIT_CWDURATION} seconds using ECM",
                     "Verify DUT_CE fails to open Commissioning window with status code 2 (Busy)"),
            TestStep(15, "TH_CR2 starts a commissioning process with DUT_CE", "Commissioning is successful"),
            TestStep(16, "TH_CR1 tries to revoke the commissioning window on DUT_CE using RevokeCommissioning command",
                     "Verify DUT_CE fails to revoke giving status code 4 (WindowNotOpen) as there was no window open"),
            TestStep(17, "TH_CR1 sends the RemoveFabric command to the DUT to remove TH_CR2 fabric",
                     "TH_CR1 removes TH_CR2 fabric"),
        ]

    def pics_TC_CADMIN_1_5(self) -> list[str]:
        return ["CADMIN.S"]

    @async_test_body
    async def test_TC_CADMIN_1_5(self):
        self.step(1)
        # Establishing TH1 and TH2
        self.th1 = self.default_controller
        th2_certificate_authority = self.certificate_authority_manager.NewCertificateAuthority()
        th2_fabric_admin = th2_certificate_authority.NewFabricAdmin(vendorId=0xFFF1, fabricId=self.th1.fabricId + 1)
        self.th2 = th2_fabric_admin.NewController(nodeId=2)

        self.step(2)
        params = await self.open_commissioning_window(dev_ctrl=self.th1, timeout=180, node_id=self.dut_node_id)
        logging.info(f"Commissioning window params: {params}")

        self.step(3)
        # Wait for DNS-SD advertisement with correct CM value and discriminator
        # This will either return a valid service or assert failure
        service = await self.wait_for_correct_cm_value(
            expected_cm_value=2,
            expected_discriminator=params.randomDiscriminator
        )
        logging.info(f"Successfully found service with CM={service.txt_record.get('CM')}, D={service.txt_record.get('D')}")
        sleep(190)

        self.step(4)
        await self.commission_on_network(setup_code=params.commissioningParameters.setupPinCode, discriminator=params.randomDiscriminator)

        self.step(5)
        params2 = await self.open_commissioning_window(dev_ctrl=self.th1, timeout=180, node_id=self.dut_node_id)

        self.step(6)
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        sleep(1)

        self.step(7)
        await self.commission_on_network(setup_code=params2.commissioningParameters.setupPinCode, discriminator=params2.randomDiscriminator, expected_error=0x00000032)

        self.step(8)
        try:
            revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        except IME as e:
            asserts.assert_true(e.clusterStatus, Clusters.AdministratorCommissioning.Enums.StatusCode.kWindowNotOpen,
                                "Cluster status must be 4 to pass this step as window should be reported as not open")

        self.step(9)
        # Used following command to generate PAKE verifier for this test step:
        # ./scripts/tools/spake2p/spake2p.py gen-verifier -p 20202021 -s U1BBS0UyUCBLZXkgU2FsdA== -i 999
        iter999 = b"hex:/q9Xque1iokBVf/SGwjfzJWY0vgmFapUoIcgR+4rXdEEHBELKQ2VYwF9XjZiIrfYztJo2adB8O9MtQ/LWlfJMqMUt8jYcuQtYTc2NQIOZWFiKXbT5K7ipt4svYVEs1rmLA=="
        try:
            cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(
                iterations=999, discriminator=3045, PAKEPasscodeVerifier=iter999, commissioningTimeout=180, salt=b"SPAKE2P_Key_Salt")
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd, timedRequestTimeoutMs=2000000)
        except IME as e:
            asserts.assert_equal(e.clusterStatus, Clusters.AdministratorCommissioning.Enums.StatusCode.kPAKEParameterError,
                                 f'Failed to open commissioning window due to an unexpected error code of {e.clusterStatus}')

        self.step(10)
        # Used following command to generate PAKE verifier for this test step:
        # ./scripts/tools/spake2p/spake2p.py gen-verifier -p 20202021 -s U1BBS0UyUCBLZXkgU2FsdA== -i 100001
        iter100001 = b"hex:CUhS9rS2NKjXGYwK0CCG80d6XkC1QSCAfs8++IcOCRcEwM4DlA/wxlm/B7w4G/7tZJmLycmdRLJGlYF2+HDsYdGmoxj0ENNuXTmXsoOhkZUmmTXThAak3U9vGFWbKUHXCQ=="
        try:
            cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(
                iterations=100001, discriminator=3045, PAKEPasscodeVerifier=iter100001, commissioningTimeout=180, salt=b"SPAKE2P_Key_Salt")
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd, timedRequestTimeoutMs=2000000)
        except IME as e:
            asserts.assert_equal(e.clusterStatus, Clusters.AdministratorCommissioning.Enums.StatusCode.kPAKEParameterError,
                                 f'Failed to open commissioning window due to an unexpected error code of {e.clusterStatus}')

        self.step(11)
        # Used following command to generate PAKE verifier for this test step:
        # ./scripts/tools/spake2p/spake2p.py gen-verifier -p 20202021 -s dG9vX3Nob3J0 -i 1000
        short_salt = b"hex:c8StVjueM851ZnKA+/0m83PHeVIhfhhWvGVCGcAnDD8EbCiPuKb1Z18I7l3TvxTbVkvzS2KPjKPOCZt1GW80ZoVDP48NAewqEXfl6lY7nmDG9ZzMIhfa8f1EIiBY0/7eJA=="
        try:
            cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(
                iterations=10000, discriminator=3045, PAKEPasscodeVerifier=short_salt, commissioningTimeout=180, salt=b"too_short")
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd, timedRequestTimeoutMs=2000000)
        except IME as e:
            asserts.assert_equal(e.clusterStatus, Clusters.AdministratorCommissioning.Enums.StatusCode.kPAKEParameterError,
                                 f'Failed to open commissioning window due to an unexpected error code of {e.clusterStatus}')

        self.step(12)
        # Used following command to generate PAKE verifier for this test step:
        # ./scripts/tools/spake2p/spake2p.py gen-verifier -p 20202021 -s dGhpcyBwYWtlIHNhbHQgdmVyeSB2ZXJ5IHZlcnkgbG9uZw== -i 1000
        long_salt = b"hex:nwkb2VD3OTPflW2sAChSwpfkaajErERg/XrhvWPPJL4EM6cSCY/h9lz5SgKy7WB5s1nn1u75amcumZrxnVCXbI0vRrM74BV20p0VyOhpOMBaoHpT2Tvev8pc0JDYCjn6wg=="
        try:
            cmd = Clusters.AdministratorCommissioning.Commands.OpenCommissioningWindow(
                iterations=10000, discriminator=3045, PAKEPasscodeVerifier=long_salt, commissioningTimeout=180, salt=b"'this pake salt very very very long'")
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=cmd, timedRequestTimeoutMs=2000000)
        except IME as e:
            asserts.assert_equal(e.clusterStatus, Clusters.AdministratorCommissioning.Enums.StatusCode.kPAKEParameterError,
                                 f'Failed to open commissioning window due to an unexpected error code of {e.clusterStatus}')

        self.step(13)
        cluster = Clusters.GeneralCommissioning
        attribute = cluster.Attributes.BasicCommissioningInfo
        duration = await self.read_single_attribute_check_success(endpoint=0, cluster=cluster, attribute=attribute)
        params3 = await self.open_commissioning_window(dev_ctrl=self.th1, timeout=duration.maxCumulativeFailsafeSeconds, node_id=self.dut_node_id)

        self.step(14)
        try:
            await self.open_commissioning_window(dev_ctrl=self.th1, timeout=duration.maxCumulativeFailsafeSeconds, node_id=self.dut_node_id)
        except ChipStackError as e:
            # Converting error code to useable format to do assert with
            code = int(((e.msg.split(":"))[2]), 16)
            asserts.assert_equal(code, Clusters.AdministratorCommissioning.Enums.StatusCode.kBusy,
                                 f'Failed to open commissioning window due to an unexpected error code of {e.code}')

        self.step(15)
        await self.th2.CommissionOnNetwork(
            nodeId=self.dut_node_id, setupPinCode=params3.commissioningParameters.setupPinCode,
            filterType=ChipDeviceCtrl.DiscoveryFilterType.LONG_DISCRIMINATOR, filter=params3.randomDiscriminator)

        self.step(16)
        try:
            revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
            await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=revokeCmd, timedRequestTimeoutMs=6000)
        except IME as e:
            asserts.assert_equal(e.clusterStatus, Clusters.AdministratorCommissioning.Enums.StatusCode.kWindowNotOpen,
                                 f'Failed to open commissioning window due to an unexpected error code of {e.clusterStatus}')

        self.step(17)
        TH2_fabric_index = await self.read_single_attribute_check_success(dev_ctrl=self.th2, endpoint=0, cluster=Clusters.OperationalCredentials, attribute=Clusters.OperationalCredentials.Attributes.CurrentFabricIndex)
        removeFabricCmd = Clusters.OperationalCredentials.Commands.RemoveFabric(TH2_fabric_index)
        await self.th1.SendCommand(nodeid=self.dut_node_id, endpoint=0, payload=removeFabricCmd)


if __name__ == "__main__":
    default_matter_test_main()

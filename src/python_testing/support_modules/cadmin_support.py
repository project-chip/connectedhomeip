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

"""
Support module for CADMIN test modules containing shared functionality.
"""

import logging
import random
from dataclasses import dataclass
from time import sleep
from typing import Optional

import chip.clusters as Clusters
from chip import ChipDeviceCtrl
from chip.ChipDeviceCtrl import CommissioningParameters
from chip.interaction_model import Status
from mdns_discovery import mdns_discovery
from mobly import asserts


class CADMINSupport:
    def __init__(self, test_instance):
        self.test = test_instance

    async def get_fabrics(self, th: ChipDeviceCtrl, fabric_filtered: bool = True) -> int:
        """Get fabrics information from the device."""
        OC_cluster = Clusters.OperationalCredentials
        fabric_info = await self.test.read_single_attribute_check_success(
            dev_ctrl=th,
            fabric_filtered=fabric_filtered,
            endpoint=0,
            cluster=OC_cluster,
            attribute=OC_cluster.Attributes.Fabrics
        )
        return fabric_info

    async def read_currentfabricindex(self, th: ChipDeviceCtrl) -> int:
        """Read the current fabric index from the device."""
        cluster = Clusters.OperationalCredentials
        attribute = Clusters.OperationalCredentials.Attributes.CurrentFabricIndex
        current_fabric_index = await self.test.read_single_attribute_check_success(
            dev_ctrl=th,
            endpoint=0,
            cluster=cluster,
            attribute=attribute
        )
        return current_fabric_index

    async def get_txt_record(self):
        discovery = mdns_discovery.MdnsDiscovery(verbose_logging=True)
        comm_service = await discovery.get_commissionable_service(
            discovery_timeout_sec=240,
            log_output=False,
        )
        return comm_service

    async def open_commissioning_window(
        self,
        th: ChipDeviceCtrl,
        timeout: int,
        node_id: int,
        discriminator: int = None,
        option: int = 1,
        iteration: int = 10000
    ) -> CommissioningParameters:
        """
        Open a commissioning window with the specified parameters.

        Args:
            th: Controller to use
            timeout: Window timeout in seconds
            node_id: Target node ID
            discriminator: Optional discriminator value
            option: Commissioning option (default: 1)
            iteration: Number of iterations (default: 10000)
        """
        try:
            params = await th.OpenCommissioningWindow(
                nodeid=node_id,
                timeout=timeout,
                iteration=iteration,
                discriminator=discriminator if discriminator is not None else random.randint(0, 4095),
                option=option
            )
            return params
        except Exception as e:
            logging.exception('Error running OpenCommissioningWindow %s', e)
            asserts.fail('Failed to open commissioning window')

    async def write_nl_attr(self, dut_node_id: int, th: ChipDeviceCtrl, attr_val: object):
        result = await th.WriteAttribute(nodeid=dut_node_id, attributes=[(0, attr_val)])
        asserts.assert_equal(result[0].Status, Status.Success, f"{th} node label write failed")

    async def read_nl_attr(self, dut_node_id: int, th: ChipDeviceCtrl, attr_val: object):
        try:
            await th.ReadAttribute(nodeid=dut_node_id, attributes=[(0, attr_val)])
        except Exception as e:
            asserts.assert_equal(e.err, "Received error message from read attribute attempt")
            self.print_step(0, e)

    async def get_window_status(self, th: ChipDeviceCtrl) -> int:
        """Get the current commissioning window status."""
        AC_cluster = Clusters.AdministratorCommissioning
        window_status = await self.test.read_single_attribute_check_success(
            dev_ctrl=th,
            fabric_filtered=False,
            endpoint=0,
            cluster=AC_cluster,
            attribute=AC_cluster.Attributes.WindowStatus
        )
        return window_status

    def generate_unique_random_value(self, exclude_value: int) -> int:
        """Generate a random value that's different from the specified value."""
        while True:
            random_value = random.randint(10000000, 99999999)
            if random_value != exclude_value:
                return random_value

    async def revoke_commissioning(self, th: ChipDeviceCtrl, node_id: int):
        """Revoke the current commissioning window."""
        revokeCmd = Clusters.AdministratorCommissioning.Commands.RevokeCommissioning()
        await th.SendCommand(
            nodeid=node_id,
            endpoint=0,
            payload=revokeCmd,
            timedRequestTimeoutMs=6000
        )
        # The failsafe cleanup is scheduled after the command completes
        sleep(1)

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
                logging.info(f"  {service}")
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

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

from mdns_discovery import mdns_discovery
from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.interaction_model import Status
from matter.testing.matter_testing import MatterBaseTest


class CADMINBaseTest(MatterBaseTest):
    async def get_fabrics(self, th: ChipDeviceCtrl, fabric_filtered: bool = True) -> int:
        """Get fabrics information from the device."""
        OC_cluster = Clusters.OperationalCredentials
        fabric_info = await self.read_single_attribute_check_success(
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
        current_fabric_index = await self.read_single_attribute_check_success(
            dev_ctrl=th,
            endpoint=0,
            cluster=cluster,
            attribute=attribute
        )
        return current_fabric_index

    async def write_nl_attr(self, dut_node_id: int, th: ChipDeviceCtrl, attr_val: object):
        result = await th.WriteAttribute(nodeId=dut_node_id, attributes=[(0, attr_val)])
        asserts.assert_equal(result[0].Status, Status.Success, f"{th} node label write failed")

    async def read_nl_attr(self, dut_node_id: int, th: ChipDeviceCtrl, attr_val: object):
        try:
            await th.ReadAttribute(nodeId=dut_node_id, attributes=[(0, attr_val)])
        except Exception as e:
            asserts.assert_equal(e.err, "Received error message from read attribute attempt")
            self.print_step(0, e)

    async def get_window_status(self, th: ChipDeviceCtrl) -> int:
        """Get the current commissioning window status."""
        AC_cluster = Clusters.AdministratorCommissioning
        window_status = await self.read_single_attribute_check_success(
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
            nodeId=node_id,
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
            cm_value = self.service.txt.get('CM')
            if cm_value is not None:
                try:
                    self.cm = int(cm_value)
                except (ValueError, TypeError):
                    logging.warning(f"Could not convert CM value '{cm_value}' to integer")
                    self.cm = None

            # Safely convert D value to int if present
            d_value = self.service.txt.get('D')
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

    async def wait_for_correct_cm_value(self, expected_cm_value: int, expected_discriminator: int, max_attempts: int = 5, delay_sec: int = 5):
        """Wait for the correct CM value and discriminator in DNS-SD with retries."""
        for attempt in range(max_attempts):
            discovery = mdns_discovery.MdnsDiscovery()
            raw_services = await discovery.get_commissionable_services(discovery_timeout_sec=240, log_output=True)

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

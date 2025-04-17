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
from time import sleep

from mdns_discovery import mdns_discovery
from mobly import asserts

import matter.clusters as Clusters
from matter import ChipDeviceCtrl
from matter.ChipDeviceCtrl import CommissioningParameters
from matter.interaction_model import Status


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

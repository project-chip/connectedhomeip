#
#    Copyright (c) 2023 Project CHIP Authors
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

import dataclasses
import ipaddress

from chip import ChipDeviceCtrl, commissioning, discovery


@dataclasses.dataclass
class Session:
    node_id: int
    device: ChipDeviceCtrl.DeviceProxyWrapper


class ContextManager:
    def __init__(self, devCtrl: ChipDeviceCtrl.ChipDeviceControllerBase, node_id: int, is_ble: bool):
        self.devCtrl = devCtrl
        self.node_id = node_id
        self.is_ble = is_ble

    def __enter__(self) -> Session:
        return Session(
            node_id=self.node_id,
            device=self.devCtrl.GetConnectedDeviceSync(self.node_id, allowPASE=True, timeoutMs=1000))

    def __exit__(self, type, value, traceback):
        self.devCtrl.MarkSessionDefunct(self.node_id)
        if self.is_ble:
            self.devCtrl.CloseBLEConnection(self.is_ble)


async def establish_session(devCtrl: ChipDeviceCtrl.ChipDeviceControllerBase, parameter: commissioning.PaseParameters) -> ContextManager:
    if isinstance(parameter, commissioning.PaseOverBLEParameters):
        await devCtrl.EstablishPASESessionBLE(parameter.setup_pin, parameter.discriminator, parameter.temporary_nodeid)
    elif isinstance(parameter, commissioning.PaseOverIPParameters):
        device = await devCtrl.DiscoverCommissionableNodes(filterType=discovery.FilterType.LONG_DISCRIMINATOR,
                                                           filter=parameter.long_discriminator, stopOnFirst=True)
        if not device:
            raise ValueError("No commissionable device found")
        selected_address = None
        for ip in device[0].addresses:
            if ipaddress.ip_address(ip).is_link_local:
                # TODO(erjiaqing): To connect a device using link local address requires an interface identifier,
                # however, the link local address returned from DiscoverCommissionableNodes does not have an
                # interface identifier.
                continue
            selected_address = ip
            break
        if selected_address is None:
            raise ValueError("The node for commissioning does not contains routable ip addresses information")
        await devCtrl.EstablishPASESessionIP(selected_address, parameter.setup_pin, parameter.temporary_nodeid)
    else:
        raise TypeError("Expect PaseOverBLEParameters or PaseOverIPParameters for establishing PASE session")
    return ContextManager(
        devCtrl=devCtrl, node_id=parameter.temporary_nodeid, is_ble=isinstance(parameter, commissioning.PaseOverBLEParameters))

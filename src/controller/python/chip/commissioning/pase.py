#
# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
        self.devCtrl.CloseSession(self.node_id)
        if self.is_ble:
            self.devCtrl.CloseBLEConnection(self.is_ble)


def establish_session(devCtrl: ChipDeviceCtrl.ChipDeviceControllerBase, parameter: commissioning.PaseParameters) -> ContextManager:
    if isinstance(parameter, commissioning.PaseOverBLEParameters):
        devCtrl.EstablishPASESessionBLE(parameter.setup_pin, parameter.discriminator, parameter.temporary_nodeid)
    elif isinstance(parameter, commissioning.PaseOverIPParameters):
        device = devCtrl.DiscoverCommissionableNodes(filterType=discovery.FilterType.LONG_DISCRIMINATOR,
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
        devCtrl.EstablishPASESessionIP(selected_address, parameter.setup_pin, parameter.temporary_nodeid)
    else:
        raise TypeError("Expect PaseOverBLEParameters or PaseOverIPParameters for establishing PASE session")
    return ContextManager(
        devCtrl=devCtrl, node_id=parameter.temporary_nodeid, is_ble=isinstance(parameter, commissioning.PaseOverBLEParameters))

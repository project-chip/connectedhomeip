# Copyright (c) 2009-2021 Arm Limited
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import logging
from time import sleep

import pytest
from button_service import button_service_pb2
from chip import ChipDeviceCtrl
from common.pigweed_client import PigweedClient
from common.utils import (check_chip_ble_devices_advertising, close_ble, close_connection, commissioning_wifi,
                          connect_device_over_ble, get_device_details, resolve_device, send_zcl_command)
from device_service import device_service_pb2
from locking_service import locking_service_pb2
from pw_status import Status

log = logging.getLogger(__name__)

BLE_DEVICE_NAME = "MBED-lock"
DEVICE_NODE_ID = 1234
RPC_PROTOS = [device_service_pb2, button_service_pb2, locking_service_pb2]


@pytest.mark.smoketest
def test_smoke_test(device):
    device.reset(duration=1)
    ret = device.wait_for_output("Mbed lock-app example application start")
    assert ret is not None and len(ret) > 0
    ret = device.wait_for_output("Mbed lock-app example application run")
    assert ret is not None and len(ret) > 0


def test_wifi_provisioning(device, network):
    network_ssid = network[0]
    network_pass = network[1]

    devCtrl = ChipDeviceCtrl.ChipDeviceController()

    device_details = get_device_details(device)
    assert device_details is not None and len(device_details) != 0

    assert check_chip_ble_devices_advertising(
        devCtrl, BLE_DEVICE_NAME, device_details)

    ret = connect_device_over_ble(devCtrl, int(device_details["Discriminator"]), int(
        device_details["SetUpPINCode"]), DEVICE_NODE_ID)
    assert ret is not None and ret == DEVICE_NODE_ID

    ret = device.wait_for_output("Device completed Rendezvous process")
    assert ret is not None and len(ret) > 0

    ret = commissioning_wifi(devCtrl, network_ssid,
                             network_pass, DEVICE_NODE_ID)
    assert ret == 0

    ret = device.wait_for_output("StationConnected")
    assert ret is not None and len(ret) > 0

    ret = device.wait_for_output("address set")
    assert ret is not None and len(ret) > 0

    device_ip_address = ret[-1].partition("address set:")[2].strip()

    ret = resolve_device(devCtrl, DEVICE_NODE_ID)
    assert ret is not None and len(ret) == 2

    ip_address = ret[0]

    assert device_ip_address == ip_address

    assert close_connection(devCtrl, DEVICE_NODE_ID)
    assert close_ble(devCtrl)


def test_lock_ctrl(device, network):
    network_ssid = network[0]
    network_pass = network[1]

    devCtrl = ChipDeviceCtrl.ChipDeviceController()

    device_details = get_device_details(device)
    assert device_details is not None and len(device_details) != 0

    assert check_chip_ble_devices_advertising(
        devCtrl, BLE_DEVICE_NAME, device_details)

    ret = connect_device_over_ble(devCtrl, int(device_details["Discriminator"]), int(
        device_details["SetUpPINCode"]), DEVICE_NODE_ID)
    assert ret is not None and ret == DEVICE_NODE_ID

    ret = device.wait_for_output("Device completed Rendezvous process")
    assert ret is not None and len(ret) > 0

    ret = commissioning_wifi(devCtrl, network_ssid,
                             network_pass, DEVICE_NODE_ID)
    assert ret == 0

    ret = resolve_device(devCtrl, DEVICE_NODE_ID)
    assert ret is not None and len(ret) == 2

    err, res = send_zcl_command(
        devCtrl, "OnOff Off {} 1 0".format(DEVICE_NODE_ID))
    assert err == 0

    ret = device.wait_for_output("Unlock Action has been completed", 20)
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(
        devCtrl, "OnOff On {} 1 0".format(DEVICE_NODE_ID))
    assert err == 0

    ret = device.wait_for_output("Lock Action has been completed", 20)
    assert ret is not None and len(ret) > 0

    err, res = send_zcl_command(
        devCtrl, "OnOff Toggle {} 1 0".format(DEVICE_NODE_ID))
    assert err == 0

    ret = device.wait_for_output("Unlock Action has been completed", 20)
    assert ret is not None and len(ret) > 0

    assert close_connection(devCtrl, DEVICE_NODE_ID)
    assert close_ble(devCtrl)


def test_device_info_rpc(device):
    with PigweedClient(device, RPC_PROTOS) as pw_client:
        status, payload = pw_client.rpcs.chip.rpc.Device.GetDeviceInfo()
        assert status.ok() is True
        assert payload.vendor_id is not None and payload.product_id is not None and payload.serial_number is not None

        device_details = get_device_details(device)
        assert device_details is not None and len(device_details) != 0

        assert int(device_details["VendorID"]) == payload.vendor_id
        assert int(device_details["ProductID"]) == payload.product_id
        assert int(device_details["Discriminator"]
                   ) == payload.pairing_info.discriminator
        assert int(device_details["SetUpPINCode"]) == payload.pairing_info.code


def test_device_factory_reset_rpc(device):
    with PigweedClient(device, RPC_PROTOS) as pw_client:
        status, payload = pw_client.rpcs.chip.rpc.Device.FactoryReset()
        assert status.ok() is True


def test_device_reboot_rpc(device):
    with PigweedClient(device, RPC_PROTOS) as pw_client:
        status, payload = pw_client.rpcs.chip.rpc.Device.Reboot()
        assert status == Status.UNIMPLEMENTED


def test_device_ota_rpc(device):
    with PigweedClient(device, RPC_PROTOS) as pw_client:
        status, payload = pw_client.rpcs.chip.rpc.Device.TriggerOta()
        assert status == Status.UNIMPLEMENTED


def test_lock_ctrl_rpc(device):
    with PigweedClient(device, RPC_PROTOS) as pw_client:

        # Check locked
        status, payload = pw_client.rpcs.chip.rpc.Locking.Set(locked=True)
        assert status.ok() is True
        status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
        assert status.ok() is True
        assert payload.locked is True

        # Check unlocked
        status, payload = pw_client.rpcs.chip.rpc.Locking.Set(locked=False)
        assert status.ok() is True
        status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
        assert status.ok() is True
        assert payload.locked is False


def test_button_ctrl_rpc(device):
    with PigweedClient(device, RPC_PROTOS) as pw_client:

        # Check button 0 (locking)
        status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
        assert status.ok() is True
        initial_state = bool(payload.locked)

        compare_state = not initial_state
        status, payload = pw_client.rpcs.chip.rpc.Button.Event(idx=0,
                                                               pushed=True)
        assert status.ok() is True
        sleep(2)
        status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
        assert status.ok() is True
        assert payload.locked is compare_state

        compare_state = initial_state
        status, payload = pw_client.rpcs.chip.rpc.Button.Event(idx=0,
                                                               pushed=True)
        assert status.ok() is True
        sleep(2)
        status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
        assert status.ok() is True
        assert payload.locked == compare_state

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

import pytest
import re
from time import sleep

from chip.setup_payload import SetupPayload
from chip import exceptions
from chip import ChipDeviceCtrl

from common.utils import scan_chip_ble_devices, connect_device_over_ble, commissioning_wifi, close_ble

from common.pigweed_client import PigweedClient
from device_service import device_service_pb2
from button_service import button_service_pb2
from locking_service import locking_service_pb2
from pw_status import Status

import logging
log = logging.getLogger(__name__)

BLE_DEVICE_NAME = "MBED-lock"
DEVICE_NODE_ID = 1234
RPC_PROTOS = [device_service_pb2, button_service_pb2, locking_service_pb2]
DEVICE_TEST_SERIAL_NUMBER = "TEST_SN"

@pytest.mark.smoketest
def test_smoke_test(device):
    device.reset(duration=1)
    ret = device.wait_for_output("Mbed lock-app example application start")
    assert ret != None and len(ret) > 0
    ret = device.wait_for_output("Mbed lock-app example application run")
    assert ret != None and len(ret) > 0


def test_wifi_provisioning(device, network):
    network_ssid = network[0]
    network_pass = network[1]

    devCtrl = ChipDeviceCtrl.ChipDeviceController()
    
    ret = device.wait_for_output("SetupQRCode")
    assert ret != None and len(ret) > 1

    qr_code = re.sub(r"[\[\]]", "", ret[-1].partition("SetupQRCode:")[2]).strip()
    try:
        device_details = dict(SetupPayload().ParseQrCode("VP:vendorpayload%{}".format(qr_code)).attributes)
    except exceptions.ChipStackError as ex:
        log.error(ex.msg)
        assert False
    assert device_details != None and len(device_details) != 0

    ble_chip_device = scan_chip_ble_devices(devCtrl, BLE_DEVICE_NAME)
    assert ble_chip_device != None and len(ble_chip_device) != 0

    chip_device_found = False

    for ble_device in ble_chip_device:
        if (int(ble_device.discriminator) == int(device_details["Discriminator"]) and
            int(ble_device.vendorId) == int(device_details["VendorID"]) and
            int(ble_device.productId) == int(device_details["ProductID"])):
            chip_device_found = True
            break

    assert chip_device_found

    ret = connect_device_over_ble(devCtrl, int(device_details["Discriminator"]), int(device_details["SetUpPINCode"]), DEVICE_NODE_ID)
    assert ret != None and ret == DEVICE_NODE_ID

    ret = device.wait_for_output("Device completed Rendezvous process")
    assert ret != None and len(ret) > 0

    ret = commissioning_wifi(devCtrl, network_ssid, network_pass, DEVICE_NODE_ID)
    assert ret == 0

    ret = device.wait_for_output("Event - StationConnected")
    assert ret != None and len(ret) > 0

    assert close_ble(devCtrl)

def test_device_info_rpc(device):
    pw_client = PigweedClient(device, RPC_PROTOS)
    status, payload = pw_client.rpcs.chip.rpc.Device.GetDeviceInfo()
    assert status.ok() == True
    assert payload.vendor_id != None and payload.product_id != None and payload.serial_number != None

    ret = device.wait_for_output("SetupQRCode")
    assert ret != None and len(ret) > 1

    qr_code = re.sub(r"[\[\]]", "", ret[-1].partition("SetupQRCode:")[2]).strip()
    try:
        device_details = dict(SetupPayload().ParseQrCode("VP:vendorpayload%{}".format(qr_code)).attributes)
    except exceptions.ChipStackError as ex:
        log.error(ex.msg)
        assert False
    assert device_details != None and len(device_details) != 0

    assert int(device_details["VendorID"]) == payload.vendor_id
    assert int(device_details["ProductID"]) == payload.product_id
    assert DEVICE_TEST_SERIAL_NUMBER == payload.serial_number

def test_device_factory_reset_rpc(device):
    pw_client = PigweedClient(device, RPC_PROTOS)
    status, payload = pw_client.rpcs.chip.rpc.Device.FactoryReset()
    assert status.ok() == True

def test_device_reboot_rpc(device):
    pw_client = PigweedClient(device, RPC_PROTOS)
    status, payload = pw_client.rpcs.chip.rpc.Device.Reboot()
    assert status == Status.UNIMPLEMENTED

def test_device_ota_rpc(device):
    pw_client = PigweedClient(device, RPC_PROTOS)
    status, payload = pw_client.rpcs.chip.rpc.Device.TriggerOta()
    assert status == Status.UNIMPLEMENTED

def test_lock_ctrl_rpc(device):
    pw_client = PigweedClient(device, RPC_PROTOS)
    
    # Check locked
    status, payload = pw_client.rpcs.chip.rpc.Locking.Set(locked=True)
    assert status.ok() == True
    status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
    assert status.ok() == True
    assert payload.locked == True

     # Check unlocked
    status, payload = pw_client.rpcs.chip.rpc.Locking.Set(locked=False)
    assert status.ok() == True
    status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
    assert status.ok() == True
    assert payload.locked == False

def test_button_ctrl_rpc(device):
    pw_client = PigweedClient(device, RPC_PROTOS)

    # Check button 0 (locking)
    status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
    assert status.ok() == True
    initial_state = bool(payload.locked)

    compare_state = not initial_state
    status, payload = pw_client.rpcs.chip.rpc.Button.Event(idx=0, pushed=True)
    assert status.ok() == True
    sleep(0.5)
    status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
    assert status.ok() == True
    assert payload.locked == compare_state

    compare_state = initial_state
    status, payload = pw_client.rpcs.chip.rpc.Button.Event(idx=0, pushed=True)
    assert status.ok() == True
    sleep(0.5)
    status, payload = pw_client.rpcs.chip.rpc.Locking.Get()
    assert status.ok() == True
    assert payload.locked == compare_state
#!/usr/bin/env python3
"""
Copyright (c) 2020 Project CHIP Authors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import logging
import os
import re
import requests
import time
import traceback
import sys
import ipaddress

from helper.CHIPTestBase import CHIPVirtualHome

import cirque.proto.capability_pb2 as capability_pb2
import cirque.proto.device_pb2 as device_pb2
import cirque.proto.service_pb2 as service_pb2
import cirque.proto.service_pb2_grpc as service_pb2_grpc

from cirque.proto.device_pb2 import DeviceSpecification

from cirque.common.cirquelog import CirqueLog
from cirque.common.utils import sleep_time

from cirque.proto.capability_pb2 import (
    WeaveCapability, ThreadCapability, WiFiCapability, XvncCapability,
    InteractiveCapability, LanAccessCapability
)

logger = logging.getLogger('CHIPOnOffTest')
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
logger.addHandler(sh)

DEVICE_CONFIG = {
    'device0': {
        'device_type': 'CHIP-Server',
        'base_image': 'chip_server',
        'wifi_capability': WiFiCapability(),
        'thread_capability': ThreadCapability(rcp_mode=True),
        'interactive_capability': InteractiveCapability()
    },
    'device1': {
        'device_type': 'CHIP-Tool',
        'base_image': 'chip_tool',
        'wifi_capability': WiFiCapability(),
        'thread_capability': ThreadCapability(rcp_mode=True),
        'interactive_capability': InteractiveCapability()
    },
    'wifi-ap': {
        'device_type': 'wifi_ap',
        'base_image': 'mac80211_ap_image',
    }
}

CHIP_PORT = 11095

class TestOnOffCluster(CHIPVirtualHome):
    def __init__(self, device_config):
        super().__init__(device_config)
        self.logger = logger
        self.logger.info('TestOnOffCluster')
    
    def setup(self):
        self.setUpClass()
        self.create_home()
        self.enable_wifi_on_device()
        self.connect_to_thread_network()
    
    def teardown(self):
        self.tearDownClass()
    
    def test_routine(self):
        self.run_data_model_test()
    
    def run_data_model_test(self):
        server_ip_address = set()
        tool_device_id = ''

        server_ids = [
            device.device_id for device in self.non_ap_devices
            if device.device_specification.device_type == "CHIP-Server"]
        tool_ids = [
            device.device_id for device in self.non_ap_devices
            if device.device_specification.device_type == "CHIP-Tool"]

        tool_device_id = tool_ids[0]
        
        for device_id in server_ids:
            server_ip_address.add(self.get_device_thread_ip(device_id))
        
        for ip in server_ip_address:
            self.stub.ExecuteDeviceCommand(
                service_pb2.ExecuteDeviceCommandRequest(
                    home_id=self.home_id,
                    device_id=tool_device_id,
                    command="chip-standalone-demo.out on {} {} 1".format(ip, CHIP_PORT),
                    streaming=False))
        time.sleep(1)
        for ip in server_ip_address:
            self.stub.ExecuteDeviceCommand(
                service_pb2.ExecuteDeviceCommandRequest(
                    home_id=self.home_id,
                    device_id=tool_device_id,
                    command="chip-standalone-demo.out off {} {} 1".format(ip, CHIP_PORT),
                    streaming=False))

        for device_id in server_ids:
            self.assertTrue(self.sequenceMatch(self.get_device_log(device_id).log, ["CHIP:ZCL: RX: On", "CHIP:ZCL: RX: Off"]),
                    "Datamodel test failed: cannot find matching string from device {}".format(device_id))

def do_test():
    test_success = True
    test = TestOnOffCluster(DEVICE_CONFIG)
    test.setup()
    try:
        test.test_routine()
    except:
        traceback.print_exc(file=sys.stdout)
        test_success = False
        pass
    try:
        test.save_device_logs()
    except:
        traceback.print_exc(file=sys.stdout)
        test_success = False
        pass
    test.teardown()
    return test_success

if __name__ == "__main__":
    sys.exit(0 if do_test() else 1)

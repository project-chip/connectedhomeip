# Lint as: python3
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
import time
import ipaddress

import grpc
from google.rpc import code_pb2, status_pb2

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

class CHIPVirtualHome:
    def __init__(self, device_config):
        self.home_id = None
        self.device_config = device_config
        self.device_ids = set()
        self.devices = []
        self.non_ap_devices = []
        self.ap_devices = []
    
    def sequenceMatch(self, string, patterns):
        last_find = 0
        for s in patterns:
            self.logger.info('Finding {}'.format(s))
            this_find = string.find(s, last_find)
            if this_find < 0:
                self.logger.info('Cannot find!')
                return False
            self.logger.info("Found at index={}".format(this_find))
            last_find = this_find + len(s)
        return True

    def assertTrue(self, exp, note = None):
        if not exp:
            if note:
                self.logger.error(note)
            raise RuntimeError
    
    def assertEqual(self, val1, val2, note = None):
        if not val1 == val2:
            if note:
                self.logger.error(note)
            raise RuntimeError

    def assertNotEqual(self, val1, val2, note = None):
        if val1 == val2:
            if note:
                self.logger.error(note) 
            raise RuntimeError

    def tearDownClass(self):

        self.logger.info("tearing down test class")
        self.logger.info("stopping home: {}".format(self.home_id))
        self.stub.StopCirqueHome(service_pb2.StopCirqueHomeRequest(
                home_id=self.home_id))

        self.channel.close()

    def setUpClass(self):
        self.channel = grpc.insecure_channel('localhost:50051')
        self.stub = service_pb2_grpc.CirqueServiceStub(self.channel)
        
        if hasattr(self, 'logger') and self.logger:
            return
        self.logger = CirqueLog.get_cirque_logger('CHIPVirtualHome')
        sh = logging.StreamHandler()
        sh.setFormatter(
            logging.Formatter(
                '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
        self.logger.addHandler(sh)
    
    def create_home(self):
        home_id = self.stub.CreateCirqueHome(
            service_pb2.CreateCirqueHomeRequest()).home_id

        self.logger.info("home id: {} created!".format(home_id))

        self.assertTrue(
            home_id in self.stub.ListCirqueHomes(
                service_pb2.ListCirqueHomesRequest()).home_id,
            "created home_id could not find in the cirque service!!")
        
        self.home_id = home_id

        device_ids = set()

        for device in self.device_config:
            device_id = self.stub.CreateCirqueDevice(
                service_pb2.CreateCirqueDeviceRequest(
                    home_id=home_id,
                    specification=device_pb2.DeviceSpecification(
                        **self.device_config[device]
                    ))).device.device_id
            device_ids.add(device_id)
        
        devices = self.stub.ListCirqueHomeDevices(
            service_pb2.ListCirqueHomeDevicesRequest(
                home_id=home_id))

        device_ids_from_request = set([device.device_id
                                       for device in devices.devices])
        self.assertTrue(
            device_ids == device_ids_from_request,
            "device created did not match from devices query command!")
        
        self.devices = [device for device in devices.devices]
        self.non_ap_devices = [d for d in devices.devices
                   if d.device_specification.device_type != 'wifi_ap']
    
    def connect_to_thread_network(self):
        self.logger.info("Running commands to form Thread network")
        for device in self.non_ap_devices:
            self.stub.ExecuteDeviceCommand(
                service_pb2.ExecuteDeviceCommandRequest(
                    home_id=self.home_id,
                    device_id=device.device_id,
                    command="bash -c 'ot-ctl panid 0x1234 && \
                             ot-ctl ifconfig up && \
                             ot-ctl thread start'"))
        self.logger.info("Waiting for Thread network to be formed...")
        time.sleep(10)
        roles = set()
        for device in self.non_ap_devices:
            reply = self.stub.ExecuteDeviceCommand(
                service_pb2.ExecuteDeviceCommandRequest(
                    home_id=self.home_id,
                    device_id=device.device_id,
                    command="ot-ctl state"))
            roles.add(reply.output.split()[0])
        self.assertTrue('leader' in roles)
        self.assertTrue('router' in roles or 'child' in roles)
        self.logger.info("Thread network formed")
    
    def enable_wifi_on_device(self):
        ssid, psk = [
            (device.device_description.ssid, device.device_description.psk)
            for device in self.devices
            if device.device_specification.device_type == 'wifi_ap'][0]

        self.logger.info("wifi ap ssid: {}, psk: {}".format(ssid, psk))

        for device in self.non_ap_devices:
            self.logger.info(
                "device id: {} connecting to desired ssid: {}".format(
                    device.device_id, ssid))
            write_psk_to_wpa_supplicant_config(
                self.logger, self.stub, self.home_id, device.device_id, ssid, psk)

            kill_existing_wpa_supplicant(
                self.logger, self.stub, self.home_id, device.device_id)

            start_wpa_supplicant(
                self.logger, self.stub, self.home_id, device.device_id)
        time.sleep(5)

    def get_device_thread_ip(self, device_id):
        ret = self.stub.ExecuteDeviceCommand(
                service_pb2.ExecuteDeviceCommandRequest(
                    home_id=self.home_id,
                    device_id=device_id,
                    command="ot-ctl ipaddr",
                    streaming=False))
        ipaddr_list = ret.output.splitlines()
        for ipstr in ipaddr_list:
            try:
                self.logger.info("device id: {} thread ip: {}".format(device_id, ipstr))
                ipaddr = ipaddress.ip_address(ipstr)
                if ipaddr.is_link_local:
                    self.logger.info("ignore ip: {} : link local".format(ipstr))
                    continue
                if not ipaddr.is_private:
                    self.logger.info("ignore ip: {} : global ip addr".format(ipstr))
                    continue
                if re.match("fd[0-9a-f]{2}:[0-9a-f]{4}:[0-9a-f]{4}:[0-9a-f]{4}:0000:00ff:fe00:[0-9a-f]{4}", ipaddr.exploded) != None:
                    self.logger.info("ignore ip: {} : not for app".format(ipstr))
                    continue
                self.logger.info("return ip: {}".format(ipstr))
                return str(ipaddr)
            except:
                pass
        return None
    
    def get_device_log(self, device_id, tail=0):
        if tail == 0:
            # Cirque contains a bug that it cannot pass all to docker since tail is a uint32
            # Set it to INT32_MAX for maximum lines of logs
            tail = 2147483647
        return self.stub.GetCirqueDeviceLog(service_pb2.GetCirqueDeviceLogRequest(
            home_id=self.home_id, device_id=device_id, tail=tail))
        
    def save_device_logs(self):
        timestamp = int(time.time())
        if not os.path.exists("logs"):
            os.makedirs("logs")

        for device in self.non_ap_devices:
            ret_log = self.get_device_log(device.device_id)
            f_name = '{}-{}-{}.log'.format(device.device_specification.device_type, timestamp, device.device_id[-8:])
            self.logger.debug("device log name: \n{}".format(f_name))
            with open(os.path.join('logs', f_name), 'w') as fp:
                fp.write(ret_log.log)


def create_device_from_config():
    for device in DEVICE_CONFIG:
        yield DEVICE_CONFIG[device]


def write_psk_to_wpa_supplicant_config(
        logger, stub, home_id, device_id, ssid, psk):

    logger.info("device id: {} writing ssid, psk to wpa_supplicant config"
                .format(device_id))

    write_psk_command = "".join(
        ["sh -c 'wpa_passphrase {} {} >> ".format(ssid, psk),
         "/etc/wpa_supplicant/wpa_supplicant.conf'"])

    return stub.ExecuteDeviceCommand(
        service_pb2.ExecuteDeviceCommandRequest(
            home_id=home_id,
            device_id=device_id,
            command=write_psk_command,
            streaming=False))


def kill_existing_wpa_supplicant(logger, stub, home_id, device_id):

    logger.info("device id: {} kill existing wpa_supplicant"
                .format(device_id))

    kill_wpa_supplicant_command = 'killall wpa_supplicant'

    return stub.ExecuteDeviceCommand(
        service_pb2.ExecuteDeviceCommandRequest(
            home_id=home_id,
            device_id=device_id,
            command=kill_wpa_supplicant_command,
            streaming=False))


def start_wpa_supplicant(logger, stub, home_id, device_id):

    logger.info("device id: {} starting wpa_supplicant on device"
                .format(device_id))

    start_wpa_supplicant_command = "".join(
        ["wpa_supplicant -B -i wlan0 ",
         "-c /etc/wpa_supplicant/wpa_supplicant.conf ",
         "-f /var/log/wpa_supplicant.log -t -dd"])

    return stub.ExecuteDeviceCommand(
        service_pb2.ExecuteDeviceCommandRequest(
            home_id=home_id,
            device_id=device_id,
            command=start_wpa_supplicant_command,
            streaming=False))

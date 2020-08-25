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

import ipaddress
import logging
import os
import re
import requests
import sys
import time
import traceback
from enum import IntEnum
from urllib.parse import urljoin


class TestResult(IntEnum):
    OK = 0
    TEST_FAILURE = 1
    SYSTEM_FAILURE = 2


'''
CHIPVirtualHome is a base class for single home tests
child classes should implement:
- setup()
- test_routine()
- tear_down()
'''


class CHIPVirtualHome:
    def __init__(self, cirque_url, device_config):
        self.home_id = None
        self.logger = None
        self.cirque_url = cirque_url
        self.device_config = device_config
        self.device_ids = []
        self.devices = []
        self.non_ap_devices = []
        self.ap_devices = []

    # The entrance of the whole test
    def run_test(self, save_logs=True):
        test_ret = TestResult.OK
        try:
            self.setup()
            self.test_routine()
        except AssertionError:
            # AssertionError is thrown in self.assertXxx function
            test_ret = TestResult.TEST_FAILURE
            traceback.print_exc(file=sys.stderr)
        except Exception:
            # Other errors indicate a failure in system.
            test_ret = TestResult.SYSTEM_FAILURE
            traceback.print_exc(file=sys.stderr)
        if save_logs:
            try:
                self.save_device_logs()
            except:
                test_ret = TestResult.SYSTEM_FAILURE
                traceback.print_exc(file=sys.stderr)
        try:
            self.destroy_home()
        except:
            test_ret = TestResult.SYSTEM_FAILURE
            traceback.print_exc(file=sys.stderr)
        return test_ret

    def query_api(self, end_point, args=[], binary=False):
        ret = requests.get(self._build_request_url(end_point, args))
        if binary:
            return ret.content
        return ret.json()

    def execute_device_cmd(self, device_id, cmd, stream=False):
        ret = requests.get(self._build_request_url('device_cmd', [self.home_id, device_id, cmd]),
                           params={'stream': stream},
                           stream=stream)
        return ret if stream else ret.json()

    def sequenceMatch(self, string, patterns):
        last_find = 0
        for s in patterns:
            self.logger.info('Finding string: "{}"'.format(s))
            this_find = string.find(s, last_find)
            if this_find < 0:
                self.logger.info('String not found')
                return False
            self.logger.info("Found at index={}".format(this_find))
            last_find = this_find + len(s)
        return True

    def connect_to_thread_network(self):
        self.logger.info("Running commands to form Thread network")
        for device in self.non_ap_devices:
            self.execute_device_cmd(device['id'],
                                    "bash -c 'ot-ctl panid 0x1234 && \
                             ot-ctl ifconfig up && \
                             ot-ctl thread start'")
        self.logger.info("Waiting for Thread network to be formed...")
        time.sleep(10)
        roles = set()
        for device in self.non_ap_devices:
            reply = self.execute_device_cmd(device['id'], 'ot-ctl state')
            roles.add(reply['output'].split()[0])
        self.assertTrue('leader' in roles)
        self.assertTrue('router' in roles or 'child' in roles)
        self.logger.info("Thread network formed")

    def enable_wifi_on_device(self):
        ssid, psk = self.query_api('wifi_ssid_psk', [self.home_id])

        self.logger.info("wifi ap ssid: {}, psk: {}".format(ssid, psk))

        for device in self.non_ap_devices:
            self.logger.info(
                "device id: {} connecting to desired ssid: {}".format(
                    device['id'], ssid))
            self.write_psk_to_wpa_supplicant_config(device['id'], ssid, psk)
            self.kill_existing_wpa_supplicant(device['id'])
            self.start_wpa_supplicant(device['id'])
        time.sleep(5)

    def get_device_thread_ip(self, device_id):
        ret = self.execute_device_cmd(device_id, 'ot-ctl ipaddr')
        ipaddr_list = ret["output"].splitlines()
        for ipstr in ipaddr_list:
            try:
                self.logger.info(
                    "device id: {} thread ip: {}".format(device_id, ipstr))
                ipaddr = ipaddress.ip_address(ipstr)
                if ipaddr.is_link_local:
                    self.logger.info(
                        "ignore ip: {} : link local".format(ipstr))
                    continue
                if not ipaddr.is_private:
                    self.logger.info(
                        "ignore ip: {} : global ip addr".format(ipstr))
                    continue
                if re.match("fd[0-9a-f]{2}:[0-9a-f]{4}:[0-9a-f]{4}:[0-9a-f]{4}:0000:00ff:fe00:[0-9a-f]{4}", ipaddr.exploded) != None:
                    self.logger.info(
                        "ignore ip: {} : not for app".format(ipstr))
                    continue
                self.logger.info("return ip: {}".format(ipstr))
                return str(ipaddr)
            except ValueError:
                # Since we are using ot-ctl, which is a command line interface and it will append 'Done' to end of output
                pass
        return None

    def get_device_log(self, device_id):
        return self.query_api('device_log', [self.home_id, device_id], binary=True)

    def assertTrue(self, exp, note=None):
        '''
        assert{True|False}
        assert(Not)Equal
        python unittest style functions that raise exceptions when condition not met
        '''
        if not exp == True:
            if note:
                self.logger.error(note)
            raise AssertionError

    def assertFalse(self, exp, note=None):
        if not exp == False:
            if note:
                self.logger.error(note)
            raise AssertionError

    def assertEqual(self, val1, val2, note=None):
        if not val1 == val2:
            if note:
                self.logger.error(note)
            raise AssertionError

    def assertNotEqual(self, val1, val2, note=None):
        if val1 == val2:
            if note:
                self.logger.error(note)
            raise AssertionError

    def _build_request_url(self, end_point, args=[]):
        if len(args) == 0:
            return urljoin(self.cirque_url, end_point)
        return urljoin(self.cirque_url, "{}/{}".format(end_point, '/'.join([str(argv) for argv in args])))

    def destroy_home(self):
        self.logger.info("destroying home: {}".format(self.home_id))
        self.query_api('destroy_home', [self.home_id])

    def initialize_home(self):
        home_id = requests.post(
            self._build_request_url('create_home'), json=self.device_config).json()

        self.logger.info("home id: {} created!".format(home_id))

        self.assertTrue(home_id in
                        list(self.query_api('get_homes')),
                        "created home_id did not match id from get_homes!!")

        self.home_id = home_id

        device_types = set()
        created_devices = self.query_api('home_devices', [home_id])
        for device in created_devices.values():
            device_types.add(device['type'])

        wanted_device_types = set()
        for device in self.device_config.values():
            wanted_device_types.add(device['type'])

        self.assertEqual(device_types, wanted_device_types,
                         "created device does not match to device config!!")

        self.device_config = created_devices

        self.device_ids = [device_id for device_id in self.device_config]
        self.non_ap_devices = [device for device in self.device_config.values()
                               if device['type'] != 'wifi_ap']
        self.ap_devices = [device for device in self.device_config.values()
                           if device['type'] == 'wifi_ap']

    def save_device_logs(self):
        timestamp = int(time.time())
        if not os.path.exists("logs"):
            os.makedirs("logs")

        for device in self.non_ap_devices:
            ret_log = self.get_device_log(device['id'])
            # Use this format for easier sort
            f_name = '{}-{}-{}.log'.format(device['type'],
                                           timestamp, device['id'][-8:])
            self.logger.debug("device log name: \n{}".format(f_name))
            with open(os.path.join('logs', f_name), 'wb') as fp:
                fp.write(ret_log)

    def start_wpa_supplicant(self, device_id):
        self.logger.info("device id: {}: starting wpa_supplicant on device"
                         .format(device_id))

        start_wpa_supplicant_command = "".join(
            ["wpa_supplicant -B -i wlan0 ",
             "-c /etc/wpa_supplicant/wpa_supplicant.conf ",
             "-f /var/log/wpa_supplicant.log -t -dd"])

        return self.execute_device_cmd(device_id, start_wpa_supplicant_command)

    def write_psk_to_wpa_supplicant_config(self, device_id, ssid, psk):
        self.logger.info("device id: {}: writing ssid, psk to wpa_supplicant config"
                         .format(device_id))

        write_psk_command = "".join(
            ["sh -c 'wpa_passphrase {} {} >> ".format(ssid, psk),
             "/etc/wpa_supplicant/wpa_supplicant.conf'"])

        return self.execute_device_cmd(device_id, write_psk_command)

    def kill_existing_wpa_supplicant(self, device_id):
        self.logger.info("device id: {}: kill existing wpa_supplicant"
                         .format(device_id))

        kill_wpa_supplicant_command = 'killall wpa_supplicant'

        return self.execute_device_cmd(device_id, kill_wpa_supplicant_command)

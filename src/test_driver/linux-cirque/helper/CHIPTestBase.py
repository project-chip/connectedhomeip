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
import json
import os
import re
import sys
import time
import traceback
from enum import IntEnum
from typing import List, Mapping, Union
from urllib.parse import urljoin

import requests


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
    def __init__(self, cirque_url, device_config: Mapping[str, dict]):
        self.home_id = None
        self.logger = None
        self.cirque_url = cirque_url
        self.device_config = device_config
        self.device_ids = []
        self.devices = []
        self.non_ap_devices = []
        self.thread_devices = []
        self.ap_devices = []

        for device in device_config.values():
            if device.get("base_image", "@default") == "@default":
                device["base_image"] = self.default_base_image

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
            except Exception:
                test_ret = TestResult.SYSTEM_FAILURE
                traceback.print_exc(file=sys.stderr)
        try:
            self.destroy_home()
        except requests.exceptions.RequestException:
            test_ret = TestResult.SYSTEM_FAILURE
            traceback.print_exc(file=sys.stderr)
        return test_ret

    def query_api(self, end_point, args=[], binary=False):
        ret = requests.get(self._build_request_url(end_point, args))
        if binary:
            return ret.content
        return ret.json()

    def execute_device_cmd(self, device_id, cmd, stream=False):
        self.logger.info(
            "device: {} exec: {}".format(self.get_device_pretty_id(device_id), cmd))
        ret = requests.get(self._build_request_url('device_cmd', [self.home_id, device_id, cmd]),
                           params={'stream': stream},
                           stream=stream)
        if stream:
            return ret

        ret_struct = ret.json()
        command_ret_code = ret_struct.get('return_code', None)
        if command_ret_code is None:
            # could be 0
            self.logger.error("cannot get command return code")
            raise Exception("cannot get command return code")
        self.logger.info(
            "command return code: {}".format(
                ret_struct.get('return_code', 'Unknown'))
        )
        command_output = ret_struct.get('output', None)
        if command_output is None:
            # could be empty string
            self.logger.error("cannot get command output")
            raise Exception("cannot get command output")
        self.logger.info(
            "command output: \n{}".format(ret_struct.get('output', ''))
        )
        return ret_struct

    def sequenceMatch(self, string, patterns):
        last_find = 0
        for s in patterns:
            self.logger.info('Finding string: "{}"'.format(s))
            this_find = string.find(s, last_find)
            if this_find < 0:
                self.logger.info('Not found')
                return False
            self.logger.info("Found at index={}".format(this_find))
            last_find = this_find + len(s)
        return True

    def reset_thread_devices(self, devices: Union[List[str], str]):
        """
        Reset device's thread settings and verify state.
        """
        if isinstance(devices, str):
            devices = [devices]
        for device_id in devices:
            # Wait for otbr-agent and CHIP server start
            self.assertTrue(self.wait_for_device_output(
                device_id, "Thread interface: wpan0", 10))
            self.assertTrue(self.wait_for_device_output(
                device_id, "[SVR] Server Listening...", 15))
            # Clear default Thread network commissioning data
            self.logger.info("Resetting thread network on {}".format(
                self.get_device_pretty_id(device_id)))
            self.execute_device_cmd(device_id, 'ot-ctl factoryreset')
            self.check_device_thread_state(
                device_id=device_id, expected_role="disabled", timeout=10)

    def check_device_thread_state(self, device_id, expected_role, timeout):
        if isinstance(expected_role, str):
            expected_role = [expected_role]
        self.logger.info(
            f"Waiting for expected role. {self.get_device_pretty_id(device_id)}: {expected_role}")
        start = time.time()
        while time.time() < (start + timeout):
            reply = self.execute_device_cmd(device_id, 'ot-ctl state')
            if reply['output'].split()[0] in expected_role:
                return
            time.sleep(0.5)

        self.logger.error(
            f"Device {self.get_device_pretty_id(device_id)} does not reach expected role")
        raise AssertionError

    def form_thread_network(self, device_id: str, expected_role: Union[str, List[str]], timeout: int = 15,
                            dataset: str = ""):
        """
        Start Thread Network with provided dataset. If dataset is not provided then default will be set.
        Function that will be also verifying if device start in expected role.
        """
        if not dataset:
            dataset = "0e080000000000010000" + \
                      "000300000c" + \
                      "35060004001fffe0" + \
                      "0208fedcba9876543210" + \
                      "0708fd00000000001234" + \
                      "0510ffeeddccbbaa99887766554433221100" + \
                      "030e54657374696e674e6574776f726b" + \
                      "0102d252" + \
                      "041081cb3b2efa781cc778397497ff520fa50c0302a0ff"

        ot_init_commands = [
            "ot-ctl thread stop",
            "ot-ctl ifconfig down",
            f"ot-ctl dataset set active {dataset}",
            "ot-ctl ifconfig up",
            "ot-ctl thread start",
            "ot-ctl dataset active",
        ]
        self.logger.info(
            f"Setting Thread dataset for {self.get_device_pretty_id(device_id)}: {dataset}")
        for cmd in ot_init_commands:
            self.execute_device_cmd(device_id, cmd)
        self.check_device_thread_state(
            device_id=device_id, expected_role=expected_role, timeout=timeout)

    def connect_to_thread_network(self):
        '''
        The dataset in this function is used to replace the default dataset generated by openthread.
        When the test writer is calling this function to setup a thread network, it means they just
        want a working IPv6 network or a working thread network and don't care about the detail of
        this network.
        '''
        self.logger.info("Running commands to form default Thread network")
        for device in self.thread_devices:
            self.wait_for_device_output(
                device['id'], "Border router agent started.", 5)

        otInitCommands = [
            "ot-ctl thread stop",
            "ot-ctl ifconfig down",
            ("ot-ctl dataset set active 0e080000000000010000000300000d35060004001fffe00208d"
             "ead00beef00cafe0708fd01234567890abc051000112233445566778899aabbccddeeff030a4f"
             "70656e546872656164010212340410ad463152f9622c7297ec6c6c543a63e70c0302a0ff"),
            "ot-ctl ifconfig up",
            "ot-ctl thread start",
            "ot-ctl dataset active",  # Emit
        ]
        for device in self.thread_devices:
            # Set default openthread provisioning
            for cmd in otInitCommands:
                self.execute_device_cmd(device['id'], cmd)
        self.logger.info("Waiting for Thread network to be formed...")
        threadNetworkFormed = False
        for i in range(30):
            roles = list()
            for device in self.thread_devices:
                # We can only check the status of ot-agent by query its state.
                reply = self.execute_device_cmd(device['id'], 'ot-ctl state')
                roles.append(reply['output'].split()[0])
            threadNetworkFormed = (roles.count('leader') == 1) and (roles.count(
                'leader') + roles.count('router') + roles.count('child') == len(self.thread_devices))
            if threadNetworkFormed:
                break
            time.sleep(1)
        self.assertTrue(threadNetworkFormed)
        self.logger.info("Thread network formed")

    def enable_wifi_on_device(self):
        ssid, psk = self.query_api('wifi_ssid_psk', [self.home_id])

        self.logger.info("wifi ap ssid: {}, psk: {}".format(ssid, psk))

        for device in self.non_ap_devices:
            self.logger.info(
                "device: {} connecting to desired ssid: {}".format(
                    self.get_device_pretty_id(device['id']), ssid))
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
                    "device: {} thread ip: {}".format(self.get_device_pretty_id(device_id), ipstr))
                ipaddr = ipaddress.ip_address(ipstr)
                if ipaddr.is_link_local:
                    continue
                if not ipaddr.is_private:
                    continue
                if re.match(("fd[0-9a-f]{2}:[0-9a-f]{4}:[0-9a-f]"
                             "{4}:[0-9a-f]{4}:0000:00ff:fe00:[0-9a-f]{4}"), ipaddr.exploded) is not None:
                    continue
                self.logger.info("Get Mesh-Local EID: {}".format(ipstr))
                return str(ipaddr)
            except ValueError:
                # Since we are using ot-ctl, which is a command line interface and it will append 'Done' to end of output
                pass
        return None

    def get_device_log(self, device_id):
        return self.query_api('device_log', [self.home_id, device_id], binary=True)

    def wait_for_device_output(self, device_id, pattern, timeout=1):
        due = time.time() + timeout
        while True:
            if self.sequenceMatch(self.get_device_log(device_id).decode(), [pattern, ]):
                return True
            if time.time() < due:
                time.sleep(1)
            else:
                break
        return False

    def assertTrue(self, exp, note=None):
        '''
        assert{True|False}
        assert(Not)Equal
        python unittest style functions that raise exceptions when condition not met
        '''
        if exp is not True:
            if note:
                self.logger.error(note)
            raise AssertionError

    def assertFalse(self, exp, note=None):
        if exp is not False:
            if note:
                self.logger.error(note)
            raise AssertionError

    def assertEqual(self, val1, val2, note=None):
        if not (val1 == val2):
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

        self.logger.info("home id: {} devices: {}".format(
            home_id, json.dumps(created_devices, indent=4, sort_keys=True)))

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
        self.thread_devices = [device for device in self.device_config.values()
                               if device['capability'].get('Thread', None) is not None]
        self.ap_devices = [device for device in self.device_config.values()
                           if device['type'] == 'wifi_ap']

    def save_device_logs(self):
        timestamp = int(time.time())
        log_dir = os.environ.get("DEVICE_LOG_DIR", None)
        if log_dir is not None and not os.path.exists(log_dir):
            os.makedirs("logs")

        for device in self.non_ap_devices:
            ret_log = self.get_device_log(device['id'])
            # Use this format for easier sort
            f_name = '{}-{}-{}.log'.format(device['type'],
                                           timestamp, device['id'][:8])
            self.logger.debug("device log name: \n{}".format(f_name))
            with open(os.path.join(log_dir, f_name), 'wb') as fp:
                fp.write(ret_log)

    def start_wpa_supplicant(self, device_id):
        self.logger.info("device: {}: starting wpa_supplicant on device"
                         .format(self.get_device_pretty_id(device_id)))

        start_wpa_supplicant_command = "".join(
            ["wpa_supplicant -B -i wlan0 ",
             "-c /etc/wpa_supplicant/wpa_supplicant.conf ",
             "-f /var/log/wpa_supplicant.log -t -dd"])

        return self.execute_device_cmd(device_id, start_wpa_supplicant_command)

    def write_psk_to_wpa_supplicant_config(self, device_id, ssid, psk):
        self.logger.info("device: {}: writing ssid, psk to wpa_supplicant config"
                         .format(self.get_device_pretty_id(device_id)))

        write_psk_command = "".join(
            ["sh -c 'wpa_passphrase {} {} >> ".format(ssid, psk),
             "/etc/wpa_supplicant/wpa_supplicant.conf'"])

        return self.execute_device_cmd(device_id, write_psk_command)

    def kill_existing_wpa_supplicant(self, device_id):
        self.logger.info("device: {}: kill existing wpa_supplicant"
                         .format(self.get_device_pretty_id(device_id)))

        kill_wpa_supplicant_command = 'killall wpa_supplicant'

        return self.execute_device_cmd(device_id, kill_wpa_supplicant_command)

    def get_device_pretty_name(self, device_id):
        device_obj = self.device_config.get(device_id, None)
        if device_obj is not None:
            return device_obj['type']
        return "<unknown>"

    def get_device_pretty_id(self, device_id):
        return "{}({}...)".format(self.get_device_pretty_name(device_id), device_id[:8])

    @property
    def default_base_image(cls):
        return os.environ.get("CHIP_CIRQUE_BASE_IMAGE", "project-chip/chip-cirque-device-base")

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

import asyncio
import ipaddress
import os
import typing

from capture.base import PlatformLogStreamer
from capture.file_utils import create_standard_log_name
from capture.shell_utils import Bash


class Android(PlatformLogStreamer):
    """
    Class that supports:
    - Running synchronous adb commands
    - Maintaining a singleton logcat stream
    - Maintaining a singleton screen recording
    """

    def __init__(self, artifact_dir: str) -> None:

        self.artifact_dir = artifact_dir

        self.device_id: str | None = None
        self.adb_devices: typing.Dict[str, bool] = {}
        self._authorize_adb()

        self.logcat_output_path = os.path.join(
            self.artifact_dir, create_standard_log_name(
                'logcat', 'txt'))
        self.logcat_command = f'adb -s {self.device_id} logcat -T 1 >> {self.logcat_output_path}'
        self.logcat_proc = Bash(self.logcat_command)

        screen_cast_name = create_standard_log_name('screencast', 'mp4')
        self.screen_cap_output_path = os.path.join(
            self.artifact_dir, screen_cast_name)
        self.check_screen_command = "shell dumpsys deviceidle | grep mScreenOn"
        self.screen_path = f'/sdcard/Movies/{screen_cast_name}'
        self.screen_command = f'adb -s {self.device_id} shell screenrecord --bugreport {self.screen_path}'
        self.screen_proc = Bash(self.screen_command)
        self.pull_screen = False
        self.screen_pull_command = f'pull {self.screen_path} {self.screen_cap_output_path}'

    def run_adb_command(
            self,
            command: str,
            capture_output: bool = False) -> Bash:
        """
        Run an adb command synchronously
        Capture_output must be true to call get_captured_output() later
        """
        return Bash(
            f'adb -s {self.device_id} {command}',
            sync=True,
            capture_output=capture_output)

    def get_adb_devices(self) -> typing.Dict[str, bool]:
        """Returns a dict of device ids and whether they are authorized"""
        adb_devices = Bash('adb devices', sync=True, capture_output=True)
        adb_devices_output = adb_devices.get_captured_output().split('\n')
        devices_auth = {}
        header_done = False
        for line in adb_devices_output:
            if header_done:
                line_parsed = line.split("\t")
                device_id = line_parsed[0]
                device_is_auth = line_parsed[1] == "device"
                if line_parsed[1] == "offline":
                    disconnect_command = f"adb disconnect {device_id}"
                    print(f"Device {device_id} is offline, trying disconnect!")
                    Bash(
                        disconnect_command,
                        sync=True,
                        capture_output=False)
                else:
                    devices_auth[device_id] = device_is_auth
            header_done = True
        self.adb_devices = devices_auth
        return self.adb_devices

    def _only_one_device_connected(self) -> bool:
        return len(self.adb_devices) == 1

    def _get_first_connected_device(self) -> str:
        return list(self.adb_devices.keys())[0]

    def _set_device_if_only_one_connected(self) -> None:
        if self._only_one_device_connected():
            self.device_id = self._get_first_connected_device()
            print(f'Only one device detected; using {self.device_id}')

    def _log_adb_devices(self) -> None:
        for dev in self.adb_devices:
            print(dev)

    @staticmethod
    def _is_connection_str(adb_input_str: str) -> bool:
        valid_ipv4 = False
        port_entered = False
        valid_port = False
        split_on_colon = adb_input_str.split(":")
        try:
            ipaddress.IPv4Network(split_on_colon[0])
            valid_ipv4 = True
        except ValueError:
            pass
        if len(split_on_colon) > 1:
            port_entered = True
            try:
                port = int(split_on_colon[1])
                valid_port = port < 65535
            except ValueError:
                pass
        valid_ip_no_port = valid_ipv4 and not port_entered
        valid_ip_valid_port = valid_ipv4 and valid_port
        return valid_ip_no_port or valid_ip_valid_port

    def _check_connect_wireless_adb(self, temp_device_id: str) -> None:
        if Android._is_connection_str(temp_device_id):
            connect_command = f"adb connect {temp_device_id}"
            print(
                f"Detected connection string; attempting to connect: {connect_command}")
            Bash(connect_command, sync=True, capture_output=False)
            self.get_adb_devices()

    def _device_id_user_input(self) -> None:
        print('If there is no output below, press enter after connecting your phone under test OR')
        print('Enter (copy paste) the target device id from the list of available devices below OR')
        print('Enter $IP4:$PORT to connect wireless debugging.')
        self._log_adb_devices()
        temp_device_id = input('').strip()
        self._check_connect_wireless_adb(temp_device_id)
        if self._only_one_device_connected():
            self._set_device_if_only_one_connected()
        elif temp_device_id not in self.adb_devices:
            print('Entered device not in adb devices!')
        else:
            self.device_id = temp_device_id

    def _choose_device_id(self) -> None:
        """
        Prompts the user to select a single device ID for this transport
        If only one device is ever connected, use it.
        """
        self._set_device_if_only_one_connected()
        while self.device_id not in self.get_adb_devices():
            self._device_id_user_input()
        print(f'Selected device {self.device_id}')

    def _authorize_adb(self) -> None:
        """
        Prompts the user until a single device is selected and adb is auth'd
        """
        self.get_adb_devices()
        self._choose_device_id()
        while not self.get_adb_devices()[self.device_id]:
            print('Confirming authorization, press enter after auth')
            input('')
        print(f'Target android device ID is authorized: {self.device_id}')

    def check_screen(self) -> bool:
        screen_cmd_output = self.run_adb_command(
            self.check_screen_command, capture_output=True)
        return "true" in screen_cmd_output.get_captured_output()

    async def prepare_screen_recording(self) -> None:
        if self.screen_proc.command_is_running():
            return
        try:
            async with asyncio.timeout_at(asyncio.get_running_loop().time() + 20.0):
                screen_on = self.check_screen()
                print("Please turn the screen on so screen recording can start!")
                while not screen_on:
                    await asyncio.sleep(2)
                    screen_on = self.check_screen()
                    if not screen_on:
                        print("Screen is still not on for recording!")
        except TimeoutError:
            print("WARNING screen recording timeout")
            return

    async def start_streaming(self) -> None:
        await self.prepare_screen_recording()
        if self.check_screen():
            self.pull_screen = True
            self.screen_proc.start_command()
        self.logcat_proc.start_command()

    async def pull_screen_recording(self) -> None:
        if self.pull_screen:
            self.screen_proc.stop_command()
            print("screen proc stopped")
            await asyncio.sleep(3)
            self.run_adb_command(self.screen_pull_command)
            print("screen recording pull attempted")
            self.pull_screen = False

    async def stop_streaming(self) -> None:
        await self.pull_screen_recording()
        self.logcat_proc.stop_command()
        print("logcat stopped")

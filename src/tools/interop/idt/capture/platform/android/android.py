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
import traceback
import typing
from asyncio import Task

from capture.base import PlatformLogStreamer
from utils.shell import Bash, log

from . import config, streams
from .capabilities import Capabilities

logger = log.get_logger(__file__)


class Android(PlatformLogStreamer):

    def __init__(self, artifact_dir: str) -> None:
        self.logger = logger
        self.artifact_dir = artifact_dir
        self.device_id: str | None = None
        self.adb_devices: typing.Dict[str, bool] = {}
        self.capabilities: None | Capabilities = None
        self.streams = {}
        self.connected = False

    def run_adb_command(
            self,
            command: str,
            capture_output: bool = False,
            cwd=None) -> Bash:
        """
        Run an adb command synchronously
        Capture_output must be true to call get_captured_output() later
        """
        bash_command = Bash(
            f'adb -s {self.device_id} {command}',
            sync=True,
            capture_output=capture_output,
            cwd=cwd)
        bash_command.start_command()
        return bash_command

    def get_adb_background_command(
            self,
            command: str,
            cwd=None) -> Bash:
        return Bash(f'adb -s {self.device_id} {command}', cwd=cwd)

    def get_adb_devices(self) -> typing.Dict[str, bool]:
        """Returns a dict of device ids and whether they are authorized"""
        adb_devices = Bash('adb devices', sync=True, capture_output=True)
        adb_devices.start_command()
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
                    self.logger.warning(f"Device {device_id} is offline, trying disconnect!")
                    Bash(
                        disconnect_command,
                        sync=True,
                        capture_output=False).start_command()
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
            self.logger.warning(f'Only one device detected; using {self.device_id}')

    def _log_adb_devices(self) -> None:
        for dev in self.adb_devices:
            self.logger.info(dev)

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
            self.logger.warning(
                f"Detected connection string; attempting to connect: {connect_command}")
            Bash(connect_command, sync=True, capture_output=False).start_command()
            self.get_adb_devices()

    def _device_id_user_input(self) -> None:
        self.logger.error('Connect additional android devices via USB and press enter OR')
        self.logger.error('Enter (copy paste) the target device id from the list of available devices below OR')
        self.logger.error('Enter $IP4:$PORT to connect wireless debugging.')
        self._log_adb_devices()
        temp_device_id = input('').strip()
        self._check_connect_wireless_adb(temp_device_id)
        self.get_adb_devices()
        if self._only_one_device_connected():
            self._set_device_if_only_one_connected()
        elif temp_device_id not in self.adb_devices:
            self.logger.warning('Entered device not in adb devices!')
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
        self.logger.info(f'Selected device {self.device_id}')

    def _authorize_adb(self) -> None:
        """
        Prompts the user until a single device is selected and adb is auth'd
        """
        self.get_adb_devices()
        self._choose_device_id()
        while not self.get_adb_devices()[self.device_id]:
            self.logger.info('Confirming authorization, press enter after auth')
            input('')
        self.logger.info(f'Target android device ID is authorized: {self.device_id}')

    async def connect(self) -> None:
        if not self.connected:
            self._authorize_adb()
            self.capabilities = Capabilities(self)
            self.capabilities.check_capabilities()
            for stream in streams.__all__:
                self.streams[stream] = getattr(streams, stream)(self)
            self.connected = True

    async def handle_stream_action(self, action: str) -> None:
        had_error = False
        for stream_name, stream in self.streams.items():
            self.logger.info(f"Doing {action} for {stream_name}!")
            try:
                await getattr(stream, action)()
            except Exception:
                self.logger.error(traceback.format_exc())
                had_error = True
        if had_error:
            raise Exception("Propagating to controller!")

    async def start_streaming(self) -> None:
        await self.handle_stream_action("start")

    async def run_observers(self) -> None:
        try:
            observer_tasks: [Task] = []
            for stream_name, stream in self.streams.items():
                observer_tasks.append(asyncio.create_task(stream.run_observer()))
            while True:
                self.logger.info("Android root observer task checking sub tasks")
                for task in observer_tasks:
                    if task.done() or task.cancelled():
                        self.logger.error(f"An android monitoring task has died, consider restarting! {task.__str__()}")
                await asyncio.sleep(30)
        except asyncio.CancelledError:
            self.logger.info("Cancelling observer tasks")
            for observer_tasks in observer_tasks:
                observer_tasks.cancel()

    async def stop_streaming(self) -> None:
        await self.handle_stream_action("stop")
        if config.enable_bug_report:
            found = False
            for item in os.listdir(self.artifact_dir):
                if "bugreport" in item and ".zip" in item:
                    found = True
            if not found:
                self.logger.info("Taking bugreport")
                self.run_adb_command("bugreport", cwd=self.artifact_dir)
            else:
                self.logger.warning("bugreport already taken")
        else:
            self.logger.critical("bugreport disabled in settings!")

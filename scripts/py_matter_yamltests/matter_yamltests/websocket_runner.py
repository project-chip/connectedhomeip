#
#    Copyright (c) 2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the 'License');
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an 'AS IS' BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import subprocess
import time
from dataclasses import dataclass

import websockets

from .hooks import WebSocketRunnerHooks
from .runner import TestRunner


@dataclass
class WebSocketRunnerConfig:
    server_address: str = 'localhost'
    server_port: int = '9002'
    server_path: str = None
    server_arguments: str = None
    hooks: WebSocketRunnerHooks = WebSocketRunnerHooks()


class WebSocketRunner(TestRunner):
    def __init__(self, config: WebSocketRunnerConfig):
        self._client = None
        self._server = None
        self._hooks = config.hooks

        self._server_connection_url = self._make_server_connection_url(
            config.server_address, config.server_port)
        self._server_startup_command = self._make_server_startup_command(
            config.server_path, config.server_arguments, config.server_port)

    async def start(self):
        self._server = await self._start_server(self._server_startup_command)
        self._client = await self._start_client(self._server_connection_url)

    async def stop(self):
        await self._stop_client(self._client)
        await self._stop_server(self._server)
        self._client = None
        self._server = None

    async def execute(self, request):
        instance = self._client
        if instance:
            await instance.send(request)
            return await instance.recv()
        return None

    async def _start_client(self, url, max_retries=4, interval_between_retries=1):
        if max_retries:
            start = time.time()
            try:
                self._hooks.connecting(url)
                connection = await websockets.connect(url)
                duration = round((time.time() - start) * 1000, 0)
                self._hooks.success(duration)
                return connection
            except Exception:
                duration = round((time.time() - start) * 1000, 0)
                self._hooks.failure(duration)
                self._hooks.retry(interval_between_retries)
                time.sleep(interval_between_retries)
                return await self._start_client(url, max_retries - 1, interval_between_retries + 1)

        self._hooks.abort(url)
        raise Exception(f'Connecting to {url} failed.')

    async def _stop_client(self, instance):
        if instance:
            await instance.close()

    async def _start_server(self, command):
        instance = None
        if command:
            instance = subprocess.Popen(command, stdout=subprocess.DEVNULL)
        return instance

    async def _stop_server(self, instance):
        if instance:
            instance.kill()

    def _make_server_connection_url(self, address: str, port: int):
        return 'ws://' + address + ':' + str(port)

    def _make_server_startup_command(self, path: str, arguments: str, port: int):
        if path is None:
            return None
        elif arguments is None:
            return [path] + ['--port', str(port)]
        else:
            return [path] + [arg.strip() for arg in arguments.split(' ')] + ['--port', str(port)]

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

import logging
import re
import select
import subprocess
import time
from dataclasses import dataclass

import websockets

from .hooks import WebSocketRunnerHooks
from .runner import TestRunner

_KEEP_ALIVE_TIMEOUT_IN_SECONDS = 120
_MAX_MESSAGE_SIZE_IN_BYTES = 10485760  # 10 MB
_CONNECT_MAX_RETRIES_DEFAULT = 4
_WEBSOCKET_SERVER_MESSAGE = '== WebSocket Server Ready'
_WEBSOCKET_SERVER_MESSAGE_TIMEOUT = 60  # seconds
_WEBSOCKET_SERVER_TERMINATE_TIMEOUT = 10  # seconds


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

    @property
    def is_connected(self) -> bool:
        if self._client is None:
            return False

        return self._client.state == websockets.protocol.State.OPEN

    async def start(self):
        self._server = await self._start_server(self._server_startup_command, self._server_connection_url)
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

    async def _start_client(self, url, max_retries=_CONNECT_MAX_RETRIES_DEFAULT, interval_between_retries=1):
        if max_retries:
            start = time.time()
            try:
                self._hooks.connecting(url)
                connection = await websockets.connect(url, ping_timeout=_KEEP_ALIVE_TIMEOUT_IN_SECONDS, max_size=_MAX_MESSAGE_SIZE_IN_BYTES)
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

    async def _start_server(self, command, url):
        instance = None
        if command:
            start_time = time.time()

            command = ['stdbuf', '-o0', '-e0'] + command  # disable buffering
            instance = subprocess.Popen(
                command, text=False, bufsize=0, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

            # Loop to read the subprocess output with a timeout
            lines = []
            while True:
                if time.time() - start_time > _WEBSOCKET_SERVER_MESSAGE_TIMEOUT:
                    for line in lines:
                        print(line.decode('utf-8'), end='')
                    self._hooks.abort(url)
                    await self._stop_server(instance)
                    raise Exception(
                        f'Connecting to {url} failed. WebSocket startup has not been detected.')

                ready, _, _ = select.select([instance.stdout], [], [], 1)
                if ready:
                    line = instance.stdout.readline()
                    if len(line):
                        lines.append(line)
                        if re.search(_WEBSOCKET_SERVER_MESSAGE, line.decode('utf-8')):
                            break  # Exit the loop if the pattern is found
                else:
                    continue
            instance.stdout.close()

        return instance

    async def _stop_server(self, instance):
        if instance:
            instance.terminate()  # sends SIGTERM
            try:
                instance.wait(_WEBSOCKET_SERVER_TERMINATE_TIMEOUT)
            except subprocess.TimeoutExpired:
                logging.debug(
                    'Subprocess did not terminate on SIGTERM, killing it now')
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

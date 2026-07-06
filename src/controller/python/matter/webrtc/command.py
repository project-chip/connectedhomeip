#
#    Copyright (c) 2020 Project CHIP Authors
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
import builtins
import ctypes

from ..clusters.ClusterObjects import ClusterCommand
from ..clusters.Command import (AsyncCommandTransaction, CommandPath, _OnCommandSenderDoneCallback, _OnCommandSenderErrorCallback,
                                _OnCommandSenderResponseCallback)
from ..native import PyChipError
from .library_handle import get_webrtc_provider_handle


class WebRTCProviderCommand:
    """Sends WebRTC provider commands `ProvideOffer`/`SolicitOffer` using WebRTC Provider Client.

    python is simply too slow on its own to upsert the webrtc session struct after receiving
    `ProvideOfferResponse`/`SolicitOfferResponse`, by which time webrtc requestor server could
    have returned `NOT_FOUND` status for requestor commands. So we need to do response handling
    in C++. Hence the need for separate command sender for such commands with custom response
    handling. This class handles that.
    """

    def __init__(self):
        lib = get_webrtc_provider_handle()
        self._handle = lib.pychip_webrtc_provider_client_create()
        lib.pychip_webrtc_provider_client_init_commandsender_callbacks(
            self._handle, _OnCommandSenderResponseCallback, _OnCommandSenderErrorCallback, _OnCommandSenderDoneCallback
        )

    def __del__(self):
        get_webrtc_provider_handle().pychip_webrtc_provider_client_destroy(self._handle)

    async def send_webrtc_provider_command(self, endpoint: int, cmd: ClusterCommand, *args, **kwargs):
        return await self._SendCommand(
            commandPath=CommandPath(EndpointId=endpoint, ClusterId=cmd.cluster_id, CommandId=cmd.command_id), payload=cmd
        )

    async def _SendCommand(self, commandPath: CommandPath, payload: ClusterCommand, responseType=None) -> PyChipError:
        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()
        if (responseType is not None) and (not issubclass(responseType, ClusterCommand)):
            raise ValueError("responseType must be a ClusterCommand or None")

        handle = get_webrtc_provider_handle()
        transaction = AsyncCommandTransaction(future, eventLoop, responseType)

        payloadTLV = payload.ToTLV()
        ctypes.pythonapi.Py_IncRef(ctypes.py_object(transaction))
        res = await builtins.chipStack.CallAsyncWithResult(
            lambda: handle.pychip_webrtc_provider_client_send_command(
                self._handle,
                ctypes.py_object(transaction),
                commandPath.EndpointId,
                commandPath.ClusterId,
                commandPath.CommandId,
                payloadTLV,
                len(payloadTLV),
            )
        )
        if not res.is_success:
            ctypes.pythonapi.Py_DecRef(ctypes.py_object(transaction))
            res.raise_on_error()
        return await future

    def init(self, node_id: int, fabric_index: int, endpoint: int):
        lib = get_webrtc_provider_handle()
        lib.pychip_webrtc_provider_client_init(self._handle, node_id, fabric_index, endpoint)

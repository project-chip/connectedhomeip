#
#    Copyright (c) 2024 Project CHIP Authors
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
from ctypes import c_void_p
from dataclasses import dataclass

from . import Bdx


@dataclass
class InitMessage:
    # The transfer control flag constants SENDER_DRIVE, RECEIVER_DRIVE, and ASYNC are defined in BdxProtocol.py.
    TransferControlFlags: int
    MaxBlockSize: int
    StartOffset: int
    Length: int
    FileDesignator: bytes
    Metadata: bytes


class BdxTransfer:
    def __init__(self, bdx_transfer: c_void_p, init_message: InitMessage, data: bytes = None):
        self.init_message = init_message
        self._bdx_transfer = bdx_transfer
        # _data is a bytearray when receiving data, so the data to send is converted to one as well for consistency.
        self._data = bytearray(data) if data else None

    async def accept_and_send_data(self) -> None:
        assert self._data is not None
        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()
        res = Bdx.AcceptReceiveTransfer(self._bdx_transfer, self._data, future)
        res.raise_on_error()
        await future

    async def accept_and_receive_data(self) -> bytes:
        assert self._data is None
        eventLoop = asyncio.get_running_loop()
        future = eventLoop.create_future()
        self._data = bytearray()
        res = Bdx.AcceptSendTransfer(self._bdx_transfer, lambda data: self._data.extend(data), future)
        res.raise_on_error()
        await future
        return bytes(self._data)

    async def reject(self) -> None:
        res = await Bdx.RejectTransfer(self._bdx_transfer)
        res.raise_on_error()

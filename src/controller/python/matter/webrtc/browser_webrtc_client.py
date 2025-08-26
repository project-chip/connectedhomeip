#
#  Copyright (c) 2025 Project CHIP Authors
#  All rights reserved.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

import asyncio
from typing import Any, Callable

from .async_websocket_client import AsyncWebSocketClient
from .types import WebSocketMessage


class BrowserWebRTCClient:
    def __init__(self, ws_client: AsyncWebSocketClient, id: int):
        self.event_callbacks: dict[str, Callable] = {}
        self.ws_client = ws_client
        self.pending_cmd_responses: dict[str, asyncio.Future] = {}
        self.event_loop = asyncio.get_event_loop()
        self._event_callbacks_without_parameter = ("GATHERING_STATE_COMPLETE",)
        self.id = id

    async def create_peer_connection(self):
        event = "CREATE_PEER_CONNECTION"
        return await self.send_message(event)

    async def close_peer_connection(self):
        event = "CLOSE_PEER_CONNECTION"
        return await self.send_message(event)

    async def create_offer(self):
        event = "CREATE_OFFER"
        offer_sdp = await self.send_message(event)
        self.event_callbacks["OFFER"](offer_sdp)

    async def create_answer(self):
        event = "CREATE_ANSWER"
        answer_sdp = await self.send_message(event)
        self.event_callbacks["ANSWER"](answer_sdp)

    async def set_remote_description(self, sdp: str, type: str):
        event = f"SET_REMOTE_{type.upper()}"
        return await self.send_message(event, sdp)

    async def set_remote_icecandidates(self, candidates):
        event = "SET_REMOTE_ICE_CANDIDATES"
        return await self.send_message(event, candidates)

    def on_local_description(self, callback):
        self.event_callbacks["OFFER"] = lambda desc: callback(desc, "offer")
        self.event_callbacks["ANSWER"] = lambda desc: callback(desc, "answer")

    def on_ice_candidate(self, callback):
        self.event_callbacks["LOCAL_ICE_CANDIDATES"] = callback

    async def get_local_description(self):
        event = "GET_LOCAL_DESCRIPTION"
        return await self.send_message(event)

    async def get_peer_connection_state(self):
        event = "GET_PEER_CONNECTION_STATE"
        state = await self.send_message(event)
        return state

    def on_gathering_complete(self, callback):
        self.event_callbacks["GATHERING_STATE_COMPLETE"] = callback

    def on_state_change(self, callback):
        self.event_callbacks["PEER_CONNECTION_STATE"] = callback

    async def send_message(self, event, value=None):
        self.pending_cmd_responses[event] = self.event_loop.create_future()
        self.ws_client.send_message(WebSocketMessage(type=event, sessionId=self.id, data=value))
        return await self.pending_cmd_responses[event]

    def handle_messsage(self, message: Any):
        ws_message = self.parse_messsage(message)

        if ws_message.type in self.pending_cmd_responses:
            self.handle_cmd_event(ws_message)
        elif ws_message.type in self.event_callbacks:
            self.handle_messsage_event(ws_message)
        else:
            raise RuntimeError("Unknown event received from server")

    def handle_messsage_event(self, message: WebSocketMessage):
        callback = self.event_callbacks[message.type]
        if message.type in self._event_callbacks_without_parameter:
            callback()
        elif message.type == "LOCAL_ICE_CANDIDATES":
            ice = message.data
            callback(ice["candidate"], ice["sdpMid"], ice["sdpMLineIndex"])
        else:
            callback(message.data)

    def handle_cmd_event(self, message: WebSocketMessage):
        if message.error:
            self.pending_cmd_responses[message.type].set_exception(Exception(message.error))
        else:
            self.pending_cmd_responses[message.type].set_result(message.data)
        return

    def parse_messsage(self, message: dict[str, Any]) -> WebSocketMessage:
        if not self.validate_message(message):
            # return a dummy WebSocketMessage to set exception
            raise RuntimeError(f"Invalid message received from server {message =}")
        return WebSocketMessage(**message)

    def validate_message(self, message: dict[str, Any]) -> bool:
        if not isinstance(message, dict):
            return False
        valid_keys = ("data", "error", "type", "sessionId")
        if len(valid_keys) != len(message.keys()):
            return False
        return all([key in message for key in valid_keys])

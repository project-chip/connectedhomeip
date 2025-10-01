import asyncio
import json
import logging
from dataclasses import asdict
from typing import Any, Callable, Optional

import websockets

from .types import WebSocketMessage

LOGGER = logging.getLogger(__name__)
LOGGER.setLevel(logging.INFO)


class AsyncWebSocketClient:
    def __init__(self, uri: str, message_handler: Optional[Callable] = None):
        self.uri = uri
        self.websocket = None
        self.send_queue = asyncio.Queue()
        self.message_handler = message_handler
        self.running = False
        self.tasks = []

    async def connect(self):
        try:
            self.websocket = await websockets.connect(self.uri)
            self.running = True
            LOGGER.debug(f"Connected to {self.uri}")
            self.tasks.append(asyncio.create_task(self._send_loop()))
            self.tasks.append(asyncio.create_task(self._receive_loop()))

        except Exception as e:
            LOGGER.error(f"Failed to connect to {self.uri}: {e}")
            raise e

    async def _send_loop(self):
        while self.running:
            try:
                message = await self.send_queue.get()
                if self.websocket:
                    await self.websocket.send(message)
                    LOGGER.debug(f"Sent message: {message}")

            except websockets.exceptions.ConnectionClosed:
                LOGGER.error("Websocket connection closed")
                break

            except Exception as e:
                LOGGER.error(f"Error in send loop: {e}")

    async def _receive_loop(self):
        while self.running:
            try:
                if self.websocket:
                    message = await self.websocket.recv()
                    LOGGER.debug(f"Received message: {message}")
                    if self.message_handler:
                        self.message_handler(message)

            except websockets.exceptions.ConnectionClosed:
                LOGGER.error("Websocket connection closed")
                break

            except Exception as e:
                LOGGER.error(f"Error in receive loop: {e}")

    def send_message(self, message: Any):
        if not self.running:
            raise RuntimeError("Cannot send message when WebSocket is not connected")
        if isinstance(message, WebSocketMessage):
            message = json.dumps(asdict(message))
        self.send_queue.put_nowait(message)

    async def disconnect(self):
        self.running = False

        # cancel send and receive tasks and wait for them to complete
        if self.tasks:
            for task in self.tasks:
                task.cancel()
            await asyncio.gather(*self.tasks, return_exceptions=True)
            self.tasks = []

        if self.websocket:
            await self.websocket.close()
            self.websocket = None
            LOGGER.debug(f"Disconnected from {self.uri}")

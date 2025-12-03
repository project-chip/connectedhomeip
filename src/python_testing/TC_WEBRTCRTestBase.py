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

import logging

import websockets

from matter.testing.matter_testing import MatterBaseTest

log = logging.getLogger(__name__)

# WebSocket server URI for sending commands to the DUT
SERVER_URI = "ws://localhost:9002"


class WEBRTCRTestBase(MatterBaseTest):
    """
    Base class for WebRTC Transport Requestor test cases.

    Provides common functionality including:
    - WebSocket-based command sending to the DUT
    - Shared SERVER_URI constant
    """

    async def send_command(self, command):
        """
        Send a command to the DUT via WebSocket and wait for response.

        Args:
            command: The command string to send to the DUT

        This method:
        1. Connects to the WebSocket server at SERVER_URI
        2. Sends the command
        3. Waits for and receives the response
        4. Logs the connection, command, and response status

        Raises:
            Exception: Re-raises any exceptions after logging them clearly
        """
        try:
            async with websockets.connect(SERVER_URI) as websocket:
                log.info(f"Connected to {SERVER_URI}")

                # Send command
                log.info(f"Sending command: {command}")
                await websocket.send(command)

                # Receive response
                await websocket.recv()
                log.info("Received command response")

        except ConnectionRefusedError as e:
            log.error(f"Failed to connect to WebSocket server at {SERVER_URI}: Connection refused. "
                          f"Is the DUT WebSocket server running? Error: {e}")
            raise

        except websockets.exceptions.WebSocketException as e:
            log.error(f"WebSocket error while communicating with {SERVER_URI}: {e}")
            raise

        except OSError as e:
            log.error(f"Network error while connecting to {SERVER_URI}: {e}")
            raise

        except Exception as e:
            log.error(f"Unexpected error while sending command '{command}' to {SERVER_URI}: {e}")
            raise

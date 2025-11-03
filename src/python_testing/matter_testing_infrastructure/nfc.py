#!/usr/bin/python3
#
#    Copyright (c) 2025 Project CHIP Authors
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
import logging
import threading
import time

import ndef
import smartcard
import smartcard.CardMonitoring

logger = logging.getLogger(__name__)


class TagEventObserver(smartcard.CardMonitoring.CardObserver):
    """
    Observer class to handle tag (NFC) events.
    """

    def __init__(self, reader):
        self.reader = reader
        self.last_ndef = None  # Store last NDEF bytes read

    # This method is called when a tag is inserted or removed
    def update(self, observable, actions):
        (added_tags, removed_tags) = actions
        for tag in added_tags:
            logger.debug("NFC tag detected")
            try:
                ndef_bytes = self.reader.read_t4t_ndef()
                self.last_ndef = ndef_bytes
                logger.debug(self.reader.ndef_content_to_string(ndef_bytes))
            except Exception as e:
                logger.info(f"Error reading NFC tag: {e}")
        for tag in removed_tags:
            logger.debug("Tag removed.")


class TagMonitorManager:
    """
    Class to activate/deactivate NFC monitoring in background.
    """

    def __init__(self, reader):
        self.reader = reader
        self.tag_monitor = None
        self.observer = None
        self._stop_event = threading.Event()
        self._thread = None

    def _monitoring_thread(self):
        self.tag_monitor = smartcard.CardMonitoring.CardMonitor()
        self.observer = TagEventObserver(self.reader)
        self.tag_monitor.addObserver(self.observer)
        logger.info("Start monitoring NFC tags")
        try:
            while not self._stop_event.is_set():
                time.sleep(0.1)
        finally:
            self.tag_monitor.deleteObserver(self.observer)
            logger.info("Stopped monitoring NFC tags")

    async def activate(self):
        self._stop_event.clear()
        self._thread = threading.Thread(target=self._monitoring_thread, daemon=True)
        self._thread.start()
        # Return control to the caller, but keep this coroutine alive until deactivation
        while not self._stop_event.is_set():
            await asyncio.sleep(0.1)
        # After deactivation, join the thread to ensure cleanup
        if self._thread is not None:
            self._thread.join()
            self._thread = None
        # Return the last NDEF bytes read
        if self.observer:
            return self.observer.last_ndef
        return None

    def deactivate(self):
        self._stop_event.set()


class NFCReader:
    """
    Provides several NFC related functions:
    - NFC reader operations: monitoring tags, reading and writing a tag.
    - NFC tag conversion to string
    - Recognition of of NFC-based Matter onboarding payload
    Use the constructor without argument to connect with the first available reader.
        reader = nfc.NFCReader()
    """

    def __init__(self, reader=None):
        """
        If reader is None, automatically connect to the first available reader.
        """
        if reader is None:
            reader = self.get_connected_reader()
            if reader is None:
                raise RuntimeError("No smartcard reader found.")
        self.reader = reader
        self._monitor_manager = None

    @staticmethod
    def get_connected_reader():
        """
        Function to find and return the first available NFC reader.
        """
        reader_list = smartcard.System.readers()
        if not reader_list:
            logger.info("No smartcard reader found.")
            return None
        logger.debug("Available readers:")
        for idx, reader in enumerate(reader_list):
            logger.debug(f"{idx}: {reader}")
        reader = reader_list[0]  # Use the first reader found
        logger.debug(f"Using NFC reader: {reader}")
        return reader

    def activate_tag_monitoring(self):
        """
        Start monitoring in the background and return control immediately.
        """
        self._monitor_manager = TagMonitorManager(self)
        return self._monitor_manager.activate()

    def deactivate_tag_monitoring(self):
        """
        Stop tag monitoring.
        """
        if self._monitor_manager is not None:
            self._monitor_manager.deactivate()
            self._monitor_manager = None

    def ndef_content_to_string(self, ndef_bytes):
        """
        Returns a string representation of all NDEF records and their fields.
        """
        if not ndef_bytes:
            return "No NDEF data to decode."

        lines = []
        try:
            for idx, record in enumerate(ndef.message_decoder(ndef_bytes), 1):
                lines.append(f"NDEF Record {idx}: {record}")
        except Exception as e:
            lines.append(f"Error decoding NDEF: {e}")

        return "\n".join(lines)

    def read_t4t_ndef(self):
        """
        Reads the NDEF message from an NFC tag using the provided reader.

        Returns:
            bytes: The raw NDEF message bytes, or None if an error occurs.
        """
        with NFCConnection(self) as connection:

            # Select NDEF Tag Application
            SELECT_AID = [0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00]
            _, sw1, sw2 = connection.transmit(SELECT_AID)
            if (sw1, sw2) != (0x90, 0x00):
                logger.debug(f"SELECT_AID failed: SW1=0x{sw1:02X}, SW2=0x{sw2:02X}")
                return None

            # Select CC file (E103)
            SELECT_CC = [0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x03]
            _, sw1, sw2 = connection.transmit(SELECT_CC)
            if (sw1, sw2) != (0x90, 0x00):
                logger.debug(f"SELECT_CC failed: SW1=0x{sw1:02X}, SW2=0x{sw2:02X}")
                return None

            # Read CC file
            READ_CC = [0x00, 0xB0, 0x00, 0x00, 0x0F]
            cc_bytes, sw1, sw2 = connection.transmit(READ_CC)
            if (sw1, sw2) != (0x90, 0x00):
                logger.debug(f"READ_CC failed: SW1=0x{sw1:02X}, SW2=0x{sw2:02X}")
                return None

            # Select NDEF file (E104)
            SELECT_NDEF = [0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x04]
            _, sw1, sw2 = connection.transmit(SELECT_NDEF)
            if (sw1, sw2) != (0x90, 0x00):
                logger.debug(f"SELECT_NDEF failed: SW1=0x{sw1:02X}, SW2=0x{sw2:02X}")
                return None

            # Read NDEF length
            READ_NLEN = [0x00, 0xB0, 0x00, 0x00, 0x02]
            nlen, sw1, sw2 = connection.transmit(READ_NLEN)
            if (sw1, sw2) != (0x90, 0x00):
                logger.debug(f"READ_NLEN failed: SW1=0x{sw1:02X}, SW2=0x{sw2:02X}")
                return None
            length = (nlen[0] << 8) + nlen[1]

            # Read NDEF message
            READ_NDEF = [0x00, 0xB0, 0x00, 0x02, length]
            ndef_data, sw1, sw2 = connection.transmit(READ_NDEF)
            if (sw1, sw2) != (0x90, 0x00):
                logger.debug(f"READ_NDEF failed: SW1=0x{sw1:02X}, SW2=0x{sw2:02X}")
                return None

        logger.debug(self.ndef_content_to_string(bytes(ndef_data)))
        return bytes(ndef_data)

    def is_onboarding_data(self, ndef_bytes):
        """
        Checks if the NDEF message contains a URI record starting with 'MT:' (case-insensitive).

        Args:
            ndef_bytes (bytes): Raw NDEF message bytes.

        Returns:
            bool: True if a URI record starts with 'MT:' (case-insensitive), else False.
        """
        if not ndef_bytes:
            logger.debug("No NDEF!")
            return False
        try:
            for record in ndef.message_decoder(ndef_bytes):
                if hasattr(record, "uri") and record.uri and record.uri.lower().startswith("mt:"):
                    logger.debug(f"Found Matter URI: {record.uri}")
                    return True
        except Exception as e:
            logger.error(f"NDEF parsing error: {e}")
        return False

    def write_t4t_ndef_uri(self, uri):
        """
        Writes an NDEF message with a single URI record to the NFC tag.

        Args:
            uri (str): The URI string to write.

        Returns:
            bool: True if write was successful, False otherwise.
        """
        with NFCConnection(self) as connection:

            # Select NDEF file (E104)
            SELECT_NDEF = [0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x04]
            _, sw1, sw2 = connection.transmit(SELECT_NDEF)
            if (sw1, sw2) != (0x90, 0x00):
                logger.debug(f"SELECT_NDEF failed: SW1=0x{sw1:02X}, SW2=0x{sw2:02X}")
                return False

            # Create NDEF message with a single URI record
            record = ndef.UriRecord(uri)
            ndef_message = b''.join(ndef.message_encoder([record]))
            ndef_length = len(ndef_message)

            # Write NDEF length (first 2 bytes)
            WRITE_NLEN = [0x00, 0xD6, 0x00, 0x00, 0x02, (ndef_length >> 8) & 0xFF, ndef_length & 0xFF]
            _, sw1, sw2 = connection.transmit(WRITE_NLEN)
            if (sw1, sw2) != (0x90, 0x00):
                logger.debug(f"WRITE_NLEN failed: SW1=0x{sw1:02X}, SW2=0x{sw2:02X}")
                return False

            # Write the NDEF message itself
            # The maximum length per APDU is usually 255 bytes; split if needed
            offset = 0
            while offset < ndef_length:
                chunk = ndef_message[offset:offset+0xFF]
                chunk_len = len(chunk)
                WRITE_NDEF = [0x00, 0xD6, (offset + 2) >> 8, (offset + 2) & 0xFF, chunk_len] + list(chunk)
                _, sw1, sw2 = connection.transmit(WRITE_NDEF)
                if (sw1, sw2) != (0x90, 0x00):
                    logger.debug(f"WRITE_NDEF failed at offset {offset}: SW1=0x{sw1:02X}, SW2=0x{sw2:02X}")
                    return False
                offset += chunk_len

        logger.info(f"Successfully wrote URI '{uri}' to NFC tag.")
        return True


class NFCConnection:
    def __init__(self, reader: NFCReader):
        self.reader = reader
        self.connection = None

    def __enter__(self):
        self.connection = self.reader.reader.createConnection()
        self.connection.connect()
        return self.connection

    def __exit__(self, exc_type, exc_val, exc_tb):
        try:
            if self.connection:
                self.connection.disconnect()
        except Exception as e:
            logger.warning(f"Failed to disconnect NFC connection: {e}")
        return False

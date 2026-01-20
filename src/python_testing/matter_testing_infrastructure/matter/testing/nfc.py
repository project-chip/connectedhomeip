#
#    Copyright (c) 2026 Project CHIP Authors
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

import ndef  # From 'ndeflib' library
import smartcard  # From 'pyscard' library
import smartcard.CardMonitoring
from mobly import asserts

log = logging.getLogger(__name__)

# NFC transmission success status words
NFC_SUCCESS_SW1 = 0x90
NFC_SUCCESS_SW2 = 0x00

MAX_SHORT_APDU_LENGTH = 255

# APDU Class (CLA) bytes
CLA_ISO = 0x00              # ISO/IEC 7816-4 standard class

# APDU Instruction (INS) bytes
INS_SELECT = 0xA4           # Select file or application
INS_READ_BINARY = 0xB0      # Read binary data from file
INS_UPDATE_BINARY = 0xD6    # Update binary data in file

# Constant representing the NFC Forum Well-Known Type (WKT) for a URI record.
# Used to identify NDEF records containing a Uniform Resource Identifier (URI)
# such as a website link, app link, or other addressable resource . urn:nfc:wkt:U -> refers to URI type
# Some commonly used formats are below for reference
# urn:nfc:wkt:T -> refers Text record
# urn:nfc:wkt:Sp -> Smart Poster record
NFC_WKT = 'urn:nfc:wkt:U'


class NFCReader:
    """
    Provides several NFC related functions:
    - NFC reader operations: monitoring tags, reading and writing a tag.
    - NFC tag conversion to string
    - Recognition of NFC-based Matter onboarding payload

    Use the constructor with an integer index to select the NFC reader:
        reader = NFCReader(nfc_reader_index=0)
    """

    def __init__(self, nfc_reader_index=0):
        """Connects to the NFC reader at the specified index.

        Args:
            nfc_reader_index: Index of the NFC reader to use (default: 0).
        """
        reader_list = smartcard.System.readers()
        if not reader_list:
            raise ValueError("No smartcard reader found.")

        log.info("Available readers:")
        for idx, reader in enumerate(reader_list):
            log.info(f"{idx}: {reader}")

        if nfc_reader_index < 0 or nfc_reader_index >= len(reader_list):
            raise IndexError(f"Reader index {nfc_reader_index} is out of range.")

        self.reader = reader_list[nfc_reader_index]
        self._monitor_manager = None
        log.info(f"Using NFC reader: {self.reader}")

    def read_nfc_tag_data(self) -> str:
        """
        Read NDEF data from a tag.

        This function reads NDEF data from any tag present in the reader's field.
        It expects the first NDEF record to be a URI record.

        Args:
            This function takes no arguments.

        Returns:
            str: The decoded NDEF record data.

        Raises:
            AssertionError: If any message transmission fails.

        Example:
            >>> tag_data = reader.read_nfc_tag_data()
            >>> print(f"NFC tag contains: {tag_data}")
        """
        with NFCConnection(self) as connection:
            # Perform NDEF file system navigation sequence
            _select_ndef_application(connection)    # Select NDEF app
            _select_cc_file(connection)             # Select Capability Container (aka CC File)
            _select_ndef_file(connection)           # Select data file

            # Read NDEF message length and data
            ndef_length = _read_ndef_length(connection)
            ndef_data = _read_ndef_data(connection, ndef_length)

            # Parse NDEF message into records and find record with data
            ndef_records = list(ndef.message_decoder(ndef_data))
            if not ndef_records:
                raise ValueError("No NDEF records found in message - tag may be corrupted or empty")

            # Loop through records to find a URI record
            for record in ndef_records:
                # Check for URI record type (well-known type 'U')
                if record.type == NFC_WKT:
                    # The payload is described in NFC Forum's "URI Record Type Definition Technical Specification"
                    # available here https://berlin.ccc.de/~starbug/felica/NFCForum-TS-RTD_URI_1.0.pdf
                    # As indicated in paragraph 3, the payload format is:
                    #     [identifier code (1 byte)] + [URI string]
                    # There is currently no prefix officially registered for Matter so the on-boarding data string
                    # is fully in the URI string.
                    #
                    # Ignore the identifier code and read the URI string
                    if hasattr(record, 'data') and record.data and len(record.data) > 1:
                        return record.data[1:].decode("utf-8")
                    raise ValueError("NDEF URI record payload is missing or too short")
            # If we get here, no URI record was found
            raise ValueError("No NDEF URI record found in message")

    def write_ndef_uri(self, uri: str) -> None:
        """
        Writes an NDEF message with a single URI record to the NFC tag.

        Args:
            uri (str): The URI string to write.

        Raises:
            NFCTransmissionError or other exceptions if writing fails.
        """
        with NFCConnection(self) as connection:

            _select_ndef_application(connection)    # Select NDEF app
            _select_ndef_file(connection)           # Select NDEF file

            # Create NDEF message with a single URI record
            record = ndef.UriRecord(uri)

            _write_ndef_record(connection, record)

        log.info(f"Successfully wrote URI '{uri}' to NFC tag.")

    def is_onboarding_data(self, ndef_uri: str) -> bool:
        """
        Checks if the NDEF URI string corresponds to a Matter onboarding payload.

        Args:
            ndef_uri (str): The URI string extracted from the NFC tag.

        Returns:
            bool: True if the string starts with 'MT:', False otherwise.

        Example:
            >>> is_onboarding_data("mt:ABC123")
            True
            >>> is_onboarding_data("https://example.com")
            False
        """
        if ndef_uri is None:
            return False
        return ndef_uri.startswith("MT:")

    def activate_tag_monitoring(self):
        """
        Start NFC tag monitoring in background and return control immediately.
        """
        self._monitor_manager = TagMonitorManager(self)
        return self._monitor_manager.activate()

    def deactivate_tag_monitoring(self):
        """
        Stop NFC tag monitoring.
        """
        if self._monitor_manager is not None:
            self._monitor_manager.deactivate()
            self._monitor_manager = None


def _check_transmission_status(sw1: int, sw2: int, operation_name: str):
    """
    Check status words from NFC message transmission.

    This function validates that the transmitted message was processed successfully
    by checking the status words SW1 and SW2. Successful transmission returns
    SW1=0x90 and SW2=0x00 according to ISO/IEC 7816-4 standard.

    Args:
        sw1 (int): Status Word 1 returned from the message transmission.
        sw2 (int): Status Word 2 returned from the message transmission.
        operation_name (str): Descriptive name of the operation for error reporting.

    Raises:
        AssertionError: If the transmission failed (SW1, SW2 != 0x90, 0x00).
    """
    asserts.assert_true(
        (sw1, sw2) == (NFC_SUCCESS_SW1, NFC_SUCCESS_SW2),
        f"Message transmission failed - {operation_name}: SW1={sw1:02X}, SW2={sw2:02X}"
    )


def _select_ndef_application(connection):
    """
    Select the NDEF Tag Application on the NFC reader.

    This function transmits a SELECT message to choose the NDEF (NFC Data Exchange Format)
    Tag Application using the standardized AID (Application Identifier) D2760000850101.
    This is a prerequisite step before accessing NDEF data structures on NFC tags.

    Args:
        connection: The NFC reader connection object used to
                                    communicate with the NFC tag.

    Raises:
        AssertionError: If the message transmission fails (SW1, SW2 != 0x90, 0x00).

    Note:
        The NDEF Tag Application AID (D2760000850101) is defined in the NFC Forum
        specifications for NDEF-compatible tags.
    """
    # ISO/IEC 7816-4 APDU command to select NDEF's Application ID (= "D2760000850101")
    # (P1, P2)=(0x04, 0x0C) corresponds to select first or only occurrence
    # For more Information you can refer here https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit
    SELECT_NDEF_AID = [
        CLA_ISO, INS_SELECT, 0x04, 0x00, 0x07,                # CLA INS P1 P2 Lc
        0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01,             # NDEF's Application ID
        0x00                                                  # Le
    ]
    data, sw1, sw2 = connection.transmit(SELECT_NDEF_AID)
    _check_transmission_status(sw1, sw2, "select NDEF application")


def _select_cc_file(connection):
    """
    Select the Capability Container (CC) file on the NFC tag.

    The Capability Container file (E103) contains metadata about the NDEF data
    structure capabilities and constraints of the NFC tag, including maximum
    NDEF message size, read/write permissions, and supported operations.

    Args:
        connection: The NFC reader connection object used to
                                    communicate with the NFC tag.

    Raises:
        AssertionError: If the message transmission fails (SW1, SW2 != 0x90, 0x00).

    Note:
        This function must be called after _select_ndef_application() and before
        accessing NDEF data files.
    """
    # ISO/IEC 7816-4 APDU command to select the Capability Container file (file ID: 0xE103)
    # (P1, P2)=(0x00, 0x0C) corresponds to select by file ID
    SELECT_CC_FILE = [CLA_ISO, INS_SELECT, 0x00, 0x0C, 0x02,  # CLA INS P1 P2 Lc
                      0xE1, 0x03]                             # File ID
    data, sw1, sw2 = connection.transmit(SELECT_CC_FILE)
    _check_transmission_status(sw1, sw2, "select CC file")


def _select_ndef_file(connection):
    """
    Select the NDEF data file on the NFC tag.

    The NDEF file (E104) contains the actual NDEF message data that can be read
    from or written to the NFC tag. This file stores the structured NDEF records
    that contain the application data (such as URLs, text, or other payloads).

    Args:
        connection: The NFC reader connection object used to
                                    communicate with the NFC tag.

    Raises:
        AssertionError: If the message transmission fails (SW1, SW2 != 0x90, 0x00).

    Note:
        This function must be called after _select_ndef_application() and
        _select_cc_file() before reading or writing NDEF data.
    """
    # ISO/IEC 7816-4 APDU command to select the NDEF file (file ID: 0xE104)
    # (P1, P2)=(0x00, 0x0C) corresponds to select by file ID
    SELECT_NDEF_FILE = [CLA_ISO, INS_SELECT, 0x00, 0x0C, 0x02,  # CLA INS P1 P2 Lc
                        0xE1, 0x04]                             # File ID
    data, sw1, sw2 = connection.transmit(SELECT_NDEF_FILE)
    _check_transmission_status(sw1, sw2, "select NDEF file")


def _read_ndef_length(connection):
    """
    Read the NDEF message length from the NFC tag.

    This function reads the first 2 bytes from the selected NDEF file, which contain
    the length of the NDEF message data in big-endian format. The length field is
    a mandatory component of the NDEF file structure that indicates how many bytes
    of NDEF data follow.

    Args:
        connection: The NFC reader connection object used to communicate with the
                   NFC tag.

    Returns:
        int: The length of the NDEF message data in bytes.

    Raises:
        AssertionError: If the message transmission fails (SW1, SW2 != 0x90, 0x00).

    Note:
        This function must be called after selecting the NDEF file and before
        reading the actual NDEF data. The returned length is used to determine
        how many bytes to read in subsequent operations.
    """
    # ISO/IEC 7816-4 APDU command to read a binary message of 2 bytes starting from offset 0x00.
    # Those 2 bytes contain the NDEF message length.
    # (P1, P2) contain respectively the offset high byte and the offset low byte.
    READ_NDEF_LENGTH = [CLA_ISO, INS_READ_BINARY, 0x00, 0x00, 0x02]           # CLA INS P1 P2 Lc
    data, sw1, sw2 = connection.transmit(READ_NDEF_LENGTH)
    _check_transmission_status(sw1, sw2, "read NDEF length")

    # Convert 2-byte big-endian length to integer
    return (data[0] << 8) + data[1]


def _read_ndef_data(connection, length: int) -> bytes:
    """
    Read the NDEF message data from the NFC tag.

    This function reads the actual NDEF message content from the NFC tag, starting
    from offset 0x0002 (after the 2-byte length field) and reading the specified
    number of bytes. The returned data contains the raw NDEF message that can be
    parsed into NDEF records.

    Args:
        connection: The NFC reader connection object used to communicate with the
                   NFC tag.
        length (int): The number of bytes to read, typically obtained from
                     _read_ndef_length().

    Returns:
        bytes: The raw NDEF message data as a bytes object.

    Raises:
        AssertionError: If the message transmission fails (SW1, SW2 != 0x90, 0x00).

    Note:
        This function must be called after _read_ndef_length() to ensure the
        correct number of bytes are read. The returned data can be processed
        with NDEF parsers to extract individual records and their payloads.
    """
    data = bytearray()
    offset = 2  # Start after the 2-byte length field

    while len(data) < length:
        chunk_size = min(MAX_SHORT_APDU_LENGTH, length - len(data))
        offset_high = (offset >> 8) & 0xFF
        offset_low = offset & 0xFF
        # ISO/IEC 7816-4 APDU command to read a binary message of 'chunk_size' bytes, starting from
        # the offset indicated in (P1, P2)
        apdu = [
            CLA_ISO,
            INS_READ_BINARY,
            offset_high,        # P1
            offset_low,         # P2
            chunk_size
        ]
        chunk, sw1, sw2 = connection.transmit(apdu)
        _check_transmission_status(sw1, sw2, f"read NDEF data at offset {offset}")
        data.extend(chunk)
        offset += chunk_size

    return bytes(data)


def _write_ndef_record(connection, record):

    ndef_message = b''.join(ndef.message_encoder([record]))
    ndef_length = len(ndef_message)

    # Write NDEF header
    ndef_header = [
        CLA_ISO,
        INS_UPDATE_BINARY,
        0x00, 0x00,
        0x02,
        (ndef_length >> 8) & 0xFF,
        ndef_length & 0xFF,
    ]

    _, sw1, sw2 = connection.transmit(ndef_header)
    _check_transmission_status(sw1, sw2, "Write NDEF header failed")

    # Write the NDEF message itself
    # The maximum length per APDU is usually 255 bytes; split if needed
    offset = 0
    while offset < ndef_length:
        chunk = ndef_message[offset:offset+MAX_SHORT_APDU_LENGTH]
        chunk_len = len(chunk)
        WRITE_NDEF = [
            CLA_ISO,
            INS_UPDATE_BINARY,
            (offset + 2) >> 8,
            (offset + 2) & 0xFF,
            chunk_len,
        ] + list(chunk)

        _, sw1, sw2 = connection.transmit(WRITE_NDEF)
        _check_transmission_status(sw1, sw2, "Write NDEF message failed")
        offset += chunk_len


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
            log.debug("NFC tag detected")
            try:
                nfc_tag_data = self.reader.read_nfc_tag_data()
                self.last_ndef = nfc_tag_data
                log.debug(nfc_tag_data)
            except Exception as e:
                log.info(f"Error reading NFC tag: {e}")
        for tag in removed_tags:
            log.debug("Tag removed.")


class TagMonitorManager:
    """
    Class to activate/deactivate NFC monitoring in background.
    """

    def __init__(self, reader):
        self.reader = reader
        self.tag_monitor = None
        self.observer = None
        self._stop_event = threading.Event()
        self._async_stop_event = None  # Will be set in activate()
        self._thread = None

    def _monitoring_thread(self):
        self.tag_monitor = smartcard.CardMonitoring.CardMonitor()
        self.observer = TagEventObserver(self.reader)
        self.tag_monitor.addObserver(self.observer)
        log.info("Start monitoring NFC tags")
        try:
            # Wait until stop event is set
            self._stop_event.wait()
        finally:
            self.tag_monitor.deleteObserver(self.observer)
            log.info("Stopped monitoring NFC tags")
            # Signal the async event when thread is done
            if self._async_stop_event is not None:
                loop = self._async_stop_event._loop
                if loop.is_running():
                    loop.call_soon_threadsafe(self._async_stop_event.set)

    async def activate(self):
        self._stop_event.clear()
        self._async_stop_event = asyncio.Event()
        self._thread = threading.Thread(target=self._monitoring_thread, daemon=True)
        self._thread.start()
        # Wait for the async event to be set
        await self._async_stop_event.wait()
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
            log.warning(f"Failed to disconnect NFC connection: {e}")
        return False

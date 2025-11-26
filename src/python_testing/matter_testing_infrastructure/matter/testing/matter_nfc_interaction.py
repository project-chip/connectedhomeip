import logging

import ndef
from mobly import asserts
from smartcard.System import readers

logger = logging.getLogger(__name__)

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


def _select_ndef_application(reader_connection_object):
    """
    Select the NDEF Tag Application on the NFC reader.

    This function transmits a SELECT message to choose the NDEF (NFC Data Exchange Format)
    Tag Application using the standardized AID (Application Identifier) D2760000850101.
    This is a prerequisite step before accessing NDEF data structures on NFC tags.

    Args:
        reader_connection_object: The NFC reader connection object used to
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
    data, sw1, sw2 = reader_connection_object.transmit(SELECT_NDEF_AID)
    _check_transmission_status(sw1, sw2, "select NDEF application")


def _select_cc_file(reader_connection_object):
    """
    Select the Capability Container (CC) file on the NFC tag.

    The Capability Container file (E103) contains metadata about the NDEF data
    structure capabilities and constraints of the NFC tag, including maximum
    NDEF message size, read/write permissions, and supported operations.

    Args:
        reader_connection_object: The NFC reader connection object used to
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
    data, sw1, sw2 = reader_connection_object.transmit(SELECT_CC_FILE)
    _check_transmission_status(sw1, sw2, "select CC file")


def _select_ndef_file(reader_connection_object):
    """
    Select the NDEF data file on the NFC tag.

    The NDEF file (E104) contains the actual NDEF message data that can be read
    from or written to the NFC tag. This file stores the structured NDEF records
    that contain the application data (such as URLs, text, or other payloads).

    Args:
        reader_connection_object: The NFC reader connection object used to
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
    data, sw1, sw2 = reader_connection_object.transmit(SELECT_NDEF_FILE)
    _check_transmission_status(sw1, sw2, "select NDEF file")


def _read_ndef_length(reader_connection_object):
    """
    Read the NDEF message length from the NFC tag.

    This function reads the first 2 bytes from the selected NDEF file, which contain
    the length of the NDEF message data in big-endian format. The length field is
    a mandatory component of the NDEF file structure that indicates how many bytes
    of NDEF data follow.

    Args:
        reader_connection_object: The NFC reader connection object used to communicate with the
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
    data, sw1, sw2 = reader_connection_object.transmit(READ_NDEF_LENGTH)
    _check_transmission_status(sw1, sw2, "read NDEF length")

    # Convert 2-byte big-endian length to integer
    return (data[0] << 8) + data[1]



def _read_ndef_data(reader_connection_object, length: int) -> bytes:
    """
    Read the NDEF message data from the NFC tag.

    This function reads the actual NDEF message content from the NFC tag, starting
    from offset 0x0002 (after the 2-byte length field) and reading the specified
    number of bytes. The returned data contains the raw NDEF message that can be
    parsed into NDEF records.

    Args:
        reader_connection_object: The NFC reader connection object used to communicate with the
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
        chunk, sw1, sw2 = reader_connection_object.transmit(apdu)
        _check_transmission_status(sw1, sw2, f"read NDEF data at offset {offset}")
        data.extend(chunk)
        offset += chunk_size

    return bytes(data)


def _read_nfc_tag_data(reader_objects: list, nfc_reader_index: int):
    """
    Read and decode NDEF URI data from an NFC tag.
    This function expects the first NDEF record to be a URI record.
    """
    # Establish connection to the selected NFC reader
    reader_connection_object = reader_objects[nfc_reader_index].createConnection()
    try:
        reader_connection_object.connect()

        # Perform NDEF file system navigation sequence
        _select_ndef_application(reader_connection_object)  # Select NDEF app
        _select_cc_file(reader_connection_object)  # Select capability container
        _select_ndef_file(reader_connection_object)  # Select data file

        # Read NDEF message length and data
        ndef_length = _read_ndef_length(reader_connection_object)
        ndef_data = _read_ndef_data(reader_connection_object, ndef_length)

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
    finally:
        try:
            reader_connection_object.disconnect()
        except Exception as e:
            logger.warning(f"Disconnect Failed Due to Reason: {e}")  # Ignore disconnect errors


def connect_read_nfc_tag_data(nfc_reader_index: int) -> str:
    """
    Connect to an NFC reader and read NDEF data from a tag.

    This function discovers available NFC readers, connects to the specified reader,
    and reads NDEF data from any tag present in the reader's field. This is the
    main public API entry point for NFC tag reading operations in the Matter
    testing infrastructure.

    Args:
        nfc_reader_index (int): Index of the NFC reader to use from the available readers.

    Returns:
        str: The decoded NDEF record data.

    Raises:
        AssertionError: If no NFC readers are found or if any message transmission fails.

    Example:
        >>> tag_data = connect_read_nfc_tag_data(0)  # Use first available reader
        >>> print(f"NFC tag contains: {tag_data}")

    Note:
        This is the main entry point for NFC tag reading operations. All other
        functions are internal implementation details and should not be called
        directly from external code.
    """
    # Discover available NFC readers on the system
    available_nfc_readers = readers()
    asserts.assert_true(len(available_nfc_readers) > 0, "No NFC readers found.")

    # Read and return NFC tag data
    asserts.assert_greater(len(available_nfc_readers), nfc_reader_index,
                           f"Invalid NFC reader index {nfc_reader_index} provided by user as the Number of "
                           f"NFC Readers available is {len(available_nfc_readers)}")
    return _read_nfc_tag_data(available_nfc_readers, nfc_reader_index)

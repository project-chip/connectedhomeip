import logging
import re

import ndef
from mobly import asserts
from smartcard.System import readers

logger = logging.getLogger(__name__)

# NFC transmission success status words
NFC_SUCCESS_SW1 = 0x90
NFC_SUCCESS_SW2 = 0x00

MAX_NDEF_DATA_LENGTH = 256

def _clean_nfc_data(ndef_data_string: str) -> str:
    """
       Return a string containing only printable characters from `ndef_data_string`.

       Purpose
       - Remove non-printable/control characters (for example NUL: '\x00', DEL, etc.)
         commonly encountered when reading raw NDEF text-record payload bytes that
         include a status byte or other metadata.
       - Produce a visible, printable string safe to pass to text-based consumers
         such as SetupPayload loaders that expect a human-readable setup string.

       Parameters
       - ndef_data_string: str
           The input text to sanitize. Typically this will already be decoded (e.g. via
           payload_bytes.decode('utf-8')) or extracted from an NDEF Text record.

       Returns
       - str: sanitized string containing only printable characters. If the input is
         empty, returns an empty string.

       Example
       >>> _clean_nfc_data("\\x00MT:4CT91EVQ20KA0648G00")
       'MT:4CT91EVQ20KA0648G00'

       Rationale (short)
       - NDEF Text record payloads include a status byte and (optionally) a language
         code before the visible text. Those metadata bytes are non-printable and should
         be removed before using the visible text. This function provides a simple,
         defensive cleanup step for that common case.
       """
    return ''.join([ndef_data_char for ndef_data_char in ndef_data_string if ndef_data_char.isprintable()])

def _check_transmission_status(sw1:int, sw2:int, operation_name:str):
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
    # Transmit SELECT message with NDEF Tag Application ID (D2760000850101)
    SELECT_MESSAGE = [0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00]
    data, sw1, sw2 = reader_connection_object.transmit(SELECT_MESSAGE)
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
    # Transmit SELECT message for Capability Container file (file ID: E103)
    SELECT_CC_MESSAGE = [0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x03]
    data, sw1, sw2 = reader_connection_object.transmit(SELECT_CC_MESSAGE)
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
    # Transmit SELECT message for NDEF data file (file ID: E104)
    SELECT_NDEF_MESSAGE = [0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x04]
    data, sw1, sw2 = reader_connection_object.transmit(SELECT_NDEF_MESSAGE)
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
    # Transmit READ BINARY message for first 2 bytes (message length)
    READ_LENGTH_MESSAGE = [0x00, 0xB0, 0x00, 0x00, 0x02]
    data, sw1, sw2 = reader_connection_object.transmit(READ_LENGTH_MESSAGE)
    _check_transmission_status(sw1, sw2, "read NDEF length")

    # Convert 2-byte big-endian length to integer
    length = (data[0] << 8) + data[1]
    asserts.assert_less(length, MAX_NDEF_DATA_LENGTH, msg=f"NDEF of longer length than 256 are currently not managed,"
                        f" current length: {length}")
    return length


def _read_ndef_data(reader_connection_object, length):
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
    # Transmit READ BINARY message for NDEF data starting from offset 0x0002
    READ_DATA_MESSAGE = [0x00, 0xB0, 0x00, 0x02, length]
    data, sw1, sw2 = reader_connection_object.transmit(READ_DATA_MESSAGE)
    _check_transmission_status(sw1, sw2, "read NDEF data")
    return bytes(data)


def _read_nfc_tag_data(reader_objects:list, nfc_reader_index):
    """
    Read and decode NDEF data from an NFC tag.

    This function performs the complete sequence of operations to read NDEF data
    from an NFC tag: establishing connection, selecting applications and files,
    reading the data, and decoding it into a readable format.

    Args:
        reader_objects: List of available NFC reader objects.
        nfc_reader_index (int): Index of the NFC reader to use from the list.

    Returns:
        str: The decoded NDEF record data with non-printable characters removed.

    Raises:
        AssertionError: If any message transmission fails during the operation.

    Note:
        This function assumes the first NDEF record contains UTF-8 text data.
    """
    # Establish connection to the selected NFC reader
    reader_connection_object = reader_objects[nfc_reader_index].createConnection()
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

    # Loop through records to find one with data attribute,
    # when the data is decodable we will exit from the loop
    # if the data could not de-coded we will catch exception and move on to the next record
    for record in ndef_records:
        if hasattr(record, 'data') and record.data:
            try:
                ndef_record_data = record.data.decode("utf-8")
                # Clean up non-printable characters and return readable text which can be used as SetupPayload later on
                cleaned_data = _clean_nfc_data(ndef_record_data)
                return cleaned_data
            except UnicodeDecodeError:
                # Continue to next record if this one can't be decoded
                logger.warning(f"NDEF record data {record.data} could not be decoded moving on to next record")

    # If we get here, no record had data attribute or all failed to decode
    raise ValueError("No NDEF records with decodable data found")


def connect_read_nfc_tag_data(nfc_reader_index):
    """
    Connect to an NFC reader and read NDEF data from a tag.

    This function discovers available NFC readers, connects to the specified reader,
    and reads NDEF data from any tag present in the reader's field. This is the
    main public API entry point for NFC tag reading operations in the Matter
    testing infrastructure.

    Args:
        nfc_reader_index (int): Index of the NFC reader to use from the available readers.

    Returns:
        str: The decoded NDEF record data with non-printable characters removed.

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
    nfc_tag_data = _read_nfc_tag_data(available_nfc_readers, nfc_reader_index)
    return nfc_tag_data

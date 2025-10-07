import logging
import re
import ndef
from mobly import asserts
from smartcard.System import readers

logger = logging.getLogger(__name__)

def select_ndef_application(nrf_reader_connection_object):
    # Select NDEF Tag Application (AID: D2760000850101)
    SELECT_APDU = [0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00]
    data, sw1, sw2 = nrf_reader_connection_object.transmit(SELECT_APDU)
    asserts.assert_true((sw1, sw2) == (0x90, 0x00),f"Failed to select NDEF application: SW1={sw1:02X}, SW2={sw2:02X}")
    logging.info("NDEF application selected.")

def select_cc_file(nrf_reader_connection_object):
    # Select Capability Container file (E103)
    SELECT_CC = [0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x03]
    data, sw1, sw2 = nrf_reader_connection_object.transmit(SELECT_CC)
    asserts.assert_true((sw1, sw2) == (0x90, 0x00), f"Failed to select CC file: SW1={sw1:02X}, SW2={sw2:02X}")
    logging.info("CC file selected.")


def select_ndef_file(nrf_reader_connection_object):
    # Select NDEF file (E104)
    SELECT_NDEF = [0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x04]
    data, sw1, sw2 = nrf_reader_connection_object.transmit(SELECT_NDEF)
    asserts.assert_true((sw1, sw2) == (0x90, 0x00), f"Failed to select NDEF file: SW1={sw1:02X}, SW2={sw2:02X}")
    logging.info("NDEF file selected.")


def read_ndef_length(connection):
    # Read first 2 bytes (NDEF length)
    READ_BINARY = [0x00, 0xB0, 0x00, 0x00, 0x02]
    data, sw1, sw2 = connection.transmit(READ_BINARY)
    asserts.assert_true((sw1, sw2) == (0x90, 0x00), f"Failed to read NDEF length: SW1={sw1:02X}, SW2={sw2:02X}")
    length = (data[0] << 8) + data[1]
    logging.info(f"NDEF data length: {length} bytes")
    return length


def read_ndef_data(connection, length):
    # Read NDEF data (after the 2-byte length)
    READ_BINARY = [0x00, 0xB0, 0x00, 0x02, length]
    data, sw1, sw2 = connection.transmit(READ_BINARY)
    asserts.assert_true((sw1, sw2) == (0x90, 0x00), f"Failed to read NDEF data: SW1={sw1:02X}, SW2={sw2:02X}")
    return bytes(data)

def read_nfc_tag_data(nrf_reader_object):
    nrf_reader_connection_object = nrf_reader_object[0].createConnection()
    nrf_reader_connection_object.connect()
    select_ndef_application(nrf_reader_connection_object)
    select_cc_file(nrf_reader_connection_object)
    select_ndef_file(nrf_reader_connection_object)
    ndef_length = read_ndef_length(nrf_reader_connection_object)
    ndef_data = read_ndef_data(nrf_reader_connection_object, ndef_length)
    ndef_records = list(ndef.message_decoder(ndef_data))
    record_data = ndef_records[0].data.decode("utf-8")
    return re.sub(r'[^\x20-\x7E]', '', record_data)

def connect_read_nfc_tag_data():
    nrf_reader_object = readers()
    asserts.assert_true(len(nrf_reader_object) > 0,"No NFC readers found.")
    logging.info(f"Available readers are: {nrf_reader_object} \n will use {nrf_reader_object[0]}")
    nfc_tag_data = read_nfc_tag_data(nrf_reader_object)
    return nfc_tag_data
# -*- coding: utf-8 -*-
"""A package for parsing, handling, and generating NDEF messages.

The NFC Data Exchange Format (NDEF) is a binary message format that
can be used to encapsulate one or more application-defined payloads
into a single message construct for exchange between NFC Devices and
Tags. Each payload is described by a type, a length, and an optional
identifier. The payload type determines the syntactical requirements
for decoding and encoding of the payload octets. The optional payload
identifier allows an application to locate a specific record within
the NDEF Message.

An NDEF Message is a sequence of one or more NDEF Records, started
with a Message Begin (MB) flag in the first record and terminated with
a Message End (ME) flag in the last record. By convention, the first
record provides the processing context for the whole NDEF Message.

An NDEF Message can be decoded from bytes, bytearray, or a file-like,
byte-oriented stream with the ndef.message_decoder() generator
function. Note that the NDEF Records are decoded sequentially while
iterating, it is thus possible to successfully decode until some
malformed record later in the message.

>>> octets = bytearray.fromhex('910301414243005903010158595a30ff')
>>> for record in ndef.message_decoder(octets): print(record)
...
NDEF Record urn:nfc:wkt:ABC PAYLOAD 1 byte 00
NDEF Record urn:nfc:wkt:XYZ ID 0 PAYLOAD 1 byte ff

An NDEF Message can be encoded to bytes or into a stream with the
ndef.message_encoder() generator function. When not encoding into a
stream the generator returns the bytes for each encoded record,
joining them produces the complete message bytes.

>>> record_1 = ndef.Record('urn:nfc:wkt:ABC', None, b'\\x00')
>>> record_2 = ndef.Record('urn:nfc:wkt:XYZ', '0', b'\\xff')
>>> octets = b''.join(ndef.message_encoder([record_1, record_2]))
>>> bytearray(octets).hex() # octets.encode('hex') for Python 2
'910301414243005903010158595a30ff'

An ndef.Record is initialized with the record type, name, and
data. The data argument is the sequence of octets that will become the
NDEF Record PAYLOAD. The name argument gives the NDEF Record ID with
up to 255 latin characters. The type argument gives the NDEF Record
TNF (Type Name Format) and TYPE field in a combined string
representation. For NFC Forum well-known and external record types
this is the prefix 'urn:nfc:wkt:' and 'urn:nfc:ext:' followed by
TYPE. The media-type and absolute URI type name formats ares
recognized by '<media-type>/<subtype>' and '<scheme>://<here-part>
pattern matching. The unknown and unchanged (chunked) type name
formats are selected with the single words 'unknown' and
'unchanged'. Finally, an empty string (or None) selects the empty
record type name format.

The ndef.Record type, name, and data attributes provide access to the
init arguments (None arguments are set as to defaults). The type
attribute is generally read-only while the name attribute is
read-writable. The data attribute is read-only but is a bytearray that
can itself be modified. Note that for derived record classes the data
attribute is an non-mutable bytes object.

The ndef.message_decoder() may return and the ndef.message_encoder()
does also accept ndef.Record derived class instances. Those are either
implemented within the ndef package or may be registered by the
application with ndef.Record.register_type().

>>> class MyRecord(ndef.Record):
...     _type = 'urn:nfc:ext:nfcpy.org:MyRecord'
...     def __init__(self, integer):
...         self._value = value
...     def _encode_payload(self):
...         return struct.pack('>H', self._value)
...     _decode_min_payload_length = 2
...     _decode_max_payload_length = 2
...     @classmethod
...     def _decode_payload(cls, octets, errors, known_types):
...         return cls(struct.unpack('>H', octets)[0])
...
>>> ndef.Record.register_type(MyRecord)

The ndef package provides a number of well-known record type classes,
specifically the NFC Forum Text, URI, Smartposter and Connection
Handover Records, as well as the Bluetooth SIG and Wi-Fi Alliance
Carrier Configuration Records for connection handover. They are
documented in the package contents files.

"""
import sys
if sys.version_info < (2, 7):  # pragma: no cover
    raise ImportError("The ndef module requires Python 2.7 or newer!")
else:
    from . import message
    from . import record
    from . import uri
    from . import text
    from . import smartposter
    from . import deviceinfo
    from . import handover
    from . import bluetooth
    from . import wifi
    from . import signature

message_decoder = message.message_decoder
message_encoder = message.message_encoder

DecodeError = record.DecodeError
EncodeError = record.EncodeError
Record = record.Record
UriRecord = uri.UriRecord
TextRecord = text.TextRecord
SmartposterRecord = smartposter.SmartposterRecord
DeviceInformationRecord = deviceinfo.DeviceInformationRecord
HandoverRequestRecord = handover.HandoverRequestRecord
HandoverSelectRecord = handover.HandoverSelectRecord
HandoverMediationRecord = handover.HandoverMediationRecord
HandoverInitiateRecord = handover.HandoverInitiateRecord
HandoverCarrierRecord = handover.HandoverCarrierRecord
WifiSimpleConfigRecord = wifi.WifiSimpleConfigRecord
WifiPeerToPeerRecord = wifi.WifiPeerToPeerRecord
BluetoothEasyPairingRecord = bluetooth.BluetoothEasyPairingRecord
BluetoothLowEnergyRecord = bluetooth.BluetoothLowEnergyRecord
SignatureRecord = signature.SignatureRecord

# METADATA ####################################################################

__version__ = "0.3.3"

__title__ = "ndef"
__description__ = "NFC Data Exchange Format decoder and encoder."
__uri__ = "https://ndeflib.readthedocs.io/"

__author__ = "Stephen Tiedemann"
__email__ = "stephen.tiedemann@gmail.com"

__license__ = "ISC"
__copyright__ = "Copyright (c) 2016 Stephen Tiedemann"

###############################################################################

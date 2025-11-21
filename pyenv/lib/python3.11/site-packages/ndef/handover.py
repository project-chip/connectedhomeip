# -*- coding: utf-8 -*-
"""Decoding and encoding of Connection Handover Records.

The NFC Forum Connection Handover specification defines a number of
Record structures that are used to exchange messages between Handover
Requester, Selector and Mediator devices to eventually establish
alternative carrier connections for additional data exchange.
Generally, a requester device sends a Handover Request Message to
announce supported alternative carriers and expects the selector
device to return a Handover Select Message with a selection of
alternative carriers supported by both devices. If the two devices are
not close enough for NFC communication, a third device may use the
Handover Mediation and Handover Initiate Messages to relay information
between the two.

Any of above mentioned Handover Messages is constructed as an NDEF
Message where the first record associates the processing context. The
Handover Record subclasses defined in this module implement the
appropriate context, i.e. record types known by context are decoded by
associated record type classes while others are decoded as generic
NDEF Records.

"""
from __future__ import absolute_import, division
from .message import message_decoder, message_encoder
from .record import Record, GlobalRecord, LocalRecord, hexlify
from .deviceinfo import DeviceInformationRecord
from .bluetooth import BluetoothEasyPairingRecord
from .bluetooth import BluetoothLowEnergyRecord
from .wifi import WifiSimpleConfigRecord
from .wifi import WifiPeerToPeerRecord
from collections import namedtuple
from io import BytesIO

try:
    from collections.abc import Sequence
except ImportError:
    from collections import Sequence

default_version = '1.3'


class AlternativeCarrierRecord(LocalRecord):
    """This is a local record class used within the payload of Handover
    Records. It encodes a carrier data reference to another record
    within the handover message along with a carrier power state
    indicator. It also encodes zero or more auxiliary data references
    that link to records with additional information about the
    alternative carrier.

    The AlternativeCarrierRecord information is normally set or
    retrieved through the interface methods and attributes of one of
    the HandoverRecord subclass. It is not intended to be directly
    used in application code.

    """
    _type = 'urn:nfc:wkt:ac'
    _cps_values = ("inactive", "active", "activating", "unknown")

    def __init__(self, cps, cdr, *adr):
        """Initialize the record with the carrier power state (cps), carrier
        data reference (cdr), and zero or more auxiliary data
        references (adr).

        """
        self.set_carrier_power_state(cps)
        self.set_carrier_data_reference(cdr)
        self.auxiliary_data_reference = []
        for reference in adr:
            self.add_auxiliary_data_reference(reference)

    @property
    def carrier_power_state(self):
        """A text value that holds the carrier power state information."""
        return self._cps_values[self._carrier_power_state]

    def set_carrier_power_state(self, value):
        """Set the carrier power state to either 'inactive', 'active',
        'activation', or 'unknown'. The value may also be an index
        into the list of carrier power states.

        """
        if isinstance(value, int) and 0 <= value < len(self._cps_values):
            self._carrier_power_state = value
        elif value in self._cps_values:
            self._carrier_power_state = self._cps_values.index(value)
        else:
            errstr = "carrier_power_state accepts index or one of {}, not {!r}"
            raise self._value_error(errstr, self._cps_values, value)

    def set_carrier_data_reference(self, reference):
        """Set the Record.name (NDEF Record ID) of another record within the
        handover message as a carrier data reference.

        """
        value, name = reference, 'set_carrier_data_reference()'
        self.carrier_data_reference = self._value_to_latin(value, name)

    def add_auxiliary_data_reference(self, reference):
        """Add the Record.name (NDEF Record ID) of another record within the
        handover message as an auxiliary data reference.

        """
        value, name = reference, 'set_auxiliary_data_reference()'
        self.auxiliary_data_reference.append(self._value_to_latin(value, name))

    def __format__(self, format_spec):
        if format_spec == 'args':
            cps = self.carrier_power_state
            cdr = self.carrier_data_reference
            adr = self.auxiliary_data_reference
            return "{!r}, {}".format(cps, ', '.join(map(repr, [cdr] + adr)))

        if format_spec == 'data':
            s = ["Carrier Reference '{r.carrier_data_reference}'"]
            s.append("Power State '{r.carrier_power_state}'")
            s.append("Auxiliary Data {r.auxiliary_data_reference}")
            return ' '.join(s).format(r=self)

        return super(AlternativeCarrierRecord, self).__format__(format_spec)

    def _encode_payload(self):
        length = 3 + len(self.carrier_data_reference) + \
                 sum([1+len(ref) for ref in self.auxiliary_data_reference])
        octets = bytearray(length)
        octets[0] = self._carrier_power_state
        offset = self._pack_ref(octets, 1, self.carrier_data_reference)
        octets[offset] = len(self.auxiliary_data_reference)
        offset = offset + 1
        for reference in self.auxiliary_data_reference:
            offset = self._pack_ref(octets, offset, reference)
        return bytes(octets)

    def _pack_ref(self, octets, offset, reference):
        length = len(reference)
        octets[offset] = length
        octets[offset+1:offset+1+length] = reference.encode('latin')
        return offset + 1 + length

    _decode_min_payload_length = 2

    @classmethod
    def _decode_payload(cls, octets, errors):
        CARRIER_POWER_STATE = cls._decode_struct("B", octets) & 0b00000011
        CARRIER_DATA_REFERENCE = cls._read_ref(octets, 1, 'carrier')
        offset = 2 + len(CARRIER_DATA_REFERENCE) + 1
        if len(octets) < offset:
            if errors == 'strict':
                errmsg = "decode is missing auxiliary data reference count"
                raise cls._decode_error(errmsg)
            else:
                return cls(CARRIER_POWER_STATE, CARRIER_DATA_REFERENCE)
        AUX_DATA_REFERENCE_COUNT = cls._decode_struct("B", octets, offset-1)
        AUX_DATA_REFERENCE_LIST = AUX_DATA_REFERENCE_COUNT * [None]
        for index in range(AUX_DATA_REFERENCE_COUNT):
            AUX_DATA_REFERENCE = cls._read_ref(octets, offset, 'auxiliary')
            AUX_DATA_REFERENCE_LIST[index] = AUX_DATA_REFERENCE
            offset += 1 + len(AUX_DATA_REFERENCE)
        if offset < len(octets) and errors == 'strict':
            errstr = "payload has {} octet left after decode"
            raise cls._decode_error(errstr, len(octets)-offset)
        return cls(CARRIER_POWER_STATE, CARRIER_DATA_REFERENCE,
                   *AUX_DATA_REFERENCE_LIST)

    @classmethod
    def _read_ref(cls, octets, offset, name):
        if offset >= len(octets):
            errstr = "decode is missing {} data reference length"
            raise cls._decode_error(errstr, name)
        return cls._decode_struct('B+', octets, offset).decode('latin')


class CollisionResolutionRecord(LocalRecord):
    """This is a local record class used within the payload of a Handover
    Request Record. It encodes a 16-bit random number that is used to
    relove a collision of two handover request messages sent
    simultaneously by two NFC peer devices and select the request
    message that is to be processed.

    The CollisionResolutionRecord information is normally set or
    retrieved through the interface methods and attributes of the
    HandoverRequestRecord class. It is not intended to be directly
    used in application code.

    """
    _type = 'urn:nfc:wkt:cr'

    def __init__(self, random_number=0):
        """Initialize the record with a random number, which must fit into a
        16-bit unsigned integer.

        """
        self.random_number = random_number

    @property
    def random_number(self):
        """Get or set the random number for handover request collision
        resolution. A set value must be either a positive integer less
        than 65536 or two bytes with a 16-bit value in big endian byte
        order.

        """
        return self._random_number

    @random_number.setter
    def random_number(self, value):
        if not isinstance(value, (int, bytes)):
            errstr = "random_number expects an int or bytes, but not {}"
            raise self._type_error(errstr, type(value).__name__)
        if isinstance(value, bytes):
            value = self._decode_struct(">H", value)
        if not 0 <= value <= 0xffff:
            errstr = "random_number must be 0 <= x <= 65535, got {}"
            raise self._value_error(errstr, value)
        self._random_number = value

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{r.random_number!r}".format(r=self)

        if format_spec == 'data':
            return "Random Number {r.random_number}".format(r=self)

        return super(CollisionResolutionRecord, self).__format__(format_spec)

    def _encode_payload(self):
        return self._encode_struct(">H", self.random_number)

    _decode_min_payload_length = 2
    _decode_max_payload_length = 2

    @classmethod
    def _decode_payload(cls, octets, errors):
        RANDOM_NUMBER = cls._decode_struct(">H", octets)
        return cls(RANDOM_NUMBER)


class ErrorRecord(LocalRecord):
    """This is a local record class used within the payload of a Handover
    Select Record. It encodes an 8-bit error reason identifier and an
    error data field that depends on the error reason code. For the
    defined error reason values 1 to 3 the error data field structure
    is well-known and decoded or encoded accordingly. For the
    undefined (reserved for future use) error reason values 4 to 255
    the error data field is treated as bytes. The reserved error
    reason value 0 is treated as invalid and causes a decode or encode
    exception.

    The ErrorRecord information is normally set or retrieved through
    the interface methods and attributes of the HandoverSelectRecord
    class. It is not intended to be directly used in application code.

    """
    _type = 'urn:nfc:wkt:err'
    _error_reason_strings = (
        "temporarily out of memory, may retry after {} milliseconds",
        "permanently out of memory, may retry with at most {} octets",
        "carrier specific error, may retry after {} milliseconds")

    def __init__(self, error_reason=None, error_data=None):
        """Initialize the record with error reason and error data
        information. None values default to error reason value 0 and
        empty error data bytes, but note that error reason 0 will
        raise an exception when encoding.

        """
        self.error_reason = error_reason if error_reason is not None else 0
        self.error_data = error_data if error_data is not None else b''

    @property
    def error_reason(self):
        """Get or set the error reason value. A set value is convert into a
        base-10 integer without further validation.

        """
        return self._error_reason

    @error_reason.setter
    def error_reason(self, value):
        self._error_reason = int(value)

    @property
    def error_data(self):
        """Get or set the error data value. A set value is converted into a
        base-10 integer if the present state of Error.error_reason is
        1, 2 or 3, otherwise it is converted into bytes.

        """
        return self._error_data

    @error_data.setter
    def error_data(self, value):
        if 1 <= self.error_reason <= 3:
            self._error_data = int(value)
        else:
            self._error_data = bytes(value)

    @property
    def error_string(self):
        """A read-only attribute that provides a human readable description of
        the error reason and error data value.

        """
        if 1 <= self._error_reason <= 3:
            reason_string = self._error_reason_strings[self._error_reason-1]
            return reason_string.format(self.error_data)
        else:
            return "Reason {} Data {}".format(
                self.error_reason, hexlify(self.error_data))

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{r.error_reason!r}, {r.error_data!r}".format(r=self)

        if format_spec == 'data':
            return "Error '{r.error_string}'".format(r=self)

        return super(ErrorRecord, self).__format__(format_spec)

    def _encode_payload(self):
        if self.error_reason == 0:
            raise self._encode_error("can't encode reserved error reason 0")

        fmt = ('BB', '>BL', 'BB', 'B*')[min(self.error_reason-1, 3)]
        return self._encode_struct(fmt, self.error_reason, self.error_data)

    _decode_min_payload_length = 1

    @classmethod
    def _decode_payload(cls, octets, errors):
        ERROR_REASON = cls._decode_struct("B", octets)
        if ERROR_REASON == 0:
            raise cls._decode_error("can't decode reserved error reason 0")
        if ERROR_REASON <= 3:
            error_data_size = (1, 4, 1)[ERROR_REASON-1]
            if len(octets) - 1 < error_data_size:
                errstr = "payload needs {} octet error data for reason {}"
                raise cls._decode_error(errstr, error_data_size, ERROR_REASON)
            if len(octets) - 1 > error_data_size and errors == 'strict':
                errstr = "payload has {} octet left after decode"
                raise cls._decode_error(errstr, len(octets)-error_data_size-1)
        error_data_fmt = ('B', '>L', 'B', '*')[min(ERROR_REASON-1, 3)]
        ERROR_DATA = cls._decode_struct(error_data_fmt, octets, 1)
        return cls(ERROR_REASON, ERROR_DATA)


class HandoverRecord(GlobalRecord):
    """This is the base class for the Handover Request, Select, Mediation,
    and Initiate Records. It implements decoding and encoding of
    information that is common to all its subclasses, specifically the
    handover version information and alternative carrier records. It
    is not intended for direct instantiation.

    """
    Version = namedtuple('Version', 'major, minor')

    def __init__(self, version, *alternative_carrier):
        """Initialize the record with the handover version number and zero or
        more alternative carriers. The version number may be set as an
        8-bit integer (with 4-bit major and minor parts), or as a
        '{major}.{minor}' version string. Each alternative carrier
        must be set as a tuple with carrier power state, carrier data
        reference and zero or more auxiliary data references.

        """
        if isinstance(version, int):
            self._version = version & 0xFF
        elif isinstance(version, str):
            try:
                major, minor = map(int, version.split('.')[0:2])
                self._version = (major << 4 & 0xF0) | (minor & 0xF)
            except ValueError:
                errstr = "can't parse {!r} as a version string"
                raise self._value_error(errstr, version)
        else:
            errstr = "version argument expects int or str, but not {}"
            raise self._type_error(errstr, type(version).__name__)
        self.alternative_carrier_records = []
        for ac in alternative_carrier:
            self.add_alternative_carrier(*ac)
        self.unknown_records = []

    @property
    def hexversion(self):
        """The version as an 8-bit integer with 4-bit major and minor
        part. This is a read-only attribute.

        """
        return self._version

    @property
    def version_info(self):
        """The version as a named tuple with major and minor version number
        attributes. This is a read-only attribute.

        """
        return self.Version(self._version >> 4, self._version & 15)

    @property
    def version_string(self):
        """The version as the '{major}.{minor}' formatted string. This is a
        read-only attribute.

        """
        return "{}.{}".format(*self.version_info)

    @property
    def alternative_carriers(self):
        """List of alternative carriers. Each item has the attributes
        carrier_power_state, carrier_data_reference, and the
        auxiliary_data_reference list.

        """
        return self.alternative_carrier_records

    def add_alternative_carrier(self, cps, cdr, *adr):
        """Add a reference to a carrier data record within the handover
        request message. The carrier data reference cdr must be the
        name (NDEF Record ID) of the carrier data record. The carrier
        power state cps must be set to either 'inactive', 'active',
        'activating', or 'unknown'. Any number of auxiliary data
        referenece adr may be added for other records in the message
        that carry information related to the carrier.

        """
        self.alternative_carrier_records.append(
            AlternativeCarrierRecord(cps, cdr, *adr))

    def __format__(self, format_spec):
        if format_spec == 'args':
            ver = "{!r}".format(self.version_string)
            acr = self.alternative_carrier_records
            acs = ', '.join(["({:args})".format(ac) for ac in acr])
            return "{}{{}}{}{}".format(ver, ', ' if acr else '', acs)

        if format_spec == 'data':
            s = ["Version '{r.version_string}'".format(r=self)]
            for acr in self.alternative_carrier_records:
                s.append("{:data}".format(acr))
            return ' '.join(s)

        return super(HandoverRecord, self).__format__(format_spec)

    _encode_records = [
        "alternative_carrier_records",
        "unknown_records"]

    def _encode_payload(self):
        stream = BytesIO()
        encoder = message_encoder(stream=stream)
        encoder.send(None)
        for name in self._encode_records:
            for record in getattr(self, name):
                encoder.send(record)
        encoder.send(None)
        return self._encode_struct('B*', self.hexversion, stream.getvalue())

    _decode_min_payload_length = 1
    _decode_records = [
        (AlternativeCarrierRecord._type, 'alternative_carrier_records')]

    @staticmethod
    def _decode_payload(cls, octets, errors):
        mapping = dict(HandoverRecord._decode_records + cls._decode_records)
        hrecord = cls(cls._decode_struct('B', octets))
        decoder = message_decoder(octets[1:], errors, hrecord._known_types)
        for record in decoder:
            if record.type in mapping:
                getattr(hrecord, mapping[record.type]).append(record)
            else:
                hrecord.unknown_records.append(record)
        return hrecord


class HandoverRequestRecord(HandoverRecord):
    """The HandoverRequestRecord is the first record of a connection
    handover request message. Information enclosed within the payload
    of a handover request record includes the handover version number,
    a random number for resolving a handover request collision (when
    both peer devices simultaenously send a handover request message)
    and a number of references to alternative carrier information
    records subsequently encoded in the same message.

    >>> import ndef
    >>> from os import urandom
    >>> wsc = 'application/vnd.wfa.wsc'
    >>> message = [ndef.HandoverRequestRecord('1.3', urandom(2))]
    >>> message.append(ndef.HandoverCarrierRecord(wsc, None, 'wifi'))
    >>> message[0].add_alternative_carrier('active', message[1].name)

    """
    _type = 'urn:nfc:wkt:Hr'

    def __init__(self, version=default_version, crn=None,
                 *alternative_carrier):
        """Initialize the record with a version number, a collision resolution
        random number crn and zero or more alternative carriers. The
        version and alternative carrier arguments are handled by the
        HandoverRecord class initialization method. The collision
        resolution number (crn) argument must be a 16-bit random
        unsigned integer for connection handover version '1.2' or
        later, for any prior version number it must be None.

        """
        super(type(self), self).__init__(version, *alternative_carrier)
        self.collision_resolution_records = []
        if crn is not None:
            self.collision_resolution_number = crn

    @property
    def collision_resolution_number(self):
        """Get or set the random number for collision resolution."""
        try:
            return self.collision_resolution_records[0].random_number
        except IndexError:
            return None

    @collision_resolution_number.setter
    def collision_resolution_number(self, value):
        if len(self.collision_resolution_records) == 0:
            self.collision_resolution_records = [CollisionResolutionRecord()]
        self.collision_resolution_records[0].random_number = value

    def __format__(self, format_spec):
        if format_spec == 'args':
            crn = ", {}".format(self.collision_resolution_number)
            return HandoverRecord.__format__(self, format_spec).format(crn)

        if format_spec == 'data':
            return HandoverRecord.__format__(self, format_spec)

        return super(HandoverRequestRecord, self).__format__(format_spec)

    _encode_records = [
        "collision_resolution_records",
    ] + HandoverRecord._encode_records

    def _encode_payload(self):
        if self.hexversion > 0x11 and self.collision_resolution_number is None:
            errstr = "can't encode {} without collision resolution record"
            raise self._encode_error(errstr, "version " + self.version_string)
        return super(type(self), self)._encode_payload()

    _decode_records = [
        (CollisionResolutionRecord._type, 'collision_resolution_records')]

    @classmethod
    def _decode_payload(cls, octets, errors):
        hr = HandoverRecord._decode_payload(cls, octets, errors)
        if errors == 'strict' and hr.hexversion > 0x11:
            if hr.collision_resolution_number is None:
                errstr = "can't decode {} without collision resolution record"
                raise cls._decode_error(errstr, "version " + hr.version_string)
        return hr


class HandoverSelectRecord(HandoverRecord):
    """The HandoverSelectRecord is the first record of a connection
    handover select message. Information enclosed within the payload
    of a handover select record includes the handover version number,
    error reason and associated error data when processing of the
    previously received handover request message failed, and a number
    of references to alternative carrier information records
    subsequently encoded in the same message.

    >>> import ndef
    >>> carrier = ndef.Record('mimetype/subtype', 'wifi', b'1234')
    >>> message = [ndef.HandoverSelectRecord('1.3'), carrier]
    >>> message[0].add_alternative_carrier('active', carrier.name)

    """
    _type = 'urn:nfc:wkt:Hs'

    def __init__(self, version=default_version, error=None,
                 *alternative_carrier):
        """Initialize the record with a version number, an error information
        tuple, and zero or more alternative carriers. The version and
        alternative carrier arguments are handled by the HandoverRecord
        class initialization method. The error argument must be a tuple
        with error reason and error data or None. Error information, if
        not None, is encoded as Error Record after all set alternative
        carriers.

        """
        super(type(self), self).__init__(version, *alternative_carrier)
        self.error_records = []
        if error is not None:
            if not (isinstance(error, (tuple, list))):
                errstr = "can't initialize error attribute from {!r}"
                raise self._value_error(errstr, error)
            self.set_error(*error)

    @property
    def error(self):
        """Either error information or None. Error details can be accessed
        through the error_reason and error_data attributes. Formatted
        error information is provided with the error_reason_string
        attribute.

        """
        try:
            return self.error_records[0]
        except IndexError:
            return None

    def set_error(self, error_reason, error_data):
        """Set error infomation. For error reason and data argument
        help consult the ErrorRecord documentation.

        """
        if len(self.error_records) == 0:
            self.error_records.append(ErrorRecord())
        self.error_records[0].error_reason = error_reason
        self.error_records[0].error_data = error_data

    def __format__(self, format_spec):
        if format_spec == 'args':
            err = ", ({:args})".format(self.error) if self.error else ', None'
            return HandoverRecord.__format__(self, format_spec).format(err)

        if format_spec == 'data':
            s = HandoverRecord.__format__(self, format_spec)
            if self.error is not None:
                s += " {r.error:data}".format(r=self)
            return s

        return super(HandoverSelectRecord, self).__format__(format_spec)

    _encode_records = HandoverRecord._encode_records + ["error_records"]

    def _encode_payload(self):
        if self.hexversion < 0x12 and self.error_records:
            errstr = "can't encode error record for version {}"
            raise self._encode_error(errstr, self.version_string)
        return super(type(self), self)._encode_payload()

    _decode_records = [(ErrorRecord._type, 'error_records')]

    @classmethod
    def _decode_payload(cls, octets, errors):
        return HandoverRecord._decode_payload(cls, octets, errors)


class HandoverMediationRecord(HandoverRecord):
    """The HandoverMediationRecord is the first record of a connection
    handover mediation message. Information enclosed within the
    payload of a handover mediation record includes the version number
    and zero or more references to alternative carrier information
    records subsequently encoded in the same message.

    """
    _type = 'urn:nfc:wkt:Hm'

    def __init__(self, version=default_version, *alternative_carrier):
        """Initialize the record with a version number and zero or more
        alternative carriers, directly handled by the HandoverRecord
        class initialization method.

        """
        super(type(self), self).__init__(version, *alternative_carrier)

    def __format__(self, format_spec):
        if format_spec == 'args':
            return HandoverRecord.__format__(self, format_spec).format('')

        if format_spec == 'data':
            return HandoverRecord.__format__(self, format_spec)

        return super(HandoverMediationRecord, self).__format__(format_spec)

    @classmethod
    def _decode_payload(cls, octets, errors):
        return HandoverRecord._decode_payload(cls, octets, errors)


class HandoverInitiateRecord(HandoverRecord):
    """The HandoverInitiateRecord is the first record of a connection
    handover initiate message. Information enclosed within the payload
    of a handover initiate record includes the version number and zero
    or more references to alternative carrier information records
    subsequently encoded in the same message.

    """
    _type = 'urn:nfc:wkt:Hi'

    def __init__(self, version=default_version, *alternative_carrier):
        """Initialize the record with a version number and zero or more
        alternative carriers, directly handled by the HandoverRecord
        class initialization method.

        """
        super(type(self), self).__init__(version, *alternative_carrier)

    def __format__(self, format_spec):
        if format_spec == 'args':
            return HandoverRecord.__format__(self, format_spec).format('')

        if format_spec == 'data':
            return HandoverRecord.__format__(self, format_spec)

        return super(HandoverInitiateRecord, self).__format__(format_spec)

    @classmethod
    def _decode_payload(cls, octets, errors):
        return HandoverRecord._decode_payload(cls, octets, errors)


class HandoverCarrierRecord(GlobalRecord):
    """The HandoverCarrierRecord allows a unique identification of an
    alternative carrier technology in a handover request message when
    no carrier configuration data is to be provided. If the handover
    selector device has the same carrier technology available, it
    would respond with a carrier configuration record with payload
    type equal to the carrier type (that is, the triples (TNF,
    TYPE_LENGTH, TYPE) and (CTF, CARRIER_TYPE_LENGTH, CARRIER_TYPE)
    match exactly).

    """
    _type = 'urn:nfc:wkt:Hc'

    def __init__(self, carrier_type=None, carrier_data=None, reference=None):
        """Initialize the HandoverCarrierRecord with a carrier type,
        potentially some carrier data, and a reference that sets the
        Record.name attribute. The carrier type name must be formatted
        like a record type name, i.e. the combination of NDEF Record
        TNF and TYPE that is used by the Record.type attribute. The
        carrier_data argument must be convertible into a bytarray if
        not None.

        """
        self.carrier_type = carrier_type
        if carrier_data is None:
            self._carrier_data = bytearray()
        elif isinstance(carrier_data, str):
            self._carrier_data = bytearray(carrier_data.encode('latin'))
        elif isinstance(carrier_data, (bytearray, Sequence)):
            self._carrier_data = bytearray(carrier_data)
        else:
            errstr = "carrier data may be sequence type or None, but not {}"
            raise self._type_error(errstr, type(carrier_data).__name__)
        self.name = reference

    @property
    def carrier_type(self):
        """Get or set the carrier type as a Record.type formatted
        representation of the Handover Carrier Record CTF and
        CARRIER_TYPE fields.

        """
        return self._carrier_type

    @carrier_type.setter
    def carrier_type(self, value):
        self._carrier_type = self._decode_type(*self._encode_type(value))

    @property
    def carrier_data(self):
        """Contents of the Handover Carrier Record CARRIER_DATA field as a
        bytearray. The attribute itself is read-only but the contents
        of the bytearray may be modified or expanded.

        """
        return self._carrier_data

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{!r}, {!r}".format(self.carrier_type, self.carrier_data)

        if format_spec == 'data':
            _data = self.carrier_data
            s = ["CARRIER '{}'".format(self.carrier_type)]
            s.append("DATA {} byte".format(len(_data)))
            if len(_data) > 0:
                s.append("'{}'".format(hexlify(_data[0:10])))
            if len(_data) > 10:
                s.append("... {} more".format(len(_data)-10))
            return ' '.join(s)

        return super(HandoverCarrierRecord, self).__format__(format_spec)

    def _encode_payload(self):
        CTF, CARRIER_TYPE = self._encode_type(self.carrier_type)
        CARRIER_DATA = self.carrier_data
        return self._encode_struct("BB+", CTF, CARRIER_TYPE) + CARRIER_DATA

    _decode_min_payload_length = 2

    @classmethod
    def _decode_payload(cls, octets, errors):
        CTF, CARRIER_TYPE_LENGTH = cls._decode_struct("BB", octets)
        CARRIER_TYPE = octets[2:2+CARRIER_TYPE_LENGTH]
        if len(CARRIER_TYPE) != CARRIER_TYPE_LENGTH:
            errstr = "carrier type length {} exceeds payload size"
            raise cls._decode_error(errstr, CARRIER_TYPE_LENGTH)
        carrier_type = cls._decode_type(CTF & 0b111, CARRIER_TYPE)
        carrier_data = octets[2+CARRIER_TYPE_LENGTH:]
        return HandoverCarrierRecord(carrier_type, carrier_data)


HandoverRequestRecord.register_type(AlternativeCarrierRecord)
HandoverRequestRecord.register_type(CollisionResolutionRecord)
HandoverRequestRecord.register_type(HandoverCarrierRecord)
HandoverRequestRecord.register_type(DeviceInformationRecord)
HandoverRequestRecord.register_type(WifiSimpleConfigRecord)
HandoverRequestRecord.register_type(WifiPeerToPeerRecord)
HandoverRequestRecord.register_type(BluetoothEasyPairingRecord)
HandoverRequestRecord.register_type(BluetoothLowEnergyRecord)

HandoverSelectRecord.register_type(AlternativeCarrierRecord)
HandoverSelectRecord.register_type(ErrorRecord)
HandoverSelectRecord.register_type(DeviceInformationRecord)
HandoverSelectRecord.register_type(WifiSimpleConfigRecord)
HandoverSelectRecord.register_type(WifiPeerToPeerRecord)
HandoverSelectRecord.register_type(BluetoothEasyPairingRecord)
HandoverSelectRecord.register_type(BluetoothLowEnergyRecord)

HandoverMediationRecord.register_type(AlternativeCarrierRecord)
HandoverInitiateRecord.register_type(AlternativeCarrierRecord)

Record.register_type(HandoverRequestRecord)
Record.register_type(HandoverSelectRecord)
Record.register_type(HandoverMediationRecord)
Record.register_type(HandoverInitiateRecord)

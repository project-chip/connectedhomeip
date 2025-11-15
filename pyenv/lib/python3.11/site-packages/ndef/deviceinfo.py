# -*- coding: utf-8 -*-
"""Decoding and encoding of the NDEF Device Information Record.

The NDEF Device Information Record is a well-known record type defined
by the NFC Forum. It carries a number of Type-Length-Value data
elements that provide information about the device, such as the
manufacturer and device model name.

"""
from __future__ import absolute_import, division
from .record import Record, GlobalRecord, convert
from collections import namedtuple
import uuid


class DeviceInformationRecord(GlobalRecord):
    """This class decodes or encodes an NDEF Device Information Record and
    provides attributes for the information elements. Individual
    attributes are available for the information elements defined in
    the NFC Forum specification. Undefined, i.e. reserved-future-use,
    elements are provided with the undefined_data_elements list
    attribute and may be added with the add_undefined_data_element
    method (this should only be used if a future element definition
    is not yet implemented).

    >>> record = ndef.DeviceInformationRecord('ABC Company', 'Device XYZ')

    """
    _type = 'urn:nfc:wkt:Di'

    _DataElement = namedtuple('DataElement', 'data_type, data_bytes')

    def __init__(self, vendor_name, model_name, unique_name=None,
                 uuid_string=None, version_string=None, *undefined_data):
        """Initialize the record with required and optional device
        information. The vendor_name and model_name arguments are
        required, all other arguments are optional information.

        The first five positional arguments set the identically named
        record attributes. All further positional arguments must each
        be a tuple with data_type and data_bytes arguments for the
        add_undefined_data_element method.

        """
        self._unique_name = self._uuid = self._version_string = None
        self._unknown_tlvs = []
        self.vendor_name = vendor_name
        self.model_name = model_name
        if unique_name is not None:
            self.unique_name = unique_name
        if uuid_string is not None:
            self.uuid_string = uuid_string
        if version_string is not None:
            self.version_string = version_string
        for data_type, data_bytes in undefined_data:
            self.add_undefined_data_element(data_type, data_bytes)

    @property
    def vendor_name(self):
        """Get or set the device vendor name string."""
        return self._vendor_name

    @vendor_name.setter
    @convert('value_to_unicode')
    def vendor_name(self, value):
        self._vendor_name = value

    @property
    def model_name(self):
        """Get or set the device model name string."""
        return self._model_name

    @model_name.setter
    @convert('value_to_unicode')
    def model_name(self, value):
        self._model_name = value

    @property
    def unique_name(self):
        """Get or set the device unique name string."""
        return self._unique_name

    @unique_name.setter
    @convert('value_to_unicode')
    def unique_name(self, value):
        self._unique_name = value

    @property
    def uuid_string(self):
        """Get or set the universially unique identifier string."""
        return str(self._uuid) if self._uuid else None

    @uuid_string.setter
    @convert('value_to_ascii')
    def uuid_string(self, value):
        self._uuid = uuid.UUID(value)

    @property
    def version_string(self):
        """Get or set the device firmware version string."""
        return self._version_string

    @version_string.setter
    @convert('value_to_unicode')
    def version_string(self, value):
        self._version_string = value

    @property
    def undefined_data_elements(self):
        """A list of undefined data elements as named tuples with data_type
        and data_bytes attributes. This is a reference to the internal
        list and may thus be updated in-place but it is strongly
        recommended to use the add_undefined_data_element method with
        data_type and data_bytes validation. It would also not be safe
        to rely on such implementation detail.

        """
        return self._unknown_tlvs

    def add_undefined_data_element(self, data_type, data_bytes):
        """Add an undefined (reserved future use) device information data
        element. The data_type must be an an integer in range(5, 256). The
        data_bytes argument provides the up to 255 octets to transmit.

        Undefined data elements should not normally be added. This
        method is primarily present to allow transmission of data
        elements defined by future revisions of the specification
        before this implementaion is potentially updated.

        """
        if not isinstance(data_type, int):
            errstr = "data_type argument must be int, not '{}'"
            raise self._value_error(errstr, type(data_type).__name__)
        if not isinstance(data_bytes, (bytes, bytearray)):
            errstr = "data_bytes may be bytes or bytearray, but not '{}'"
            raise self._value_error(errstr, type(data_bytes).__name__)
        if not 5 <= data_type <= 255:
            errstr = "data_type argument must be in range(5, 256), got {}"
            raise self._value_error(errstr, data_type)
        if len(data_bytes) > 255:
            errstr = "data_bytes can not be more than 255 octets, got {}"
            raise self._value_error(errstr, len(data_bytes))
        self._unknown_tlvs.append(self._DataElement(data_type, data_bytes))

    def __format__(self, format_spec):
        if format_spec == 'args':
            s = ("{r.vendor_name!r}, {r.model_name!r}, {r.unique_name!r}, "
                 "{r.uuid_string!r}, {r.version_string!r}".format(r=self))
            if self.undefined_data_elements:
                for data_type, data_bytes in self.undefined_data_elements:
                    s += ", ({!r}, {!r})".format(data_type, data_bytes)
            return s

        if format_spec == 'data':
            s = "Vendor '{r.vendor_name}' Model '{r.model_name}'"
            if self.unique_name:
                s += " Name '{r.unique_name}'"
            if self.uuid_string:
                s += " UUID '{r.uuid_string}'"
            if self.version_string:
                s += " Version '{r.version_string}'"
            for item in self.undefined_data_elements:
                s += " {}".format(item)
            return s.format(r=self)

        return format(str(self), format_spec)

    def _encode_payload(self):
        if not (self.vendor_name and self.model_name):
            errmsg = "encoding requires that vendor and model name are set"
            raise self._encode_error(errmsg)

        def encode(t, v): return self._encode_struct('BB+', t, v)
        octets = encode(0, self.vendor_name.encode('utf-8'))
        octets += encode(1, self.model_name.encode('utf-8'))
        if self.unique_name is not None:
            octets += encode(2, self.unique_name.encode('utf-8'))
        if self._uuid is not None:
            octets += encode(3, self._uuid.bytes)
        if self.version_string is not None:
            octets += encode(4, self.version_string.encode('utf-8'))
        for tlv_type, tlv_value in self.undefined_data_elements:
            octets += encode(tlv_type, tlv_value)

        return octets

    _decode_min_payload_length = 2

    @classmethod
    def _decode_payload(cls, octets, errors):
        record = cls('', '')
        offset = 0
        while offset < len(octets):
            tlv_type, tlv_value = cls._decode_struct('BB+', octets, offset)
            offset = offset + 2 + len(tlv_value)
            if tlv_type == 0:
                record.vendor_name = tlv_value.decode('utf-8')
            elif tlv_type == 1:
                record.model_name = tlv_value.decode('utf-8')
            elif tlv_type == 2:
                record.unique_name = tlv_value.decode('utf-8')
            elif tlv_type == 3:
                record._uuid = uuid.UUID(bytes=tlv_value)
            elif tlv_type == 4:
                record.version_string = tlv_value.decode('utf-8')
            else:
                record.add_undefined_data_element(tlv_type, tlv_value)

        if not (record.vendor_name and record.model_name):
            errmsg = "decoding requires the manufacturer and model name TLVs"
            raise cls._decode_error(errmsg)

        return record


Record.register_type(DeviceInformationRecord)

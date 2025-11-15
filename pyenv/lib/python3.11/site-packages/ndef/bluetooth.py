# -*- coding: utf-8 -*-
"""Decoding and encoding of Bluetooth Out-Of-Band Records.

"""
from __future__ import absolute_import, division
from .record import Record, GlobalRecord, convert, _PY2
from .record import decode_error, encode_error
from uuid import UUID
import struct
import re


class DeviceAddress(object):
    """Encapsulation a Bluetooth Device Address.

    Since Bluetooth 4.0 the transmission format of a Device Address
    may be 6 (Bluetooth BR/EDR) or 7 (Bluetooth LE) octets. The
    seventh octet distincts between a public and a random address.

    """
    def __init__(self, address, address_type='public'):
        """Initialize the object with an address and address_type.

        The address is a string of 6 hexadecimal octets separated by
        colon or dash (which can be mixed). The address_type must be
        either 'public' (default) or 'random'.

        """
        self.addr = address
        self.type = address_type

    def __eq__(self, other):
        """Return True if self equals other, False if not.

        """
        return (self.addr, self.type) == (other.addr, other.type)

    def __str__(self):
        """Return a formatted representation suitable for printing.

        """
        return "Device Address {addr.addr} ({addr.type})".format(addr=self)

    def __repr__(self):
        """Return a formal representation suitable for evaluation.

        """
        return "{}.{}({!r}, {!r})".format(
            self.__module__, self.__class__.__name__, self.addr, self.type)

    @property
    def addr(self):
        """Get or set the address part of a device address.

        The address part is given by a text string of hexadecimal
        octets separated by colon or dash. For examples, the strings
        '01:02:03:04:05:06', '01-02-03-04-05-06', '01-02:03-04:05-06'
        are all the same.

        """
        return self._addr

    @addr.setter
    def addr(self, value):
        assert re.match(r'([0-9A-F]{2}[:-]){5}([0-9A-F]{2})$', value)
        self._addr = re.sub(r'-', ':', value)

    @property
    def type(self):
        """Get or set the address type as either 'public' or 'random'.

        """
        return self._type

    @type.setter
    def type(self, value):
        assert value in ('public', 'random')
        self._type = value

    def encode(self, context='LE'):
        """Return the encoded representation suitable for transmission.

        The context argument determines the number of bytes that are
        returned. If context requires an 'LE' address encoding then 7
        bytes are returned. If context requires an 'EP' (Easy Pairing)
        BD_ADDR encoding then 6 bytes are returned.

        """
        assert context in ('LE', 'EP')
        octets = bytearray(int(x, 16) for x in reversed(self.addr.split(':')))
        if context == 'LE':
            octets.append({'public': 0, 'random': 1}[self.type])
        return bytes(octets)

    @classmethod
    def decode(cls, octets):
        """Return a class instance initialized from octets.

        The octets argument is a byte sequence of length 6 (BD_ADDR)
        or 7 (LE address).

        """
        if len(octets) == 6:
            addr_t = 'public'
            addr_v = ':'.join(['%02X' % x for x in bytearray(octets[::-1])])
            return cls(addr_v, addr_t)
        elif len(octets) == 7:
            addr_v, addr_t = struct.unpack('6sB', octets)
            addr_t = 'random' if addr_t & 1 else 'public'
            addr_v = ':'.join(['%02X' % x for x in bytearray(addr_v[::-1])])
            return cls(addr_v, addr_t)
        else:
            errstr = "can't be decoded from {} octets"
            raise decode_error(cls, errstr, len(octets))


class DeviceClass(object):
    """Encapsulation of a Bluetooth Class of Device/Service (CoD).

    The Bluetooth Class of Device format is defined at
    https://www.bluetooth.com/specifications/assigned-numbers/baseband

    """
    service_class_name = (
        "Limited Discoverable Mode", "Reserved (bit 14)", "Reserved (bit 15)",
        "Positioning", "Networking", "Rendering", "Capturing",
        "Object Transfer", "Audio", "Telephony", "Information")

    device_class = {
        0: ('Miscellaneous', (
            {
                '000000': 'Uncategorized',
            },)),
        1: ('Computer', (
            {
                '000000': 'Uncategorized',
                '000001': 'Desktop workstation',
                '000010': 'Server-class computer',
                '000011': 'Laptop',
                '000100': 'Handheld PC/PDA (clam shell)',
                '000101': 'Palm sized PC/PDA',
                '000110': 'Wearable computer (Watch sized)',
                '000111': 'Tablet',
            },)),
        2: ('Phone', (
            {
                '000000': 'Uncategorized',
                '000001': 'Cellular',
                '000010': 'Cordless',
                '000011': 'Smartphone',
                '000100': 'Wired modem or voice gateway',
                '000101': 'Common ISDN Access',
            },)),
        3: ('LAN / Network Access point', (
            {
                '000': 'Fully available',
                '001': '1 - 17% utilized',
                '010': '17 - 33% utilized',
                '011': '33 - 50% utilized',
                '100': '50 - 67% utilized',
                '101': '67 - 83% utilized',
                '110': '83 - 99% utilized',
                '111': 'No service available',
            },)),
        4: ('Audio / Video', (
            {
                '000000': 'Uncategorized',
                '000001': 'Wearable Headset Device',
                '000010': 'Hands-free Device',
                '000011': '(Reserved)',
                '000100': 'Microphone',
                '000101': 'Loudspeaker',
                '000110': 'Headphones',
                '000111': 'Portable Audio',
                '001000': 'Car audio',
                '001001': 'Set-top box',
                '001010': 'HiFi Audio Device',
                '001011': 'VCR',
                '001100': 'Video Camera',
                '001101': 'Camcorder',
                '001110': 'Video Monitor',
                '001111': 'Video Display and Loudspeaker',
                '010000': 'Video Conferencing',
                '010001': '(Reserved)',
                '010010': 'Gaming/Toy',
            },)),
        5: ('Peripheral', (
            {
                '00': '',
                '01': 'Keyboard',
                '10': 'Pointing device',
                '11': 'Combo keyboard/pointing device',
            }, {
                '0000': 'Uncategorized device',
                '0001': 'Joystick',
                '0010': 'Gamepad',
                '0011': 'Remote control',
                '0100': 'Sensing device',
                '0101': 'Digitizer tablet',
                '0110': 'Card Reader',
                '0111': 'Digital Pen',
                '1000': 'Handheld scanner for ID codes',
                '1001': 'Handheld gestural input device',
            },)),
        6: ('Imaging', (
            {
                '0001': "Display",
                '0010': "Camera",
                '0011': "Display/Camera",
                '0100': "Scanner",
                '0101': "Display/Scanner",
                '0110': "Camera/Scanner",
                '0111': "Display/Camera/Scanner",
                '1000': "Printer",
                '1001': "Display/Printer",
                '1010': "Camera/Printer",
                '1011': "Display/Camera/Printer",
                '1100': "Scanner/Printer",
                '1101': "Display/Scanner/Printer",
                '1110': "Camera/Scanner/Printer",
                '1111': "Display/Camera/Scanner/Printer",
            },)),
        7: ('Wearable', (
            {
                '000001': 'Wristwatch',
                '000010': 'Pager',
                '000011': 'Jacket',
                '000100': 'Helmet',
                '000101': 'Glasses',
            },)),
        8: ("Toy", (
            {
                '000001': "Robot",
                '000010': "Vehicle",
                '000011': "Doll / Action figure",
                '000100': "Controller",
                '000101': "Game",
            },)),
        9: ("Health", (
            {
                '000000': "Undefined",
                '000001': "Blood Pressure Monitor",
                '000010': "Thermometer",
                '000011': "Weighing Scale",
                '000100': "Glucose Meter",
                '000101': "Pulse Oximeter",
                '000110': "Heart/Pulse Rate Monitor",
                '000111': "Health Data Display",
                '001000': "Step Counter",
                '001001': "Body Composition Analyzer",
                '001010': "Peak Flow Monitor",
                '001011': "Medication Monitor",
                '001100': "Knee Prosthesis",
                '001101': "Ankle Prosthesis",
                '001110': "Generic Health Manager",
                '001111': "Personal Mobility Device",
            },)),
        31: ('Uncategorized', ()),
    }

    def __init__(self, cod):
        """Initialize the object with a given class-of-device.

        The class-of-device argument *cod* may be either a 24-bit
        integer in the format described by Bluetooth Assigned Numbers,
        or another instance of DeviceClass from which the
        class-of-device bits are copied.

        """
        self.cod = cod.cod if isinstance(cod, DeviceClass) else cod

    def __eq__(self, other):
        """Return True if self equals other, False if not.

        """
        return self.cod == other.cod

    def __str__(self):
        """Return a formatted representation suitable for printing.

        """
        if self.cod & 0b11 == 0:
            if self.major_service_class:
                major_service_class = ' and '.join(self.major_service_class)
            else:
                major_service_class = 'Unspecified'
            s = '{self.major_device_class} - {self.minor_device_class} - {0}'
            return 'Device Class ' + s.format(major_service_class, self=self)
        else:
            return 'Device Class {:024b}b'.format(self.cod)

    def __repr__(self):
        """Return a formal representation suitable for evaluation.

        """
        return "{}.{}(0x{:06X})".format(
            self.__module__, self.__class__.__name__, self.cod)

    @property
    def major_service_class(self):
        """A tuple of text strings for the device's major service classes.

        Note that this will be an empty tuple if no service class bits
        are set and None if the class-of-device is not "format #1".

        """
        if self.cod & 0b11 == 0:
            bits = [i-13 for i in range(13, 24) if self.cod >> i & 1]
            return tuple([self.service_class_name[i] for i in bits])

    @property
    def major_device_class(self):
        """A text string for the major device class.

        Note that this will be None if the 24-bit class-of-device
        format is not "format #1".

        """
        if self.cod & 0b11 == 0:
            major = self.cod >> 8 & 0b11111
            if major in self.device_class:
                return self.device_class[major][0]
            else:
                return 'Reserved {:05b}b'.format(major)

    @property
    def minor_device_class(self):
        """A text string for the minor device class.

        Note that this will be None if the 24-bit class-of-device
        format is not "format #1".

        """
        if self.cod & 0b11 == 0:
            text = []
            major = self.cod >> 8 & 0b11111
            minor = '{:06b}'.format(self.cod >> 2 & 0b111111)
            if major in self.device_class:
                for mapping in self.device_class[major][1]:
                    bits = minor[:len(next(iter(mapping)))]
                    text.append(mapping.get(bits, 'Reserved {}b'.format(bits)))
                    minor = minor[len(bits):]
                return ' and '.join(filter(None, text))
            else:
                return 'Undefined {}b'.format(minor)

    def encode(self):
        """Return the encoded representation suitable for transmission.

        In absence of encoding errors the return value is exactly
        three bytes.

        """
        if 0 <= self.cod <= 0xFFFFFF:
            return struct.pack('<I', self.cod)[0:3]
        else:
            errstr = "can't encode {!r} into class of device octets"
            raise encode_error(self, errstr, self.cod)

    @classmethod
    def decode(cls, octets):
        """Return a class instance initialized from octets.

        The octets argument must be a byte sequence of length 3 that
        is assumed to be the 24-bit class-of-device integer (in little
        endian).

        """
        if len(octets) == 3:
            return cls(struct.unpack('I', octets + b'\0')[0])
        else:
            errstr = "can't decode class of device from {} octets"
            raise decode_error(cls, errstr, len(octets))


class ServiceClass(object):
    """Encapsulation of a Bluetooth Service Class UUID.

    Bluetooth Service Class UUIDs are transmitted in three different
    lengths: 128-bit regular UUID and 32-bit or 16-bit shortened
    UUID. A shortened UUID implies the remaining parts to be equal to
    the well-known Blutooth base UUID. This class can decode and
    encode either form and universially represents them as a 128-bit
    regular UUID. It also knows the names of all registered Bluetooth
    Service Class UUIDs.

    Values and names of Bluetooth Service Class UUIDs are defined at
    https://www.bluetooth.com/specifications/assigned-numbers/service-discovery

    """
    bluetooth_base_uuid = UUID('00000000-0000-1000-8000-00805F9B34FB')
    bluetooth_uuid_list = {
        0x00001000: "Service Discovery Server",
        0x00001001: "Browse Group Descriptor",
        0x00001101: "Serial Port",
        0x00001102: "LAN Access Using PPP",
        0x00001103: "Dialup Networking",
        0x00001104: "IrMC Sync",
        0x00001105: "OBEX Object Push",
        0x00001106: "OBEX File Transfer",
        0x00001107: "IrMC Sync Command",
        0x00001108: "Headset",
        0x00001109: "Cordless Telephony",
        0x0000110a: "Audio Source",
        0x0000110b: "Audio Sink",
        0x0000110c: "A/V Remote Control Target",
        0x0000110d: "Advanced Audio Distribution",
        0x0000110e: "A/V Remote Control",
        0x0000110f: "A/V Remote Control Controller",
        0x00001110: "Intercom",
        0x00001111: "Fax",
        0x00001112: "Headset - Audio Gateway (AG)",
        0x00001113: "WAP",
        0x00001114: "WAP Client",
        0x00001115: "PANU",
        0x00001116: "NAP",
        0x00001117: "GN",
        0x00001118: "Direct Printing",
        0x00001119: "Reference Printing",
        0x0000111a: "Basic Imaging Profile",
        0x0000111b: "Imaging Responder",
        0x0000111c: "Imaging Automatic Archive",
        0x0000111d: "Imaging Referenced Objects",
        0x0000111e: "Handsfree",
        0x0000111f: "Handsfree Audio Gateway",
        0x00001120: "Direct Printing Reference",
        0x00001121: "Reflected UI",
        0x00001122: "Basic Printing",
        0x00001123: "Printing Status",
        0x00001124: "Human Interface Device",
        0x00001125: "Hardcopy Cable Replacement",
        0x00001126: "HCR Print",
        0x00001127: "HCR Scan",
        0x00001128: "Common ISDN Access",
        0x0000112d: "SIM Access",
        0x0000112e: "Phonebook Access - PCE",
        0x0000112f: "Phonebook Access - PSE",
        0x00001130: "Phonebook Access",
        0x00001131: "Headset - HS",
        0x00001132: "Message Access Server",
        0x00001133: "Message Notification Server",
        0x00001134: "Message Access Profile",
        0x00001135: "GNSS",
        0x00001136: "GNSS Server",
        0x00001200: "PnP Information",
        0x00001201: "Generic Networking",
        0x00001202: "Generic File Transfer",
        0x00001203: "Generic Audio",
        0x00001204: "Generic Telephony",
        0x00001205: "UPNP Service",
        0x00001206: "UPNP IP Service",
        0x00001300: "ESDP UPNP IP PAN",
        0x00001301: "ESDP UPNP IP LAP",
        0x00001302: "ESDP UPNP L2CAP",
        0x00001303: "Video Source",
        0x00001304: "Video Sink",
        0x00001305: "Video Distribution",
        0x00001400: "HDP",
        0x00001401: "HDP Source",
        0x00001402: "HDP Sink",
    }

    def __init__(self, *args, **kwargs):
        """Initialize the object with some form of UUID.

        If the first positional argument is an integer then it is used
        to construct a UUID from the Bluetooth base UUID with the
        first 32 bits substituted by the argument.

        If the first positional argument is a Bluetooth Service Class
        UUID name then the UUID is constructed from the associated
        integer value combined with the Bluetooth base UUID.

        If the first positional argument is a uuid.UUID object then it
        is copied.

        In all other cases the positional and keyword arguments are
        used to intialize a uuid.UUID object.

        """
        if args and isinstance(args[0], int):
            fields = (args[0],) + self.bluetooth_base_uuid.fields[1:]
            self._uuid = UUID(fields=fields)
        elif args and args[0] in self.bluetooth_uuid_list.values():
            index = list(self.bluetooth_uuid_list.values()).index(args[0])
            value = list(self.bluetooth_uuid_list.keys())[index]
            fields = (value,) + self.bluetooth_base_uuid.fields[1:]
            self._uuid = UUID(fields=fields)
        elif args and isinstance(args[0], UUID):
            self._uuid = UUID(str(args[0]))
        else:
            self._uuid = UUID(*args, **kwargs)

    def __eq__(self, other):
        """Return True if self equals other, False if not.

        """
        return self._uuid == other._uuid

    def __str__(self):
        """Return a formatted representation suitable for printing.

        """
        return "Service Class {}".format(self.name)

    def __repr__(self):
        """Return a formal representation suitable for evaluation.

        """
        return "{}.{}({!r})".format(
            self.__module__, self.__class__.__name__, str(self.uuid))

    @property
    def uuid(self):
        """A reference to the internally hold uuid.UUID object.

        """
        return self._uuid

    @property
    def name(self):
        """The Bluetooth Service Class UUID name or UUID string.

        """
        if self.uuid.fields[1:] == self.bluetooth_base_uuid.fields[1:]:
            if self.uuid.fields[0] in self.bluetooth_uuid_list:
                return self.bluetooth_uuid_list[self.uuid.fields[0]]
        return str(self.uuid)

    @classmethod
    def get_uuid_names(cls):
        """Returns a tuple of all known Bluetooth Service Class UUID names.

        """
        return tuple(cls.bluetooth_uuid_list.values())

    def encode(self):
        """Return the encoded representation suitable for transmission.

        This is either 2, 4 or 16 bytes depending on whether or not it
        is a shortened Bluetooth Service Class UUID.

        """
        if self.uuid.fields[1:] == self.bluetooth_base_uuid.fields[1:]:
            fmt = '<H' if self.uuid.fields[0] < 0x10000 else '<I'
            return struct.pack(fmt, self.uuid.fields[0])
        return self.uuid.bytes_le

    @classmethod
    def decode(cls, octets):
        """Return a class instance initialized from octets.

        The octets argument must be a byte sequence of either length 2
        for a uuid-16, length 4 for a uuid-32, or length 16 for a
        uuid-128.

        """
        if len(octets) in (2, 4):
            value = struct.unpack('<H' if len(octets) == 2 else '<I', octets)
            return cls(UUID(fields=value+cls.bluetooth_base_uuid.fields[1:]))
        elif len(octets) == 16:
            return cls(UUID(bytes_le=octets))
        else:
            errstr = "can't decode service class uuid from {} octets"
            raise decode_error(cls, errstr, len(octets))


class BluetoothRecord(GlobalRecord):
    """Base class that implements dict-like Bluetooth EIR/AD data access.

    """
    _attribute_name_mapping = {
        'Flags': 0x01,
        'Incomplete List of 16-bit Service Class UUIDs': 0x02,
        'Complete List of 16-bit Service Class UUIDs': 0x03,
        'Incomplete List of 32-bit Service Class UUIDs': 0x04,
        'Complete List of 32-bit Service Class UUIDs': 0x05,
        'Incomplete List of 128-bit Service Class UUIDs': 0x06,
        'Complete List of 128-bit Service Class UUIDs': 0x07,
        'Shortened Local Name': 0x08,
        'Complete Local Name': 0x09,
        'Class of Device': 0x0D,
        'Simple Pairing Hash C': 0x0E,
        'Simple Pairing Hash C-192': 0x0E,
        'Simple Pairing Randomizer R': 0x0F,
        'Simple Pairing Randomizer R-192': 0x0F,
        'Security Manager TK Value': 0x10,
        'Security Manager Out of Band Flags': 0x11,
        'Appearance': 0x19,
        'LE Secure Connections Confirmation Value': 0x22,
        'LE Secure Connections Random Value': 0x23,
        'LE Bluetooth Device Address': 0x1B,
        'LE Role': 0x1C,
        'Simple Pairing Hash C-256': 0x1D,
        'Simple Pairing Randomizer R-256': 0x1E,
        'Manufacturer Specific Data': 0xFF,
    }

    @property
    def attribute_names(self):
        """A tuple of all attribute names that may be used as keys.

        """
        return tuple(self._attribute_name_mapping.keys())

    def _map_key(self, key):
        if isinstance(key, int):
            return key
        elif key in self._attribute_name_mapping:
            return self._attribute_name_mapping[key]
        else:
            errstr = "unknown attribute name '{name}'"
            raise ValueError(errstr.format(name=key))

    def __init__(self, *args):
        self._attributes = dict()
        for _type, _value in args:
            self[_type] = _value

    def __getitem__(self, key):
        return self._attributes[self._map_key(key)]

    def __setitem__(self, key, value):
        assert isinstance(value, (bytes, bytearray))
        self._attributes[self._map_key(key)] = value

    def __delitem__(self, key):
        del self._attributes[self._map_key(key)]

    def __contains__(self, key):
        return self._map_key(key) in self._attributes

    def __iter__(self):
        return self._attributes.__iter__()

    def get(self, key, default=None):
        return self._attributes.get(self._map_key(key), default)

    def setdefault(self, key, default=None):
        return self._attributes.setdefault(self._map_key(key), default)

    def keys(self):
        return self._attributes.keys()

    def values(self):
        return self._attributes.values()

    def items(self):
        return self._attributes.items()

    def __format__(self, format_spec):
        if format_spec == 'args':
            afmt = "(0x{:02X}, {!r})"
            args = [afmt.format(k, v) for k, v in self.items()]
            return ', '.join(args)
        elif format_spec == 'data':
            keys = ["0x{:02X}".format(k) for k in sorted(self.keys())]
            return "Attributes {}".format(' '.join(keys))
        else:
            return super(BluetoothRecord, self).__format__(format_spec)

    def _get_device_name(self):
        return (
            self.get('Complete Local Name') or
            self.get('Shortened Local Name', b'')
        ).decode('utf-8')

    def _set_device_name(self, value):
        self['Complete Local Name'] = value.encode('utf-8')
        if 'Shortened Local Name' in self:
            del self['Shortened Local Name']

    def _get_integer_value(self, bluetooth_attribute_name):
        octets = self.get(bluetooth_attribute_name)
        if octets is not None:
            return (int((octets[::-1]).encode('hex'), base=16) if _PY2
                    else int.from_bytes(octets, byteorder='little'))

    def _set_integer_value(self, bluetooth_attribute_name, value):
        octets = ('{:032x}'.format(value).decode('hex')[::-1] if _PY2
                  else value.to_bytes(16, byteorder='little'))
        self[bluetooth_attribute_name] = octets


class BluetoothEasyPairingRecord(BluetoothRecord):
    """Decoder/Encoder for Bluetooth Easy Pairing out-of-band data.

    """
    _type = 'application/vnd.bluetooth.ep.oob'

    def __init__(self, device_address, *eir):
        """Initialize the record with device_address and optionally more.

        Bluetooth Easy Pairing out-of-band data must have BD_ADDR and
        may have zero or more EIR data structures. The first
        positional argument device_address argument must be a string
        of 6 hexadecimal octets separated by colon or dash, or a
        DeviceAddress instance. Any more positional arguments must be
        (tag, value) tuples of EIR data structures, where tag is an
        integer or EIR name and value is a byte sequence in
        transmission order.

        """
        super(BluetoothEasyPairingRecord, self).__init__(*eir)
        self.device_address = device_address

    @property
    def device_address(self):
        """Get or set the Bluetooth Device Address.

        The internal BD_ADDR is returned as a DeviceAddress object. A
        set value may be either another DeviceAddress or a string of 6
        hexadecimal octets separated by colon or dash.

        """
        return DeviceAddress.decode(self.bd_addr)

    @device_address.setter
    def device_address(self, value):
        if not isinstance(value, DeviceAddress):
            value = DeviceAddress(value)
        self.bd_addr = value.encode('EP')

    @property
    def device_name(self):
        """Get or set the Bluetooth Local Device Name.

        The Local Name, if configured on the Bluetooth device, is the
        name that may be displayed to the device user as part of the
        UI involving operations with Bluetooth devices. It may be
        encoded as either 'Complete Local name' or 'Shortened Local
        Name' EIR data type.

        This attribute provides the Local Name as a text string. The
        value returned is the 'Complete Local Name' or 'Shortened
        Local Name' evaluated in that order. None is returned if
        neither EIR data type exists.

        A device name assigned to this attribute is always stored as
        the 'Complete Local Name' and removes a 'Shortened Local
        Name' EIR data type if formerly present.

        """
        return self._get_device_name()

    @device_name.setter
    @convert('value_to_unicode')
    def device_name(self, value):
        self._set_device_name(value)

    @property
    def device_class(self):
        """Get or set the Bluetooth Class of Device.

        This attribute provides the decoded 'Class of Device' as a
        DeviceClass object. If 'Class of Device' is not present it
        reads as None.

        The attribute may be set by assigning either a 24-bit integer
        according to the Bluetooth Class of Device format or another
        DeviceClass object.

        """
        try:
            return DeviceClass.decode(self['Class of Device'])
        except KeyError:
            return DeviceClass(0)

    @device_class.setter
    def device_class(self, value):
        if not isinstance(value, DeviceClass):
            value = DeviceClass(value)
        self['Class of Device'] = value.encode()

    @property
    def service_class_list(self):
        """List of all present Bluetooth Service Class UUIDs.

        The list contains all Bluetooth Service Class UUIDs found in
        the the complete and incomplete variants of 16, 32 and 128-bit
        UUID data structures.

        """
        uuid_list = []
        octets = self.get(0x02, b'') + self.get(0x03, b'')
        for offset in range(0, len(octets), 2):
            uuid_list.append(ServiceClass.decode(octets[offset:offset+2]))
        octets = self.get(0x04, b'') + self.get(0x05, b'')
        for offset in range(0, len(octets), 4):
            uuid_list.append(ServiceClass.decode(octets[offset:offset+4]))
        octets = self.get(0x06, b'') + self.get(0x07, b'')
        for offset in range(0, len(octets), 16):
            uuid_list.append(ServiceClass.decode(octets[offset:offset+16]))
        return uuid_list

    def add_service_class(self, service_class, complete=False):
        """Add a service class identifier.

        The service_class argument must be either a ServiceClass
        object, a UUID string, or an integer value for a 16 or 32 bit
        Bluetooth Service Class UUID. The second argument determines
        whether the complete or incomplete service class data type is
        populated. Note that existing Service Class UUIDs (of the same
        size uuid-16, uuid-32, or uuid-128) are moved to complete or
        incomplete depending on the flag.

        """
        if not isinstance(service_class, ServiceClass):
            service_class = ServiceClass(service_class)

        octets = service_class.encode()
        if len(octets) == 2:
            octets = self.get(0x02, b'') + self.get(0x03, b'') + octets
            self[0x03 if complete else 0x02] = octets
            try:
                del self[0x02 if complete else 0x03]
            except KeyError:
                pass
        elif len(octets) == 4:
            octets = self.get(0x04, b'') + self.get(0x05, b'') + octets
            self[0x05 if complete else 0x04] = octets
            try:
                del self[0x04 if complete else 0x05]
            except KeyError:
                pass
        else:
            octets = self.get(0x06, b'') + self.get(0x07, b'') + octets
            self[0x07 if complete else 0x06] = octets
            try:
                del self[0x06 if complete else 0x07]
            except KeyError:
                pass

    @property
    def simple_pairing_hash_192(self):
        """Get or set the Simple Pairing Hash C-192.

        This attribute returns either the 128-bit integer converted
        from the 16-octet 'Simple Pairing Hash C-192' EIR value or
        None if the EIR data type is not present. When set, it stores
        a 128-bit integer as the 16-octet value of the 'Simple Pairing
        Hash C-192' EIR data type.

        """
        return self._get_integer_value('Simple Pairing Hash C-192')

    @simple_pairing_hash_192.setter
    def simple_pairing_hash_192(self, value):
        self._set_integer_value('Simple Pairing Hash C-192', value)

    @property
    def simple_pairing_randomizer_192(self):
        """Get or set the Simple Pairing Randomizer R-192.

        This attribute returns either the 128-bit integer converted
        from the 16-octet 'Simple Pairing Randomizer R-192' EIR value
        or None if the EIR data type is not present. When set, it
        stores a 128-bit integer as the 16-octet value of the 'Simple
        Pairing Randomizer R-192' EIR data type.

        """
        return self._get_integer_value('Simple Pairing Randomizer R-192')

    @simple_pairing_randomizer_192.setter
    def simple_pairing_randomizer_192(self, value):
        self._set_integer_value('Simple Pairing Randomizer R-192', value)

    @property
    def simple_pairing_hash_256(self):
        """Get or set the Simple Pairing Hash C-256.

        This attribute returns either the 128-bit integer converted
        from the 16-octet 'Simple Pairing Hash C-256' EIR value or
        None if the EIR data type is not present. When set, it stores
        a 128-bit integer as the 16-octet value of the 'Simple Pairing
        Hash C-256' EIR data type.

        """
        return self._get_integer_value('Simple Pairing Hash C-256')

    @simple_pairing_hash_256.setter
    def simple_pairing_hash_256(self, value):
        self._set_integer_value('Simple Pairing Hash C-256', value)

    @property
    def simple_pairing_randomizer_256(self):
        """Get or set the Simple Pairing Randomizer R-256.

        This attribute returns either the 128-bit integer converted
        from the 16-octet 'Simple Pairing Randomizer R-256' EIR value
        or None if the EIR data type is not present. When set, it
        stores a 128-bit integer as the 16-octet value of the 'Simple
        Pairing Randomizer R-256' EIR data type.

        """
        return self._get_integer_value('Simple Pairing Randomizer R-256')

    @simple_pairing_randomizer_256.setter
    def simple_pairing_randomizer_256(self, value):
        self._set_integer_value('Simple Pairing Randomizer R-256', value)

    # BR/EDR out-of-band payload is OOB data length (2 octets), BDADDR
    # (6 octets), and a sequence of Extended Inquiry Response (EIR)
    # structures [CORE SPEC Vol 3 Part C Sect 5.2.2.7]. An EIR
    # structure is a single octet Length and Length octets Data
    # part. The Data part contains the EIR Type (n octets) and EIR
    # Data (Length - n octets). All defined EIR Types are single
    # octets.

    def _encode_payload(self):
        octets = [self.bd_addr]
        for attr_type, attr_data in self.items():
            data = self._encode_struct('B*', attr_type, attr_data)
            octets.append(self._encode_struct('B+', data))

        octets.insert(0, self._encode_struct('<H', 2 + sum(map(len, octets))))
        return b''.join(octets)

    _decode_min_payload_length = 8  # 2 octets OOB length and 6 octets BDADDR

    @classmethod
    def _decode_payload(cls, octets, errors):
        ooblen = cls._decode_struct('<H', octets)
        bdaddr = DeviceAddress.decode(octets[2:8])

        if len(octets) < ooblen:
            errstr = "oob data length {} exceeds payload size {}"
            raise cls._decode_error(errstr, ooblen, len(octets))

        if len(octets) > ooblen and errors == "strict":
            errstr = "payload size {} exceeds oob data length {}"
            raise cls._decode_error(errstr, len(octets), ooblen)

        offset, attrs = 8, []
        while offset < ooblen:
            data = cls._decode_struct('B+', octets, offset)
            if len(data) > 0:
                attr_type, attr_data = cls._decode_struct('B*', data)
                attrs.append((attr_type, attr_data))
                offset += len(attr_data) + 2
            else:
                offset += 1

        return cls(bdaddr, *attrs)


class BluetoothLowEnergyRecord(BluetoothRecord):
    """Decoder/Encoder for Bluetooth Low Energy out-of-band pairing data.

    """
    _type = 'application/vnd.bluetooth.le.oob'

    @property
    def device_address(self):
        """Get or set the LE Bluetooth Device Address.

        The LE Bluetooth Device Address data value consists of 7
        octets made up from the 48 bit address that is used for
        Bluetooth pairing over the LE transport and a flags octet that
        defines the address type. The address type distinguishes a
        Public Device Address versus a Random Device Address. A Random
        Device Address sent with BLE out-of-band data should be used
        on the LE transport for at least ten minutes after the NFC
        data exchange.

        This attribute returns a DeviceAddress object or None,
        depending on whether the 'LE Bluetooth Device Address' AD type
        is present (under rare circumstances or just by failure it may
        not be). It may be set from either another DeviceAddress
        object, the tuple of address and address type strings, or just
        an address string for an implicitly public address.

        """
        try:
            octets = self['LE Bluetooth Device Address']
            return DeviceAddress.decode(octets)
        except KeyError:
            return None

    @device_address.setter
    def device_address(self, value):
        if not isinstance(value, DeviceAddress):
            if isinstance(value, (tuple, list)):
                value = DeviceAddress(*value)
            else:
                value = DeviceAddress(value)
        self['LE Bluetooth Device Address'] = value.encode('LE')

    @property
    def device_name(self):
        """Get or set the Bluetooth Local Device Name.

        The Local Name, if configured on the Bluetooth device, is the
        name that may be displayed to the device user as part of the
        UI involving operations with Bluetooth devices. It may be
        encoded as either 'Complete Local name' or 'Shortened Local
        Name' AD type.

        This attribute provides the Local Name as a text string. The
        value returned is the 'Complete Local Name' or 'Shortened
        Local Name' evaluated in that order. None is returned if
        neither AD type exists.

        A device name assigned to this attribute is always stored as
        the 'Complete Local Name' and removes a 'Shortened Local
        Name' AD type if formerly present.

        """
        return self._get_device_name()

    @device_name.setter
    @convert('value_to_unicode')
    def device_name(self, value):
        self._set_device_name(value)

    # Bluetooth LE out-of-band payload is coded as Advertising or Scan
    # Response Data (AD) format (only significant part). The format
    # consists of a sequence of AD structures made of a single octet
    # Length and Length octets Data part. The Data part contains the
    # AD Type (n octets) and AD Data (Length - n octets). All defined
    # AD Types are single octets.

    def _encode_payload(self):
        octets = []
        for attr_type, attr_data in self.items():
            data = self._encode_struct('B*', attr_type, attr_data)
            octets.append(self._encode_struct('B+', data))
        return b''.join(octets)

    @classmethod
    def _decode_payload(cls, octets, errors):
        offset, attrs = 0, []
        while offset < len(octets):
            data = cls._decode_struct('B+', octets, offset)
            if len(data) > 0:
                attr_type, attr_data = cls._decode_struct('B*', data)
                attrs.append((attr_type, attr_data))
                offset += len(attr_data) + 2
            else:
                offset += 1

        return cls(*attrs)

    _le_role_table = (
        "Peripheral",
        "Central",
        "Peripheral/Central",
        "Central/Peripheral",
    )

    @property
    def role_capabilities(self):
        """Get or set the LE Role capabilities.

        Role capabilties are string values describing one of the four
        defined roles 'Peripheral', 'Central', 'Peripheral/Central'
        (Peripheral Role preferred for connection establishment), or
        'Central/Peripheral' (Central is preferred for connection
        establishment).

        """
        if 'LE Role' in self:
            value = self._decode_struct('B', self['LE Role'])
            if value < len(self._le_role_table):
                return self._le_role_table[value]
            else:
                return "Reserved 0x{:02X}".format(value)

    @role_capabilities.setter
    def role_capabilities(self, value):
        if value in self._le_role_table:
            octet = self._le_role_table.index(value)
            self['LE Role'] = self._encode_struct('B', octet)
        else:
            errmsg = "undefined role capability {!r}"
            raise self._value_error(errmsg, value)

    _appearance_map = {
        0x0000: "Unknown",
        0x0040: "Phone",
        0x0080: "Computer",
        0x00c0: "Watch",
        0x00c1: "Watch: Sports Watch",
        0x0100: "Clock",
        0x0140: "Display",
        0x0180: "Remote Control",
        0x01c0: "Eye-glasses",
        0x0200: "Tag",
        0x0240: "Keyring",
        0x0280: "Media Player",
        0x02c0: "Barcode Scanner",
        0x0300: "Thermometer",
        0x0301: "Thermometer: Ear",
        0x0340: "Heart Rate Sensor",
        0x0341: "Heart Rate Sensor: Belt",
        0x0380: "Blood Pressure",
        0x0381: "Blood Pressure: Arm",
        0x0382: "Blood Pressure: Wrist",
        0x03c0: "Human Interface Device",
        0x03c1: "Human Interface Device: Keyboard",
        0x03c2: "Human Interface Device: Mouse",
        0x03c3: "Human Interface Device: Joystick",
        0x03c4: "Human Interface Device: Gamepad",
        0x03c5: "Human Interface Device: Digitizer Tablet",
        0x03c6: "Human Interface Device: Card Reader",
        0x03c7: "Human Interface Device: Digital Pen",
        0x03c8: "Human Interface Device: Barcode Scanner",
        0x0400: "Glucose Meter",
        0x0440: "Running Walking Sensor",
        0x0441: "Running Walking Sensor: In-Shoe",
        0x0442: "Running Walking Sensor: On-Shoe",
        0x0443: "Running Walking Sensor: On-Hip",
        0x0480: "Cycling",
        0x0481: "Cycling: Cycling Computer",
        0x0482: "Cycling: Speed Sensor",
        0x0483: "Cycling: Cadence Sensor",
        0x0484: "Cycling: Power Sensor",
        0x0485: "Cycling: Speed and Cadence Sensor",
        0x0c40: "Pulse Oximeter",
        0x0c41: "Pulse Oximeter: Fingertip",
        0x0c42: "Pulse Oximeter: Wrist Worn",
        0x0c80: "Weight Scale",
        0x1440: "Outdoor Sports",
        0x1441: "Outdoor Sports: Location Display Device",
        0x1442: "Outdoor Sports: Location and Navigation Display Device",
        0x1443: "Outdoor Sports: Location Pod",
        0x1444: "Outdoor Sports: Location and Navigation Pod",
    }

    @property
    def appearance_strings(self):
        """List of all known appearance strings.

        """
        return [self._appearance_map[k] for k in sorted(self._appearance_map)]

    @property
    def appearance(self):
        """Get or set the representation of the external appearance of the
        device, used by the discovering device to represent an icon,
        string, or similar to the user. The returned value is a tuple
        with the numeric value and a textual description, or None if
        the 'Appearance' AD type is not found. The appearance
        attribute accepts either a numeric value or a description
        string.

        Appearance strings consist of a generic category and an
        optional subtype. If a subtype is present it follows the generic
        category text after a colon.

        """
        if 'Appearance' in self:
            value = self._decode_struct('<H', self['Appearance'])
            return (value, self._appearance_map.get(value, ""))

    @appearance.setter
    def appearance(self, value):
        if isinstance(value, str):
            try:
                items = self._appearance_map.items()
                value = [v for v, s in items if s == value][0]
            except IndexError:
                errmsg = "unknown appearance value string {!r}"
                raise self._value_error(errmsg, value)

        self['Appearance'] = self._encode_struct('<H', value)

    _flags = (
        "LE Limited Discoverable Mode",
        "LE General Discoverable Mode",
        "BR/EDR Not Supported",
        "Simultaneous LE and BR/EDR to Same Device Capable (Controller)",
        "Simultaneous LE and BR/EDR to Same Device Capable (Host)",
    )

    @property
    def flags(self):
        """Get or set the Flags bitmap.

        The 'Flags' AD type contains information on which discoverable
        mode to use and BR/EDR support and capability. This attribute
        returns the numerical flags value and descriptions for raised
        bits as an N-tuple.

        """
        if 'Flags' in self:
            value = self._decode_struct('B', self['Flags'])
            names = [s for i, s in enumerate(self._flags) if value >> i & 1]
            return tuple([value] + names)

    @flags.setter
    def flags(self, value):
        if isinstance(value, (tuple, list)):
            value = sum(1 << self._flags.index(s) for s in value)
        self['Flags'] = self._encode_struct('B', value)

    @property
    def security_manager_tk_value(self):
        """Get or set the Security Manager TK Value.

        The Security Manager TK Value is used by the LE Security
        Manager if the OOB association model with LE Legacy pairing is
        used. Reading this attribute returns an unsigned integer
        converted from the 16 byte 'Security Manager TK Value' AD type
        octets, or None if the AD type is not found. An unsigned
        integer assigned to this attribute is written as the 16 byte
        'Security Manager TK Value' AD type after conversion.

        """
        return self._get_integer_value('Security Manager TK Value')

    @security_manager_tk_value.setter
    def security_manager_tk_value(self, value):
        self._set_integer_value('Security Manager TK Value', value)

    @property
    def secure_connections_confirmation_value(self):
        """Get or set the LE Secure Connections Confirmation Value.

        The LE Secure Connections Confirmation Value is used by the LE
        Security Manager if the OOB association model with LE Secure
        Connections pairing is used. Reading this attribute returns an
        unsigned integer converted from the 16 byte 'LE Secure
        Connections Confirmation Value' AD type octets, or None if the
        AD type is not found. An unsigned integer assigned to this
        attribute is written as the 16 byte 'LE Secure Connections
        Confirmation Value' AD type after conversion.

        """
        attribute_name = 'LE Secure Connections Confirmation Value'
        return self._get_integer_value(attribute_name)

    @secure_connections_confirmation_value.setter
    def secure_connections_confirmation_value(self, value):
        attribute_name = 'LE Secure Connections Confirmation Value'
        self._set_integer_value(attribute_name, value)

    @property
    def secure_connections_random_value(self):
        """Get the LE Secure Connections Random Value.

        The LE Secure Connections Random Value is used by the LE
        Security Manager if the OOB association model with LE Secure
        Connections pairing is used. Reading this attribute returns an
        unsigned integer converted from the 16 byte 'LE Secure
        Connections Random Value' AD type octets, or None if the AD
        type is not found. An unsigned integer assigned to this
        attribute is written as the 16 byte 'LE Secure Connections
        Random Value' AD type after conversion.

        """
        return self._get_integer_value('LE Secure Connections Random Value')

    @secure_connections_random_value.setter
    def secure_connections_random_value(self, value):
        self._set_integer_value('LE Secure Connections Random Value', value)


Record.register_type(BluetoothEasyPairingRecord)
Record.register_type(BluetoothLowEnergyRecord)

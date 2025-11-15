# -*- coding: latin-1 -*-
#
# wif.py - parse or generate Wi-Fi Simple Configuration data
#
from __future__ import absolute_import, division
from .record import Record, GlobalRecord, hexlify, _PY2
from .record import DecodeError, EncodeError
from collections import namedtuple
from functools import reduce
import operator
import uuid

VersionTuple = namedtuple('Version', 'major, minor')


class AttributeBase(object):
    def __repr__(self):
        s = "{r.__class__.__module__}.{r.__class__.__name__}({r:args})"
        return s.format(r=self)

    def __str__(self):
        return "{r._str} {r:data}".format(r=self)

    @classmethod
    def _decode_error(cls, fmt, *args, **kwargs):
        clname = cls.__module__ + "." + cls.__name__
        return DecodeError(clname + " " + fmt.format(*args, **kwargs))

    @classmethod
    def _encode_error(cls, fmt, *args, **kwargs):
        clname = cls.__module__ + "." + cls.__name__
        return EncodeError(clname + " " + fmt.format(*args, **kwargs))


class AttributeContainer(AttributeBase):
    _attribute_name_mapping = NotImplemented

    @property
    def attribute_names(self):
        return self._attribute_name_mapping.keys()

    def _map_key(self, key):
        if isinstance(key, int):
            return key
        elif key in self._attribute_name_mapping:
            return self._attribute_name_mapping[key]._key
        else:
            errstr = "unknown attribute name '{name}'"
            raise ValueError(errstr.format(name=key))

    def __init__(self, *args):
        self._attributes = dict()
        for _type, _value in args:
            self.setdefault(_type, []).append(_value)

    def __getitem__(self, key):
        return self._attributes[self._map_key(key)]

    def __setitem__(self, key, value):
        self._attributes[self._map_key(key)] = value

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
            args = [afmt.format(k, v) for k in self for v in self[k]]
            return ', '.join(args)
        elif format_spec == 'data':
            keys = ["0x{:02X}".format(k) for k in sorted(self)
                    for v in self[k]]
            return "Attributes {}".format(' '.join(keys))
        else:
            return super(AttributeContainer, self).__format__(format_spec)

    def get_attribute(self, name, index=0):
        cls = self._attribute_name_mapping[name]
        try:
            return cls.decode(self.get(cls._key, [])[index])
        except IndexError:
            pass

    def set_attribute(self, name, *args):
        cls = self._attribute_name_mapping[name]
        obj = args[0] if isinstance(args[0], cls) else cls(*args)
        self[cls._key] = [obj.encode()]

    def add_attribute(self, name, *args):
        cls = self._attribute_name_mapping[name]
        obj = args[0] if isinstance(args[0], cls) else cls(*args)
        self.setdefault(cls._key, []).append(obj.encode())


class Attribute(AttributeBase):
    def __init__(self, *args):
        if args and type(args[0]) == type(self):
            self._value = args[0]._value
        elif args:
            self.init(*args)
        else:
            self._value = ()

    @property
    def value(self):
        return self._value

    def __eq__(self, other):
        return type(self) == type(other) and self._value == other._value

    def __format__(self, format_spec):
        if format_spec == 'args':
            return ", ".join(["{!r}".format(v) for v in self._value])
        elif format_spec == 'data':
            return ", ".join(["{!s}".format(v) for v in self._value])
        else:
            return super(Attribute, self).__format__(format_spec)

    def encode(self):
        octets = self._encode_struct(self._fmt[0], *self._value)
        if not self._fmt[1] <= len(octets) <= self._fmt[2]:
            errstr = "data length is out of limits {1} <= {0} <= {2}"
            raise self._encode_error(errstr, len(octets), *self._fmt[1:3])
        return octets

    def _encode_struct(self, fmt, *values):
        return Record._encode_struct(fmt, *values)

    @classmethod
    def decode(cls, octets, offset=0):
        if not cls._fmt[1] <= len(octets) <= cls._fmt[2]:
            errstr = "data length is out of limits {1} <= {0} <= {2}"
            raise cls._decode_error(errstr, len(octets), *cls._fmt[1:3])
        values = cls._decode_struct(cls._fmt[0], octets, offset)
        return cls._from_decode(*values)

    @classmethod
    def _decode_struct(cls, fmt, octets, offset=0):
        return Record._decode_struct(fmt, octets, offset, always_tuple=True)

    @classmethod
    def _from_decode(cls, *values):
        return cls(*values)


class IntegerAttribute(Attribute):
    """Base class for Attributes that contain a single integer value.

    """
    def init(self, value):
        self._value = (int(value),)

    @property
    def value(self):
        return self._value[0]


class BooleanAttribute(Attribute):
    """Base class for Attributes that contain a single boolean value.

    """
    def init(self, value):
        self._value = (bool(value),)

    @property
    def value(self):
        return self._value[0]


class OctetsAttribute(Attribute):
    """Base class for Attributes that contain an octet string value.

    """
    def init(self, value):
        self._value = (bytes(bytearray(value) if _PY2 else value),)

    @property
    def value(self):
        return self._value[0]

    def __format__(self, format_spec):
        if format_spec == 'data':
            value = bytearray(self.value) if _PY2 else self.value
            return ":".join(["{:02X}".format(x) for x in value])
        else:
            return super(OctetsAttribute, self).__format__(format_spec)


class AsciiAttribute(Attribute):
    """Base class for Attributes that contain an ascii string value.

    """
    def init(self, *args):
        value = Record._value_to_ascii(args[0], 'value').encode('ascii')
        self._value = (value,)

    @property
    def value(self):
        return self._value[0] if _PY2 else self._value[0].decode('ascii')

    def __format__(self, format_spec):
        if format_spec == 'data':
            return "{}".format(self.value)
        else:
            return super(AsciiAttribute, self).__format__(format_spec)

    @classmethod
    def _from_decode(cls, *values):
        return cls(values[0].decode('ascii'))


class UnicodeAttribute(Attribute):
    """Base class for Attributes that contain a UTF-8 string value.

    """
    def init(self, *args):
        value = Record._value_to_unicode(args[0], 'value').encode('utf-8')
        self._value = (value,)

    @property
    def value(self):
        return self._value[0] if _PY2 else self._value[0].decode('utf-8')

    def __format__(self, format_spec):
        if format_spec == 'data':
            return "{}".format(self.value)
        else:
            return super(UnicodeAttribute, self).__format__(format_spec)

    @classmethod
    def _from_decode(cls, *values):
        return cls(values[0].decode('utf-8'))


class UUIDAttribute(Attribute):
    """Base class for Attributes that contain a UUID value.

    """
    def init(self, *args):
        if isinstance(args[0], uuid.UUID):
            self._value = (args[0].bytes,)
        elif isinstance(args[0], (bytes, bytearray)) and len(args[0]) == 16:
            self._value = (bytes(args[0]),)
        else:
            self._value = (uuid.UUID(args[0]).bytes,)

    @property
    def value(self):
        return str(uuid.UUID(bytes=self._value[0]))

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "'{}'".format(self.value)
        elif format_spec == 'data':
            return "{}".format(self.value)
        else:
            return super(UUIDAttribute, self).__format__(format_spec)


class BitmapAttribute(Attribute):
    """Base class for Attributes that map bits to values.

    """
    def init(self, *args):
        assert len(args) > 0, "at least one argument is required"
        if isinstance(args[0], int):
            self._value = (args[0],)
        else:
            args = args[0] if isinstance(args[0], (tuple, list)) else args
            bits = [self._get_mask(v) for v in args]
            self._value = (reduce(operator.or_, bits),)

    @property
    def value(self):
        bitmap, value = (self._bitmap, self._value[0])
        names = [name for mask, name in bitmap if value & mask == mask]
        return tuple([self._value[0]] + names)

    @classmethod
    def _get_mask(cls, name):
        names = [_name for _mask, _name in cls._bitmap]
        return cls._bitmap[names.index(name)][0]

    def __format__(self, format_spec):
        if format_spec == 'data':
            fmt = "0x{{val:0{width}X}} {{lst}}".format(width=self._fmt[1]*2)
            return fmt.format(val=self._value[0], lst=list(self.value[1:]))
        else:
            return super(BitmapAttribute, self).__format__(format_spec)

    def __contains__(self, name):
        mask = self._get_mask(name)
        return self._value[0] & mask == mask


class DeviceTypeAttribute(Attribute):
    """Base class for Primary and Secondary Device Type.

    """
    _mapping = (
        (0x0001000000000000, "Computer::"),
        (0x00010050F2040001, "Computer::PC"),
        (0x00010050F2040002, "Computer::Server"),
        (0x00010050F2040003, "Computer::MediaCenter"),
        (0x00010050F2040004, "Computer::UltraMobile"),
        (0x00010050F2040005, "Computer::Notebook"),
        (0x00010050F2040006, "Computer::Desktop"),
        (0x00010050F2040007, "Computer::MobileInternetDevice"),
        (0x00010050F2040008, "Computer::Netbook"),
        (0x00010050F2040009, "Computer::Tablet"),
        (0x00010050F2040009, "Computer::Ultrabook"),
        (0x0002000000000000, "Input::"),
        (0x00020050F2040001, "Input::Keyboard"),
        (0x00020050F2040002, "Input::Mouse"),
        (0x00020050F2040003, "Input::Joystick"),
        (0x00020050F2040004, "Input::Trackball"),
        (0x00020050F2040005, "Input::GameController"),
        (0x00020050F2040006, "Input::Remote"),
        (0x00020050F2040007, "Input::Touchscreen"),
        (0x00020050F2040008, "Input::BiometricReader"),
        (0x00020050F2040009, "Input::BarcodeReader"),
        (0x0003000000000000, "Printer::"),
        (0x00030050F2040002, "Printer::Scanner"),
        (0x00030050F2040003, "Printer::Fax"),
        (0x00030050F2040004, "Printer::Copier"),
        (0x00030050F2040005, "Printer::Multifunction"),
        (0x0004000000000000, "Camera::"),
        (0x00040050F2040001, "Camera::DigitalStillCamera"),
        (0x00040050F2040002, "Camera::VideoCamera"),
        (0x00040050F2040003, "Camera::WebCamera"),
        (0x00040050F2040004, "Camera::SecurityCamera"),
        (0x0005000000000000, "Storage::"),
        (0x00050050F2040001, "Storage::NAS"),
        (0x0006000000000000, "Network::"),
        (0x00060050F2040001, "Network::AccessPoint"),
        (0x00060050F2040002, "Network::Router"),
        (0x00060050F2040003, "Network::Switch"),
        (0x00060050F2040004, "Network::Gateway"),
        (0x00060050F2040005, "Network::Bridge"),
        (0x0007000000000000, "Display::"),
        (0x00070050F2040001, "Display::Television"),
        (0x00070050F2040002, "Display::PictureFrame"),
        (0x00070050F2040003, "Display::Projector"),
        (0x00070050F2040004, "Display::Monitor"),
        (0x0008000000000000, "Multimedia::"),
        (0x00080050F2040001, "Multimedia::DigitalAudioRecorder"),
        (0x00080050F2040002, "Multimedia::PersonalVideoRecorder"),
        (0x00080050F2040003, "Multimedia::MediaCenterExtender"),
        (0x00080050F2040004, "Multimedia::SetTopBox"),
        (0x00080050F2040005, "Multimedia::ServerAdapterExtender"),
        (0x00080050F2040006, "Multimedia::PortableVideoPlayer"),
        (0x0009000000000000, "Gaming::"),
        (0x00090050F2040001, "Gaming::Xbox"),
        (0x00090050F2040002, "Gaming::Xbox360"),
        (0x00090050F2040003, "Gaming::Playstation"),
        (0x00090050F2040004, "Gaming::Console"),
        (0x00090050F2040005, "Gaming::Portable"),
        (0x000A000000000000, "Telephone::"),
        (0x000A0050F2040001, "Telephone::WindowsMobile"),
        (0x000A0050F2040002, "Telephone::SingleModePhone"),
        (0x000A0050F2040003, "Telephone::DualModePhone"),
        (0x000A0050F2040004, "Telephone::SingleModeSmartphone"),
        (0x000A0050F2040005, "Telephone::DualModeSmartphone"),
        (0x000B000000000000, "Audio::"),
        (0x000B0050F2040001, "Audio::Receiver"),
        (0x000B0050F2040002, "Audio::Speaker"),
        (0x000B0050F2040003, "Audio::PortableMusicPlayer"),
        (0x000B0050F2040004, "Audio::Headset"),
        (0x000B0050F2040005, "Audio::Headphone"),
        (0x000B0050F2040006, "Audio::Microphone"),
        (0x000B0050F2040006, "Audio::HomeTheater"),
        (0x000C000000000000, "Dock::"),
        (0x000C0050F2040001, "Dock::Computer"),
        (0x000C0050F2040001, "Dock::Media"),
    )

    def init(self, *args):
        values = [v if isinstance(v, int) else self._get_enum(v) for v in args]
        self._value = tuple(values)

    @property
    def value(self):
        return tuple(["{}".format(self._get_name(v)) for v in self._value])

    def _get_enum(self, value):
        for enum, name in self._mapping:
            if value == name:
                return enum
        raise ValueError("{!r} does not have a known mapping".format(value))

    def _get_name(self, value):
        for enum, name in self._mapping:
            if value == enum:
                return name
        category = value >> 48
        for enum, name in self._mapping:
            if enum >> 48 == category:
                return "{}{:012X}".format(name, value & 0xFFFFFFFFFFFF)
        return "{:04X}::{:012X}".format(category, value & 0xFFFFFFFFFFFF)

    def __format__(self, format_spec):
        if format_spec == 'args':
            return ", ".join(["0x{:016X}".format(v) for v in self._value])
        elif format_spec == 'data':
            return " ".join(["{}".format(v) for v in self.value])
        else:
            return super(DeviceTypeAttribute, self).__format__(format_spec)


class VersionAttribute(Attribute):
    """Base class for Attributes that contain a version number.

    """
    def init(self, *args):
        value = (args[0] << 4 | args[1] & 15) if len(args) == 2 else args[0]
        self._value = (value,)

    @property
    def value(self):
        return VersionTuple(self._value[0] >> 4, self._value[0] & 15)

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{}, {}".format(*self.value)
        elif format_spec == 'data':
            return "{}.{}".format(*self.value)
        else:
            return super(VersionAttribute, self).__format__(format_spec)

#
# Wi-Fi Simple Configuration Attributes
#


class APChannel(IntegerAttribute):
    _str = "AP Channel"
    _fmt = ('H', 2, 2)
    _key = 0x1001


class AuthenticationType(BitmapAttribute):
    _str = "Authentication Type"
    _fmt = ('H', 2, 2)
    _key = 0x1003
    _bitmap = (
        (0x0001, 'Open'),
        (0x0002, 'WPA-Personal'),
        (0x0004, 'Shared'),
        (0x0008, 'WPA-Enterprise'),
        (0x0010, 'WPA2-Enterprise'),
        (0x0020, 'WPA2-Personal'))


class ConfigMethods(BitmapAttribute):
    _str = "Configuration Methods"
    _fmt = ('H', 2, 2)
    _key = 0x1008
    _bitmap = (
        (0x0001, 'USBA'),
        (0x0002, 'Ethernet'),
        (0x0004, 'Label'),
        (0x0008, 'Display'),
        (0x0010, 'External NFC Token'),
        (0x0020, 'Integrated NFC Token'),
        (0x0040, 'NFC Interface'),
        (0x0080, 'Push Button'),
        (0x0100, 'Keypad'),
        (0x0280, 'Virtual Push Button'),
        (0x0480, 'Physical Push Button'),
        (0x2008, 'Virtual Display PIN'),
        (0x4008, 'Physical Display PIN'))


class DeviceName(UnicodeAttribute):
    _str = "Device Name"
    _fmt = ('*', 0, 64)
    _key = 0x1011


class EncryptionType(BitmapAttribute):
    _str = "Encryption Type"
    _fmt = ('H', 2, 2)
    _key = 0x100F
    _bitmap = (
        (0x0001, 'None'),
        (0x0002, 'WEP'),
        (0x0004, 'TKIP'),
        (0x0008, 'AES'),
    )


class KeyProvidedAutomatically(BooleanAttribute):
    _str = "Key Provided Automatically"
    _fmt = ('?', 1, 1)
    _key = 0x1061


class MacAddress(OctetsAttribute):
    _str = "MAC Address"
    _fmt = ('*', 6, 6)
    _key = 0x1020


class Manufacturer(AsciiAttribute):
    _str = "Manufacturer"
    _fmt = ('*', 0, 64)
    _key = 0x1021


class ModelName(AsciiAttribute):
    _str = "Model Name"
    _fmt = ('*', 0, 32)
    _key = 0x1023


class ModelNumber(AsciiAttribute):
    _str = "Model Number"
    _fmt = ('*', 0, 32)
    _key = 0x1024


class NetworkIndex(IntegerAttribute):
    _str = "Network Index"
    _fmt = ('B', 1, 1)
    _key = 0x1026


class NetworkKey(OctetsAttribute):
    _str = "Network Key"
    _fmt = ('*', 0, 64)
    _key = 0x1027


class NetworkKeyShareable(BooleanAttribute):
    _str = "Network Key Shareable"
    _fmt = ('?', 1, 1)
    _key = 0x02


class OutOfBandPassword(Attribute):
    _str = "Out Of Band Device Password"
    _fmt = ('>20sH*', 22, 58)
    _key = 0x102C

    def init(self, *args):
        pkhash, pwd_id, passwd = args[0] if len(args) == 1 else args
        assert len(pkhash) == 20
        assert 16 <= pwd_id <= 0xffff or pwd_id == 7
        assert 16 <= len(passwd) <= 32 or len(passwd) == 0
        assert (pwd_id == 7 and not passwd) or (pwd_id != 7 and passwd)
        self._value = (bytes(pkhash), pwd_id, bytes(passwd))

    @property
    def public_key_hash(self):
        """The read-only 20 bytes of the device's public key hash."""
        return self._value[0]

    @property
    def password_id(self):
        """The read-only integer value of the Password ID field."""
        return self._value[1]

    @property
    def device_password(self):
        """The read-only bytes of the zero or 16-32 octets Device Password."""
        return self._value[2]

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{!r}, {!r}, {!r}".format(*self.value)
        elif format_spec == 'data':
            s = "HASH {1} ID {0} PWD {2}"
            return s.format(self.value[1], *map(hexlify, self.value[::2]))
        else:
            return super(OutOfBandPassword, self).__format__(format_spec)


class PrimaryDeviceType(DeviceTypeAttribute):
    _str = "Primary Device Type"
    _fmt = ("Q", 8, 8)
    _key = 0x1054

    def init(self, value):
        super(PrimaryDeviceType, self).init(value)

    @property
    def value(self):
        return super(PrimaryDeviceType, self).value[0]

    def __format__(self, format_spec):
        if format_spec == 'data':
            return self.value
        else:
            return super(PrimaryDeviceType, self).__format__(format_spec)


class RFBands(BitmapAttribute):
    _str = "RF Bands"
    _fmt = ('B', 1, 1)
    _key = 0x103C
    _bitmap = (
        (0x01, '2.4GHz'),
        (0x02, '5.0GHz'),
        (0x04, '60GHz'),
    )


class SecondaryDeviceTypeList(DeviceTypeAttribute):
    _str = "Secondary Device Type List"
    _fmt = ("*Q", 8, 128)
    _key = 0x1055


class SerialNumber(AsciiAttribute):
    _str = "Serial Number"
    _fmt = ('*', 0, 32)
    _key = 0x1042


class SSID(OctetsAttribute):
    _str = "SSID"
    _fmt = ('*', 0, 32)
    _key = 0x1045


class UUIDEnrollee(UUIDAttribute):
    _str = "UUID-E"
    _fmt = ('16s', 16, 16)
    _key = 0x1047


class UUIDRegistrar(UUIDAttribute):
    _str = "UUID-R"
    _fmt = ('16s', 16, 16)
    _key = 0x1048


class Version1(VersionAttribute):
    _str = "Version1"
    _fmt = ('B', 1, 1)
    _key = 0x104A


class Version2(VersionAttribute):
    _str = "Version2"
    _fmt = ('B', 1, 1)
    _key = 0x00


class VendorExtension(Attribute):
    _str = "Vendor Extension"
    _fmt = ('3s*', 3, 1024)
    _key = 0x1049

    def init(self, *args):
        vendor_id, vendor_data = args if len(args) > 1 else args[0]
        self._value = (vendor_id, vendor_data)

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{!r}, {!r}".format(*self._value)
        elif format_spec == 'data':
            return "ID {} DATA {}".format(*map(hexlify, self._value))
        else:
            return super(VendorExtension, self).__format__(format_spec)


class WifiAllianceVendorExtension(AttributeContainer):
    _str = "WFA Vendor Extension"
    _attribute_name_mapping = {
        'version-2': Version2,
        'network-key-shareable': NetworkKeyShareable,
    }

    def encode(self):
        data = [b'\x00\x37\x2A']
        for tlv_type in self.keys():
            for tlv_value in self.get(tlv_type):
                data.append(Record._encode_struct('BB+', tlv_type, tlv_value))
        return b''.join(data)

    @classmethod
    def decode(cls, octets):
        assert octets.startswith(b'\x00\x37\x2A')
        attrs = []
        offset = 3
        while offset < len(octets):
            tlv_type, tlv_value = Record._decode_struct('BB+', octets, offset)
            attrs.append((tlv_type, tlv_value))
            offset += len(tlv_value) + 2
        return cls(*attrs)


class Credential(AttributeContainer):
    _str = "Credential"
    _key = 0x100E
    _attribute_name_mapping = {
        'authentication-type': AuthenticationType,
        'encryption-type': EncryptionType,
        'key-provided-automatically': KeyProvidedAutomatically,
        'mac-address': MacAddress,
        'network-index': NetworkIndex,
        'network-key': NetworkKey,
        'ssid': SSID,
        'vendor-extension': VendorExtension,
    }

    def encode(self):
        tlvs = []
        for tlv_type in sorted(self.keys()):
            for tlv_value in self.get(tlv_type):
                tlvs.append(Record._encode_struct('>HH+', tlv_type, tlv_value))
        return b''.join(tlvs)

    @classmethod
    def decode(cls, octets):
        attrs = []
        offset = 0
        while offset < len(octets):
            tlv_type, tlv_value = Record._decode_struct('>HH+', octets, offset)
            attrs.append((tlv_type, tlv_value))
            offset += len(tlv_value) + 4
        return cls(*attrs)

#
# Wi-Fi Peer To Peer Attributes
#


class PeerToPeerCapability(Attribute):
    _str = "P2P Capability"
    _fmt = ('BB', 2, 2)
    _key = 2

    class Capability(object):
        _bitmap = NotImplemented

        def __init__(self, *args):
            assert len(args) > 0, "at least one argument is needed"
            if isinstance(args[0], int):
                self._value = args[0]
            else:
                args = args[0] if isinstance(args[0], (tuple, list)) else args
                bits = [1 << self._bitmap.index(v) for v in args]
                self._value = reduce(operator.or_, bits)

        @property
        def value(self):
            val, bitmap = (self._value, self._bitmap)
            return tuple([val] + [bitmap[i] for i in range(8) if val & 1 << i])

        def __str__(self):
            return str(list(self.value[1:]))

    class DeviceCapability(Capability):
        _str = "Device Capability"
        _bitmap = (
            'Service Discovery',
            'P2P Client Discoverability',
            'Concurrent Operation',
            'P2P Infastructure Managed',
            'P2P Device Limit',
            'P2P Invitation Procedure',
            'Reserved Bit 6',
            'Reserved Bit 7')

    class GroupCapability(Capability):
        _str = "Group Capability"
        _bitmap = (
            'P2P Group Owner',
            'Persistent P2P Group',
            'P2P Group Limit',
            'Intra-BSS Distribution',
            'Cross Connection',
            'Persistent Reconnect',
            'Group Formation',
            'IP Address Allocation')

    def init(self, device_capability, group_capability):
        if isinstance(device_capability, int):
            device_capability = (device_capability,)
        if isinstance(group_capability, int):
            group_capability = (group_capability,)
        self._value = (
            PeerToPeerCapability.DeviceCapability(*device_capability).value[0],
            PeerToPeerCapability.GroupCapability(*group_capability).value[0])

    @property
    def device_capability(self):
        return PeerToPeerCapability.DeviceCapability(self._value[0]).value

    @property
    def group_capability(self):
        return PeerToPeerCapability.GroupCapability(self._value[1]).value

    def __format__(self, format_spec):
        if format_spec == 'data':
            return "Device {} Group {}".format(
                PeerToPeerCapability.DeviceCapability(self._value[0]),
                PeerToPeerCapability.GroupCapability(self._value[1]))
        else:
            return super(PeerToPeerCapability, self).__format__(format_spec)


class ChannelList(Attribute):
    _str = "Channel List"
    _fmt = ('', 3, 0xFFFF)
    _key = 11

    class ChannelEntry(object):
        def __init__(self, *args):
            self._value = args

        @property
        def operating_class(self):
            return self._value[0]

        @property
        def channel_numbers(self):
            return self._value[1]

        def __str__(self):
            return "Class {} Channels {}".format(
                self.operating_class, list(self.channel_numbers))

    def init(self, *args):
        self._value = args

    @property
    def country_string(self):
        return self._value[0]

    def __len__(self):
        return len(self._value) - 1

    def __getitem__(self, key):
        return ChannelList.ChannelEntry(*self._value[1:][key])

    def __format__(self, format_spec):
        if format_spec == 'data':
            return "Country {} Table E-{} {}".format(
                self.country_string[0:2].decode('ascii').upper(),
                (int, ord)[_PY2](self.country_string[2]),
                ', '.join([str(v) for v in self]))
        else:
            return super(ChannelList, self).__format__(format_spec)

    def _encode_struct(self, fmt, *values):
        super_obj = super(ChannelList, self)
        octets = [super_obj._encode_struct('3s', values[0])]
        for channel_entry in values[1:]:
            octets.append(super_obj._encode_struct('BB+(B)', *channel_entry))
        return b''.join(octets)

    @classmethod
    def _decode_struct(cls, fmt, octets, offset=0):
        super_cls = super(ChannelList, cls)
        country_string = super_cls._decode_struct('3s', octets, offset)[0]
        channel_entry_list = list()
        offset = 3
        while offset < len(octets):
            channel_entry = super_cls._decode_struct('BB+(B)', octets, offset)
            channel_entry_list.append(channel_entry)
            offset += 2 + len(channel_entry[1])
        return tuple([country_string] + channel_entry_list)


class PeerToPeerDeviceInfo(Attribute):
    _str = "P2P Device Info"
    _fmt = ('6sHQB+(Q)*', 21, 0xFFFF)
    _key = 13

    def init(self, adr, cfg, pdt, sdtl, name):
        adr = MacAddress(adr)._value[0]
        cfg = ConfigMethods(*(cfg, [cfg])[isinstance(cfg, int)])._value[0]
        pdt = PrimaryDeviceType(pdt)._value[0]
        sdtl = SecondaryDeviceTypeList(*sdtl)._value
        tail = self._encode_struct('HH+', DeviceName._key,
                                   DeviceName(name).encode())
        self._value = (adr, cfg, pdt, sdtl, tail)

    @classmethod
    def _from_decode(cls, adr, cfg, pdt, sdtl, tail):
        assert cls._decode_struct('H', tail)[0] == DeviceName._key
        attribute = cls()
        attribute._value = (adr, cfg, pdt, sdtl, tail)
        return attribute

    @property
    def device_address(self):
        return MacAddress(self._value[0]).value

    @property
    def config_methods(self):
        return ConfigMethods(self._value[1]).value

    @property
    def primary_device_type(self):
        return PrimaryDeviceType(self._value[2]).value

    @property
    def secondary_device_type_list(self):
        return SecondaryDeviceTypeList(*self._value[3]).value

    @property
    def device_name(self):
        return DeviceName(self._decode_struct('HH+', self._value[4])[1]).value

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{!r}, {!r}, {!r}, {!r}, {name!r}".format(
                *self._value[0:4], name=self.device_name)
        elif format_spec == 'data':
            s = ["{:data}".format(MacAddress(self._value[0]))]
            s += ["{:data}".format(ConfigMethods(self._value[1]))]
            s += ["{:data}".format(PrimaryDeviceType(self._value[2]))]
            s += ["{:data}".format(SecondaryDeviceTypeList(*self._value[3]))]
            s += ["'{}'".format(self.device_name)]
            return " ".join(s)
        else:
            return super(PeerToPeerDeviceInfo, self).__format__(format_spec)


class PeerToPeerGroupInfo(Attribute):
    _str = "P2P Group Info"
    _fmt = ('6s6sBHQB+(Q)HH+', 0, 0xFFFF)
    _key = 14

    class Descriptor(object):
        def __init__(self, *args):
            self._value = (
                MacAddress(args[0])._value[0],
                MacAddress(args[1])._value[0],
                PeerToPeerCapability.DeviceCapability(args[2])._value,
                ConfigMethods(args[3])._value[0],
                PrimaryDeviceType(args[4])._value[0],
                SecondaryDeviceTypeList(*args[5])._value,
                DeviceName(args[6])._value[0])

        @property
        def device_address(self):
            return MacAddress(self._value[0]).value

        @property
        def interface_address(self):
            return MacAddress(self._value[1]).value

        @property
        def device_capability(self):
            return PeerToPeerCapability.DeviceCapability(self._value[2]).value

        @property
        def config_methods(self):
            return ConfigMethods(self._value[3]).value

        @property
        def primary_device_type(self):
            return PrimaryDeviceType(self._value[4]).value

        @property
        def secondary_device_type_list(self):
            return SecondaryDeviceTypeList(*self._value[5]).value

        @property
        def device_name(self):
            return DeviceName(self._value[6]).value

        def __str__(self):
            format_string = "{:data} {:data} Capability {} Config {:data} "\
                            "Type '{:data} {:data}' Name '{:data}'"
            return format_string.format(
                MacAddress(self._value[0]), MacAddress(self._value[1]),
                PeerToPeerCapability.DeviceCapability(self._value[2]),
                ConfigMethods(self._value[3]),
                PrimaryDeviceType(self._value[4]),
                SecondaryDeviceTypeList(*self._value[5]),
                DeviceName(self._value[6]))

    def init(self, *args):
        client_info = PeerToPeerGroupInfo.Descriptor
        self._value = tuple([client_info(*arg)._value for arg in args])

    def __len__(self):
        return len(self._value)

    def __getitem__(self, key):
        return PeerToPeerGroupInfo.Descriptor(*self._value[key])

    def __format__(self, format_spec):
        if format_spec == 'data':
            s = "(Device {}: {})"
            return ', '.join([s.format(i+1, v) for i, v in enumerate(self)])
        else:
            return super(PeerToPeerGroupInfo, self).__format__(format_spec)

    def _encode_struct(self, fmt, *values):
        super_obj = super(PeerToPeerGroupInfo, self)
        octets = list()
        for client_info in map(list, values):
            client_info.insert(6, DeviceName._key)
            octets.append(super_obj._encode_struct(fmt, *client_info))
        return b''.join(octets)

    @classmethod
    def _decode_struct(cls, fmt, octets, offset=0):
        super_cls = super(PeerToPeerGroupInfo, cls)
        p2p_client_info_descriptors = list()
        while offset < len(octets):
            client_info = list(super_cls._decode_struct(fmt, octets, offset))
            wsc_device_name_key = client_info.pop(6)
            assert wsc_device_name_key == DeviceName._key
            offset += 28 + len(client_info[5] * 8) + len(client_info[6])
            p2p_client_info_descriptors.append(tuple(client_info))
        return tuple(p2p_client_info_descriptors)


class PeerToPeerGroupID(Attribute):
    _str = "P2P Group ID"
    _fmt = ('6s*', 6, 38)
    _key = 15

    def init(self, device_address, ssid):
        self._value = (MacAddress(device_address)._value[0],
                       SSID(ssid)._value[0])

    @property
    def device_address(self):
        return MacAddress(self._value[0]).value

    @property
    def ssid(self):
        return SSID(self._value[1]).value

    def __format__(self, format_spec):
        if format_spec == 'data':
            return "{:data} {}".format(
                MacAddress(self._value[0]), SSID(self._value[1]))
        else:
            return super(PeerToPeerGroupID, self).__format__(format_spec)


class NegotiationChannel(Attribute):
    _str = "Negotiation Channel"
    _fmt = ('3sBBB', 6, 6)
    _key = 19

    _role_indication_names = ("Not Member", "Group Client", "Group Owner")

    def init(self, country_string, operating_class, channel_number,
             role_indication):
        self._value = (country_string, operating_class, channel_number,
                       role_indication if isinstance(role_indication, int)
                       else self._role_indication_names.index(role_indication))

    country_string = property(lambda self: self._value[0])
    operating_class = property(lambda self: self._value[1])
    channel_number = property(lambda self: self._value[2])

    @property
    def role_indication(self):
        value = self._value[3]
        try:
            return (value, self._role_indication_names[value])
        except IndexError:
            return (value,)

    def __format__(self, format_spec):
        if format_spec == 'data':
            country = self.country_string[0:2].decode('ascii').upper()
            table_e = (int, ord)[_PY2](self.country_string[2])
            s = ["Country {} Table E-{}".format(country, table_e)]
            s += ["Class {}".format(self.operating_class)]
            s += ["Channel {}".format(self.channel_number)]
            s += ["Role {!r}".format(self.role_indication[-1])]
            return " ".join(s)
        else:
            return super(NegotiationChannel, self).__format__(format_spec)


class WifiSimpleConfigRecord(AttributeContainer, GlobalRecord):
    _type = 'application/vnd.wfa.wsc'

    _attribute_name_mapping = {
        'ap-channel': APChannel,
        'credential': Credential,
        'device-name': DeviceName,
        'mac-address': MacAddress,
        'manufacturer': Manufacturer,
        'model-name': ModelName,
        'model-number': ModelNumber,
        'oob-password': OutOfBandPassword,
        'primary-device-type': PrimaryDeviceType,
        'rf-bands': RFBands,
        'secondary-device-type-list': SecondaryDeviceTypeList,
        'serial-number': SerialNumber,
        'ssid': SSID,
        'uuid-enrollee': UUIDEnrollee,
        'uuid-registrar': UUIDRegistrar,
        'vendor-extension': VendorExtension,
        'version-1': Version1,
    }

    def get_attribute(self, name, index=0):
        if name == 'wfa-vendor-extension':
            vxl = self.get(VendorExtension._key, [])
            wfa = [vx for vx in vxl if vx.startswith(b'\0\x37\x2A')]
            if len(wfa) > index:
                return WifiAllianceVendorExtension.decode(wfa[index])
        else:
            _super = super(WifiSimpleConfigRecord, self)
            return _super.get_attribute(name, index)

    def set_attribute(self, name, *args):
        if name == 'wfa-vendor-extension':
            # delete all WFA Vendor Extensions and then add attribute
            WFA = WifiAllianceVendorExtension
            wfa = args[0] if isinstance(args[0], WFA) else WFA(*args)
            vxl = self.get(VendorExtension._key, [])
            vxl = [vx for vx in vxl if not vx.startswith(b'\0\x37\x2A')]
            self[VendorExtension._key] = vxl + [wfa.encode()]
        else:
            _super = super(WifiSimpleConfigRecord, self)
            return _super.set_attribute(name, *args)

    def add_attribute(self, name, *args):
        if name == 'wfa-vendor-extension':
            # delete all WFA Vendor Extensions and then add attribute
            WFA = WifiAllianceVendorExtension
            wfa = args[0] if isinstance(args[0], WFA) else WFA(*args)
            self.setdefault(VendorExtension._key, []).append(wfa.encode())
        else:
            _super = super(WifiSimpleConfigRecord, self)
            return _super.add_attribute(name, *args)

    def __repr__(self):
        return GlobalRecord.__repr__(self)

    def __str__(self):
        return GlobalRecord.__str__(self)

    def _encode_payload(self):
        # Both WSC and P2P attributes are in the same structure but
        # WSC keys are all >= 0x1000 while P2P keys are <= 0xFF. Here
        # we'll filter to encode only WSC attributes.
        octets = []
        for _type in sorted(filter(lambda x: x >= 0x1000, self.keys())):
            for _data in self.get(_type):
                octets.append(Record._encode_struct('HH+', _type, _data))

        # The WSC spec requires the overall attribute size in octets
        # encoded before the TLVs (as a 16-bit unsigned integer) when
        # an OOB Device Password is exchanged with Password ID 0x0007
        # (NFC-Connection-Handover).
        oob_password = self.get_attribute('oob-password')
        if oob_password and oob_password.password_id == 7:
            octets.insert(0, self._encode_struct('H', sum(map(len, octets))))

        return b''.join(octets)

    # Minimum payload is a Type and a Length, each unsigned short.
    _decode_min_payload_length = 4

    @classmethod
    def _decode_payload(cls, octets, errors):
        # Unfortunately the WSC specification uses two different data
        # formats with the same Payload Type. Either the payload
        # starts directly with the first TLV or there is an initial
        # 16-bit unsigned integer that contains the overall size in
        # octets of the following TLV attributes.
        offset = (0, 2)[cls._decode_struct('H', octets) == len(octets) - 2]
        attrs = []

        while offset < len(octets):
            tlv_type, tlv_value = cls._decode_struct('HH+', octets, offset)
            if tlv_type < 0x1000:
                errstr = "reserved attribute type 0x{:04X} at offset {}"
                raise cls._decode_error(errstr, tlv_type, offset)
            attrs.append((tlv_type, tlv_value))
            offset += len(tlv_value) + 4

        return cls(*attrs)


class WifiPeerToPeerRecord(WifiSimpleConfigRecord):
    _type = 'application/vnd.wfa.p2p'

    _attribute_name_mapping = dict([
        ('p2p-capability', PeerToPeerCapability),
        ('p2p-device-info', PeerToPeerDeviceInfo),
        ('negotiation-channel', NegotiationChannel),
        ('channel-list', ChannelList),
        ('p2p-group-info', PeerToPeerGroupInfo),
        ('p2p-group-id', PeerToPeerGroupID),
    ] + list(WifiSimpleConfigRecord._attribute_name_mapping.items()))

    _mandatory_attributes = (
        'manufacturer', 'model-name', 'model-number', 'oob-password',
        'rf-bands', 'serial-number', 'uuid-enrollee', 'vendor-extension',
        'p2p-capability', 'p2p-device-info', 'negotiation-channel')

    def _encode_payload(self):
        # Make sure all the mandatory attributes are present. Unlike
        # decoding, which may skip this test if errors != 'strict', we
        # never encode if a mandatory attribute is missing.
        for attribute_name in self._mandatory_attributes:
            if attribute_name not in self:
                errstr = "'{}' attribute is required for encoding"
                raise self._encode_error(errstr, attribute_name)

        # All WSC Attributes get encoded by the base class.
        wsc_data = super(WifiPeerToPeerRecord, self)._encode_payload()

        # Encode the P2P Attributes. P2P Attribute Type values are all
        # less than 256 (1 byte) while WSC Attribute Type values start
        # at 0x1000. We encode by sorted keys simply to have reliable
        # order for regression testing.
        p2p_data = []
        for _type in sorted(filter(lambda x: x <= 0xFF, self.keys())):
            for _data in self.get(_type):
                p2p_data.append(Record._encode_struct('BH+', _type, _data))
        p2p_data = b''.join(p2p_data)

        # Build the final octets witth len(WSC) + WSC + len(P2P) + P2P
        # data. The len(WSC) may already be encoded (depends on
        # content), if not we'll do it here.
        octets = []
        if self._decode_struct('H', wsc_data) != len(wsc_data) - 2:
            octets.append(self._encode_struct('H', len(wsc_data)))
        octets.append(wsc_data)
        octets.append(self._encode_struct('H', len(p2p_data)))
        octets.append(p2p_data)
        return b''.join(octets)

    # The minimum payload length is to hold the WSC and P2P Data
    # Length values, both are unsigned 16-bit integers.
    _decode_min_payload_length = 4

    @classmethod
    def _decode_payload(cls, octets, errors):
        # The payload contains len(WSC) + WSC + len(P2P) + P2P data.
        wsc_data, p2p_data = cls._decode_struct('H+H+', octets)

        if len(wsc_data) < WifiSimpleConfigRecord._decode_min_payload_length:
            errstr = "insufficient WSC Data Length, got {} octets"
            raise cls._decode_error(errstr, len(wsc_data))

        # Start with decoding the WSC data. Because we inherit from
        # WifiSimpleConfigRecord, this can be done by our base class.
        _super = super(WifiPeerToPeerRecord, cls)
        record = _super._decode_payload(wsc_data, errors)

        # The P2P data is a number of TLV's with 8-bit Type and 16-bit
        # Length fields. Each attribute data is appended to the list
        # under key (setdefault returns the list for key if key is
        # present, otherwise inserts an empty list and returns that).
        offset = 0
        while offset < len(p2p_data):
            tlv_type, tlv_value = cls._decode_struct('BH+', p2p_data, offset)
            record.setdefault(tlv_type, []).append(tlv_value)
            offset += len(tlv_value) + 3

        # For strict decoding we require that all mandatory WSC and
        # P2P Attributes are present.
        if errors == 'strict':
            for attribute_name in cls._mandatory_attributes:
                if attribute_name not in record:
                    errstr = "'{}' attribute is required if errors is '{}'"
                    raise cls._decode_error(errstr, attribute_name, errors)

        return record


Record.register_type(WifiSimpleConfigRecord)
Record.register_type(WifiPeerToPeerRecord)

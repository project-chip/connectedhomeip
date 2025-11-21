# -*- coding: utf-8 -*-
"""Decoding and encoding of the NDEF Smartposter Record.

The NFC Forum Smart Poster Record Type Definition defines a structure
that associates an Internationalized Resource Identifier (or Uniform
Resource Identifier) with various types of metadata. For a user this
is most noteably the ability to attach descriptive text in different
languages as well as image data for icon rendering. For a smartposter
application this is a recommendation for processing as well as
resource type and size hints to guide a strategy for retrieving the
resource.

"""
from __future__ import absolute_import, division
from .message import message_decoder, message_encoder
from .record import Record, GlobalRecord, LocalRecord, convert
from .text import TextRecord
from .uri import UriRecord


class ActionRecord(LocalRecord):
    """This is a local record class used within the payload of a
    Smartposter Record. It encodes an action value that translates
    into either 'exec', 'save', or 'edit' as the recommended course of
    action that strategy for acting on the resource.

    """
    _type = 'urn:nfc:wkt:act'
    _action_strings = ('exec', 'save', 'edit')

    def __init__(self, action=None):
        """Initialize the record. The action argument, if not None, must be an
        acceptable set value for the ActionRecord.action attribute. The
        default, if action is None, is 'exec'.

        """
        self.action = action if action else 'exec'

    @property
    def action(self):
        """Get or set the action value. A set value must be either 'exec',
        'save', 'edit', or an index for that list.

        """
        return self._action_strings[self._action]

    @action.setter
    def action(self, value):
        if value in self._action_strings:
            self._action = self._action_strings.index(value)
        elif isinstance(value, int) and 0 <= value < len(self._action_strings):
            self._action = value
        else:
            errstr = "action may be one of {} or index, but not {!r}"
            raise self._value_error(errstr.format(self._action_strings, value))

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{!r}".format(self.action)

        if format_spec == 'data':
            return "Action '{r.action}'".format(r=self)

        return super(ActionRecord, self).__format__(format_spec)

    def _encode_payload(self):
        return self._encode_struct('B', self._action)

    _decode_min_payload_length = 1
    _decode_max_payload_length = 1

    @classmethod
    def _decode_payload(cls, octets, errors):
        ACTION = cls._decode_struct('B', octets)

        if not ACTION < len(cls._action_strings) and errors == 'strict':
            errmsg = "decoding of ACTION value {} is not defined"
            raise cls._decode_error(errmsg.format(ACTION))

        action = ACTION if ACTION < len(cls._action_strings) else 0
        return ActionRecord(action)


class SizeRecord(LocalRecord):
    """This is a local record class used within the payload of a
    Smartposter Record. It encodes the size of the resource referred
    to by the IRI/URI as a 32-bit unsigned integer.

    """
    _type = 'urn:nfc:wkt:s'

    def __init__(self, resource_size=None):
        """Initialize the record with a resource_size integer value. The
        default is 0.

        """
        self.resource_size = resource_size if resource_size is not None else 0

    @property
    def resource_size(self):
        """Get or set the resource size value."""
        return self._value

    @resource_size.setter
    def resource_size(self, value):
        if isinstance(value, int) and 0 <= value <= 0xFFFFFFFF:
            self._value = value
        else:
            errstr = "resource_size expects 32-bit unsigned int, but got {!r}"
            raise self._value_error(errstr.format(value))

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{r.resource_size!r}".format(r=self)

        if format_spec == 'data':
            return "Resource Size '{r.resource_size} byte'".format(r=self)

        return super(SizeRecord, self).__format__(format_spec)

    def _encode_payload(self):
        return self._encode_struct('>L', self.resource_size)

    _decode_min_payload_length = 4
    _decode_max_payload_length = 4

    @classmethod
    def _decode_payload(cls, octets, errors):
        return SizeRecord(cls._decode_struct('>L', octets))


class TypeRecord(LocalRecord):
    """This is a local record class used within the payload of a
    Smartposter Record. It encodes the type of the resource referred
    to by the IRI/URI as a, typically mime-type, string.

    """
    _type = 'urn:nfc:wkt:t'

    def __init__(self, resource_type=None):
        """Initialize the record with a resource_type text value. The
        default is an empty string.

        """
        self.resource_type = resource_type if resource_type is not None else ''

    @property
    def resource_type(self):
        """Get or set the resource type string."""
        return self._value

    @resource_type.setter
    @convert('value_to_unicode')
    def resource_type(self, value):
        self._value = value

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{r.resource_type!r}".format(r=self)

        if format_spec == 'data':
            return "Resource Type '{r.resource_type}'".format(r=self)

        return super(TypeRecord, self).__format__(format_spec)

    def _encode_payload(self):
        return self.resource_type.encode('utf-8')

    @classmethod
    def _decode_payload(cls, octets, errors):
        try:
            TYPE = octets.decode('utf-8')
        except UnicodeDecodeError:
            errstr = "can't decode payload as utf-8"
            raise cls._decode_error(errstr.format(errstr))
        return TypeRecord(TYPE)


class SmartposterRecord(GlobalRecord):
    """The SmartposterRecord provides the attributes and methods to
    conviniently access the content of a decoded NDEF Smart Poster
    Record or to encode one through the ndef.message_encoder.

    >>> import ndef
    >>> record = ndef.SmartposterRecord('https://github.com/nfcpy/ndeflib')
    >>> record.set_title("NFC Data Exchange Format decoder", "en")
    >>> record.set_title("NFC Datenaustauschformat Dekodierer", "de")
    >>> iconfile = open('images/ndeflib.ico', 'rb')
    >>> record.add_icon('image/x-icon', iconfile.read())
    >>> record.resource_type = 'text/html'
    >>> record.action = 'exec'

    """
    _type = 'urn:nfc:wkt:Sp'

    def __init__(self, resource='', title=None, action=None, icon=None,
                 resource_size=None, resource_type=None):
        """Initialize the record with resource, title, action, icon,
        resource_size and resource_type arguments. The resource
        argument is an IRI string. The title argument is either an
        English title string or a dictionary of {language: title}
        items. The action argument is a string or index acceptable as
        a ActionRecord.action attribute. The icon argument is either a
        sequence of 'image/png' data bytes or a dictionary of
        {mime-type: image-data} items. The resource_type is a text
        value that is usually a mime-type string. The resource_size is
        an integer.

        """
        self.title_records = []
        self.uri_records = []
        self.action_records = []
        self.icon_records = []
        self.size_records = []
        self.type_records = []
        if resource is not None:
            self.resource = resource
        if title is not None:
            if isinstance(title, dict):
                for lang, text in title.items():
                    self.set_title(text, lang)
            else:
                self.set_title(title)
        if action is not None:
            self.action = action
        if icon is not None:
            png_header = b'\x89PNG\x0d\x0a\x1a\x0a'
            if isinstance(icon, dict):
                for icon_type, icon_data in icon.items():
                    self.add_icon(icon_type, icon_data)
            elif isinstance(icon, bytes):
                if icon.startswith(png_header):
                    self.add_icon('image/png', icon)
                else:
                    errstr = "init requires icon bytes with png header, not {}"
                    raise self._value_error(errstr.format(repr(icon[0:8])))
            else:
                errstr = "init icon argument must be bytes or mapping, not {}"
                raise self._value_error(errstr.format(type(icon).__name__))
        if resource_size is not None:
            self.resource_size = resource_size
        if resource_type is not None:
            self.resource_type = resource_type

    @property
    def resource(self):
        """Get or set the Smartposter resource identifier. A set value is
        interpreted as an internationalized resource identifier (so it
        can be unicode). When reading, the resource attribute returns
        the internal ndef.UriRecord instance with the UriRecord.iri
        and UriRecord.uri attributes. This can also be used to set a
        value as either IRI or URI.

        """
        try:
            return self.uri_records[0]
        except IndexError:
            return None

    @resource.setter
    @convert('value_to_unicode')
    def resource(self, value):
        try:
            self.uri_records[0].iri = value
        except IndexError:
            self.uri_records.append(UriRecord(value))

    @property
    def titles(self):
        """Get a dictionary of all titles with {language: text} items."""
        return dict([(t.language, t.text) for t in self.title_records])

    @property
    def title(self):
        """Get the title string for the language code 'en'. If that is not
        available, the first title string added or decoded is supplied. In
        case that no title string is available the value is None.

        """
        try:
            return self.titles['en']
        except KeyError:
            pass
        try:
            return self.title_records[0].text
        except IndexError:
            return None

    def set_title(self, title, language='en', encoding='UTF-8'):
        """Set the title string for a specific language, default English. The
        transfer encoding may be set to either UTF-8 or UTF-16, it
        defaults to UTF-8 if not specified.

        """
        for r in self.title_records:
            if r.language == language:
                r.text, r.encoding = title, encoding
                break
        else:
            self.title_records.append(TextRecord(title, language, encoding))

    @property
    def action(self):
        """Get or set the recommended action for handling the Smartposter
        resource. A set value may be 'exec', 'save', 'edit' or a list
        index. A read value is either one of the above strings or None
        if no action value was set and no action record decoded.

        """
        try:
            return self.action_records[0].action
        except IndexError:
            return None

    @action.setter
    def action(self, value):
        action_record = ActionRecord(value)
        try:
            self.action_records[0] = action_record
        except IndexError:
            self.action_records.append(action_record)

    @property
    def icon(self):
        """Get the 'image/png' Smartposter icon data. If that is not available
        the data of the first icon decoded or added is returned. If no
        icon was decoded or added then the attribute reads as None.

        """
        try:
            return self.icons['image/png']
        except KeyError:
            pass
        try:
            return self.icon_records[0].data
        except IndexError:
            return None

    @property
    def icons(self):
        """Get a dictionary of all icons with {mime-type: icon-data} items."""
        return dict([(r.type, r.data) for r in self.icon_records])

    def add_icon(self, icon_type, icon_data):
        """Add a Smartposter icon as icon_data bytes for the image or video
        mime-type string supplied with icon_type.

        """
        if icon_type.startswith('image/') or icon_type.startswith('video/'):
            self.icon_records.append(Record(icon_type, None, icon_data))
        else:
            errstr = "expects an image or video icon mimetype, not '{}'"
            raise self._value_error(errstr.format(icon_type))

    @property
    def resource_size(self):
        """Get or set the size hint for the Smartposter resource. This will be
        None if the size hint was not previously set and no SizeRecord
        decoded.

        """
        try:
            return self.size_records[0].resource_size
        except IndexError:
            return None

    @resource_size.setter
    def resource_size(self, value):
        size_record = SizeRecord(value)
        try:
            self.size_records[0] = size_record
        except IndexError:
            self.size_records.append(size_record)

    @property
    def resource_type(self):
        """Get or set the type hint for the Smartposter resource. This will be
        None if the type hint was not previously set and no TypeRecord
        decoded.

        """
        try:
            return self.type_records[0].resource_type
        except IndexError:
            return None

    @resource_type.setter
    def resource_type(self, value):
        type_record = TypeRecord(value)
        try:
            self.type_records[0] = type_record
        except IndexError:
            self.type_records.append(type_record)

    def __format__(self, format_spec):
        if format_spec == 'args':
            s = ("{r.resource.iri!r}, {r.titles!r}, {r.action!r}, "
                 "{r.icons!r}, {r.resource_size!r}, {r.resource_type!r})")
            return s.format(r=self)

        if format_spec == 'data':
            s = ["{r.resource:data}"]
            if self.title:
                s.append("Title '{r.title}'")
            if len(self.icon_records) > 0:
                icon_types = [r.type for r in self.icon_records]
                s.append(" ".join(["Icon '{}'".format(t) for t in icon_types]))
            if self.action:
                s.append("{r.action_records[0]:data}")
            if self.resource_size:
                s.append("{r.size_records[0]:data}")
            if self.resource_type:
                s.append("{r.type_records[0]:data}")
            return ' '.join(s).format(r=self)

        return super(SmartposterRecord, self).__format__(format_spec)

    def _encode_payload(self):
        records = (
            self.uri_records + self.title_records + self.action_records +
            self.icon_records + self.size_records + self.type_records)
        return b''.join(list(message_encoder(records)))

    @classmethod
    def _decode_payload(cls, octets, errors):
        sp_record = cls(None)
        for record in message_decoder(octets, errors, cls._known_types):
            if record.type == 'urn:nfc:wkt:T':
                sp_record.title_records.append(record)
            elif record.type == 'urn:nfc:wkt:U':
                sp_record.uri_records.append(record)
            elif record.type == 'urn:nfc:wkt:act':
                sp_record.action_records.append(record)
            elif record.type.startswith('image/'):
                sp_record.icon_records.append(record)
            elif record.type.startswith('video/'):
                sp_record.icon_records.append(record)
            elif record.type == 'urn:nfc:wkt:s':
                sp_record.size_records.append(record)
            elif record.type == 'urn:nfc:wkt:t':
                sp_record.type_records.append(record)
        if errors == 'strict':
            uri_record_count = len(sp_record.uri_records)
            if uri_record_count != 1:
                errmsg = "payload must contain exactly one URI Record, got {}"
                raise cls._decode_error(errmsg.format(uri_record_count))
        return sp_record


SmartposterRecord.register_type(UriRecord)
SmartposterRecord.register_type(TextRecord)
SmartposterRecord.register_type(SizeRecord)
SmartposterRecord.register_type(TypeRecord)
SmartposterRecord.register_type(ActionRecord)
Record.register_type(SmartposterRecord)

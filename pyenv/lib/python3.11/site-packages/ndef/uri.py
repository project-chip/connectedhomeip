# -*- coding: utf-8 -*-
"""Decoding and encoding of the NDEF URI Record.

The NDEF URI Record is a well-known record type defined by the NFC
Forum. It carries a, potentially abbreviated, UTF-8 encoded
Internationalized Resource Identifier (IRI) as defined by RFC
3987. Abbreviation covers certain prefix patterns that are compactly
encoded as a single octet and automatically expanded when
decoding. The UriRecord class provides both access attributes for
decoded IRI as well as a converted URI (if a netloc part is present in
the IRI).

"""
from __future__ import absolute_import, division
from .record import Record, GlobalRecord, convert, _PY2

if _PY2:  # pragma: no cover
    from urlparse import urlsplit, urlunsplit
    from urllib import quote as _quote, unquote
else:  # pragma: no cover
    from urllib.parse import urlsplit, urlunsplit, quote as _quote, unquote


def quote(string):
    # RFC 3986 includes "~" in the set of reserved characters.
    return _quote(string, safe='/~')


class UriRecord(GlobalRecord):
    """The UriRecord class decodes or encodes an NDEF URI Record. The
    UriRecord.iri attribute holds the expanded (if a valid
    abbreviation code was decoded) internationalized resource
    identifier. The UriRecord.uri attribute is a converted version of
    the IRI. Conversion is applied only for IRI's that split with a
    netloc component. A converted URI contains only ASCII characters
    with an IDNA encoded netloc component and percent-encoded path,
    query and fragment components.

    >>> uri_record = ndef.UriRecord("http://www.hääyö.com/~user/")
    >>> uri_record.iri
    'http://www.hääyö.com/~user/'
    >>> uri_record.uri
    'http://www.xn--hy-viaa5g.com/%7Euser/'

    """
    _type = 'urn:nfc:wkt:U'
    _prefix_strings = (
        "", "http://www.", "https://www.", "http://", "https://", "tel:",
        "mailto:", "ftp://anonymous:anonymous@", "ftp://ftp.", "ftps://",
        "sftp://", "smb://", "nfs://", "ftp://", "dav://", "news:",
        "telnet://", "imap:", "rtsp://", "urn:", "pop:", "sip:", "sips:",
        "tftp:", "btspp://", "btl2cap://", "btgoep://", "tcpobex://",
        "irdaobex://", "file://", "urn:epc:id:", "urn:epc:tag:",
        "urn:epc:pat:", "urn:epc:raw:", "urn:epc:", "urn:nfc:")

    def __init__(self, iri=None):
        """Initialize the record with an internationalize resource identifier
        (IRI) string argument. The default value is an empty string.

        """
        self.iri = iri if iri is not None else ''

    @property
    def iri(self):
        """The internationalized resource identifier."""
        return self._iri

    @iri.setter
    @convert('value_to_unicode')
    def iri(self, value):
        self._iri = value

    @property
    def uri(self):
        """The uniform resource identifier."""
        scheme, netloc, path, query, fragment = urlsplit(self._iri)
        if netloc:
            netloc = netloc.encode('idna').decode()
            path, query, fragment = map(quote, [path, query, fragment])
        return urlunsplit((scheme, netloc, path, query, fragment))

    @uri.setter
    @convert('value_to_unicode')
    def uri(self, value):
        scheme, netloc, path, query, fragment = urlsplit(value)
        if netloc:
            netloc = netloc.encode().decode('idna')
            path, query, fragment = map(unquote, [path, query, fragment])
        self._iri = urlunsplit((scheme, netloc, path, query, fragment))

    def __format__(self, format_spec):
        if format_spec == 'args':
            return "{!r}".format(self.iri)

        if format_spec == 'data':
            return "Resource '{r.iri}'".format(r=self)

        return super(UriRecord, self).__format__(format_spec)

    def _encode_payload(self):
        # Called from Record._encode when the byte representation of
        # the NDEF URI Record PAYLOAD is required for Record.data.
        for prefix in sorted(self._prefix_strings, reverse=True):
            if prefix and self.iri.startswith(prefix):
                index = self._prefix_strings.index(prefix)
                URI_CODE = self._encode_struct('B', index)
                URI_DATA = self.iri[len(prefix):].encode('utf-8')
                return URI_CODE + URI_DATA
        else:
            return self._encode_struct('B*', 0, self.iri.encode('utf-8'))

    _decode_min_payload_length = 1

    @classmethod
    def _decode_payload(cls, octets, errors):
        # Called from Record._decode with the PAYLOAD of an NDEF URI
        # Record. Returns a new UriRecord instance initialized with
        # the decoded data fields. Raises a DecodeError if any of the
        # decoding steps failed. Undefined abbreviation identifier
        # codes map raise DecodeError only for strict error handling,
        # otherwise map to code zero (no prefix).
        URI_CODE, URI_DATA = cls._decode_struct('B*', octets)

        if not URI_CODE < len(cls._prefix_strings) and errors == 'strict':
            errmsg = "decoding of URI identifier {} is not defined"
            raise cls._decode_error(errmsg.format(URI_CODE))

        uri_code = URI_CODE if URI_CODE < len(cls._prefix_strings) else 0
        try:
            uri_data = URI_DATA.decode('utf-8')
        except UnicodeDecodeError:
            raise cls._decode_error("URI field is not valid UTF-8 data")

        if any([ord(char) <= 31 for char in uri_data]):
            raise cls._decode_error("URI field contains invalid characters")

        return cls(cls._prefix_strings[uri_code] + uri_data)


Record.register_type(UriRecord)

# -*- coding: utf-8 -*-
"""Decoding and encoding of the NDEF Signature Record.

The NDEF Signature Record is a well-known record type defined by the NFC
Forum. It contains three fields: a version field, a signature field and a
certificate field.

The version field is static. Currently this implementation only supports v2.0
of the NDEF Signature Record.

The signature field contains the signature type, the message hash type and
either the signature itself or a URI to the signature.

The certificate field contains the certificate format type, a certificate chain
store and an option URI to the next certificate in the chain.

"""
from __future__ import absolute_import, division
from .record import Record, GlobalRecord, convert
from collections import namedtuple

VersionTuple = namedtuple('Version', 'major, minor')


class SignatureRecord(GlobalRecord):
    """The SignatureRecord class decodes or encodes an NDEF Signature Record.

    This is default usage:

    >>> import ndef
    >>> signature_record = ndef.SignatureRecord(
    ... None, 'SHA-256', b'', '', 'X.509', [], '')

    This is a full example creating records, signing them and verifying them:

    >>> import ndef
    >>> import io
    >>> from cryptography.hazmat.backends import default_backend
    >>> from cryptography.hazmat.primitives import hashes
    >>> from cryptography.hazmat.primitives.asymmetric import ec
    >>> from cryptography.hazmat.primitives.asymmetric import utils
    >>> from cryptography.exceptions import InvalidSignature
    >>> from asn1crypto.algos import DSASignature

    >>> private_key = ec.generate_private_key(
    ... ec.SECP256K1(), default_backend())
    >>> public_key = private_key.public_key()

    >>> r1 = ndef.UriRecord("https://example.com")
    >>> r2 = ndef.TextRecord("TEST")

    >>> stream = io.BytesIO()
    >>> records = [r1, r2, ndef.SignatureRecord("ECDSA-P256", "SHA-256")]
    >>> encoder = ndef.message_encoder(records, stream)
    >>> for _ in range(len(records) - 1): next(encoder)

    >>> signature = private_key.sign(
    ... stream.getvalue(), ec.ECDSA(hashes.SHA256()))
    >>> records[-1].signature = DSASignature.load(
    ... signature, strict=True).to_p1363()
    >>> next(encoder)
    >>> octets = stream.getvalue()

    >>> records_verified = []
    >>> records_to_verify = []
    >>> known_types = {'urn:nfc:wkt:Sig': ndef.signature.SignatureRecord}
    >>> for record in ndef.message_decoder(octets, known_types=known_types):
    ...     if not record.type == 'urn:nfc:wkt:Sig':
    ...         records_to_verify.append(record)
    ...     else:
    ...         stream_to_verify = io.BytesIO()
    ...         encoder_to_verify = ndef.message_encoder(
    ...         records_to_verify + [record], stream_to_verify)
    ...         for _ in range(len(records_to_verify)): next(encoder_to_verify)
    ...         try:
    ...             public_key.verify(DSASignature.from_p1363(
    ...             record.signature).dump(), stream_to_verify.getvalue(
    ...             ), ec.ECDSA(hashes.SHA256()))
    ...             records_verified.extend(records_to_verify)
    ...             records_to_verify = []
    ...         except InvalidSignature:
    ...             pass

    >>> records_verified = list(ndef.message_decoder(b''.join(
    ... ndef.message_encoder(records_verified))))

    """
    _type = 'urn:nfc:wkt:Sig'
    _version = 0x20  # this class implements v2.0 of the Signature RTD
    _mapping_signature_type = (
        (0x00, None),
        (0x01, "RSASSA-PSS-1024"),
        (0x02, "RSASSA-PKCS1-v1_5-1024"),
        (0x03, "DSA-1024"),
        (0x04, "ECDSA-P192"),
        (0x05, "RSASSA-PSS-2048"),
        (0x06, "RSASSA-PKCS1-v1_5-2048"),
        (0x07, "DSA-2048"),
        (0x08, "ECDSA-P224"),
        (0x09, "ECDSA-K233"),
        (0x0a, "ECDSA-B233"),
        (0x0b, "ECDSA-P256"),
    )
    _mapping_hash_type = (
        (0x02, "SHA-256"),
    )
    _mapping_certificate_format = (
        (0x00, "X.509"),
        (0x01, "M2M"),
    )

    def __init__(self, signature_type=None, hash_type=None, signature=None,
                 signature_uri=None, certificate_format=None,
                 certificate_store=None, certificate_uri=None):
        """Initialize the record with signature type, hash type, signature,
        signature URI, certificate format, certificate store and/or certificate
        URI. All parameters are optional. The default value is an empty
        Signature record.

        """
        self.signature_type = signature_type
        self.hash_type = hash_type if hash_type is not None else 'SHA-256'
        self.signature = signature if signature is not None else b''
        self.signature_uri = signature_uri if signature_uri is not None else ''
        if certificate_format is not None:
            self.certificate_format = certificate_format
        else:
            self.certificate_format = 'X.509'
        self._certificate_store = []
        if isinstance(certificate_store, list):
            for certificate in certificate_store:
                self.add_certificate_to_store(certificate)
        if certificate_uri is not None:
            self.certificate_uri = certificate_uri
        else:
            self.certificate_uri = ''

    @property
    def version(self):
        """Signature Record: Version Field (Version)"""
        return VersionTuple(self._version >> 4, self._version & 0x0F)

    @property
    def signature_type(self):
        """Signature Record: Signature Field (Signature Type)"""
        return self._get_name_signature_type(self._signature_type)

    @signature_type.setter
    def signature_type(self, value):
        self._signature_type = self._get_enum_signature_type(value)

    def _get_enum_signature_type(self, value):
        for enum, name in self._mapping_signature_type:
            if value == name:
                return enum
        errstr = ("{!r} does not have a known "
                  "Signature Type mapping").format(value)
        raise self._value_error(errstr)

    def _get_name_signature_type(self, value):
        for enum, name in self._mapping_signature_type:
            if value == enum:
                return name

    @property
    def hash_type(self):
        """Signature Record: Signature Field (Hash Type)"""
        return self._get_name_hash_type(self._hash_type)

    @hash_type.setter
    def hash_type(self, value):
        self._hash_type = self._get_enum_hash_type(value)

    def _get_enum_hash_type(self, value):
        for enum, name in self._mapping_hash_type:
            if value == name:
                return enum
        errstr = "{!r} does not have a known Hash Type mapping".format(value)
        raise self._value_error(errstr)

    def _get_name_hash_type(self, value):
        for enum, name in self._mapping_hash_type:
            if value == enum:
                return name

    @property
    def signature(self):
        """Signature Record: Signature Field (Signature)"""
        return self._signature

    @signature.setter
    def signature(self, value):
        if not isinstance(value, (bytes, bytearray)):
            errstr = "signature may be bytes or bytearray, but not '{}'"
            raise self._value_error(errstr, type(value).__name__)
        if len(value) >= 2**16:
            errstr = "signature cannot be more than 2^16 octets, got {}"
            raise self._value_error(errstr, len(value))
        if value and hasattr(self, '_signature_uri') and self._signature_uri:
            errstr = "cannot set both signature and signature_uri"
            raise self._value_error(errstr)
        self._signature = value

    @property
    def signature_uri(self):
        """Signature Record: Signature Field (Signature URI)"""
        return self._signature_uri

    @signature_uri.setter
    @convert('value_to_unicode')
    def signature_uri(self, value):
        if len(value) >= 2**16:
            errstr = "signature_uri cannot be more than 2^16 octets, got {}"
            raise self._value_error(errstr, len(value))
        if value and hasattr(self, '_signature') and self._signature:
            errstr = "cannot set both signature and signature_uri"
            raise self._value_error(errstr)
        self._signature_uri = value

    @property
    def certificate_format(self):
        """Signature Record: Certificate Chain Field (Certificate Format)"""
        return self._get_name_certificate_format(self._certificate_format)

    @certificate_format.setter
    def certificate_format(self, value):
        self._certificate_format = self._get_enum_certificate_format(value)

    def _get_enum_certificate_format(self, value):
        for enum, name in self._mapping_certificate_format:
            if value == name:
                return enum
        errstr = ("{!r} does not have a known "
                  "Certificate Format mapping").format(value)
        raise self._value_error(errstr)

    def _get_name_certificate_format(self, value):
        for enum, name in self._mapping_certificate_format:
            if value == enum:
                return name

    @property
    def certificate_store(self):
        """Signature Record: Certificate Chain Field (Certificate Store)"""
        return self._certificate_store

    def add_certificate_to_store(self, value):
        if not isinstance(value, (bytes, bytearray)):
            errstr = "certificate may be bytes or bytearray, but not '{}'"
            raise self._value_error(errstr, type(value).__name__)
        if len(value) >= 2**16:
            errstr = "certificate cannot be more than 2^16 octets, got {}"
            raise self._value_error(errstr, len(value))
        if len(self._certificate_store)+1 >= 2**4:
            errstr = ("certificate store cannot hold "
                      "more than 2^4 certificates, got {}")
            raise self._value_error(errstr, len(self._certificate_store)+1)
        self._certificate_store.append(value)

    @property
    def certificate_uri(self):
        """Signature Record: Certificate Chain Field (Certificate URI)"""
        return self._certificate_uri

    @certificate_uri.setter
    @convert('value_to_unicode')
    def certificate_uri(self, value):
        self._certificate_uri = value

    def __format__(self, format_spec):
        if format_spec == 'args':
            s = ("{r.signature_type!r}, {r.hash_type!r}, {r.signature!r}, "
                 "{r.signature_uri!r}, {r.certificate_format}, "
                 "{r.certificate_store!r}, {r.certificate_uri!r}")
            return s.format(r=self)

        if format_spec == 'data':
            s = ["Signature RTD '{r.version}'"]
            if self.signature:
                s.append("Signature Type '{r.signature_type}'")
                s.append("Hash Type '{r.hash_type}'")
            if self.signature_uri:
                s.append("Signature URI '{r.signature_uri}'")
            if self.certificate_store:
                s.append("Certificate Format '{r.certificate_format}'")
            if self.certificate_uri:
                s.append("Certificate URI '{r.certificate_uri}'")
            return ' '.join(s).format(r=self)

        return super(SignatureRecord, self).__format__(format_spec)

    def _encode_payload(self):

        # Version Field
        VERSION = self._encode_struct('B', self._version)

        # Signature Field
        SUP = 0b10000000 if self.signature_uri else 0
        SST = self._signature_type & 0b01111111
        SHT = self._hash_type
        if self.signature_uri:
            SIGURI = self.signature_uri.encode('utf-8')
        else:
            SIGURI = self.signature
        SIGNATURE = self._encode_struct('BBH+', SUP | SST, SHT, SIGURI)

        # Certificate Field
        CUP = 0b10000000 if len(self.certificate_uri) else 0
        CCF = (self._certificate_format << 4) & 0b01110000
        CNC = len(self.certificate_store) & 0b00001111
        CST = b''
        for certificate in self._certificate_store:
            CST += self._encode_struct('H+', certificate)
        CERTIFICATE = self._encode_struct('B', CUP | CCF | CNC)
        if len(CST):
            CERTIFICATE += self._encode_struct(str(len(CST))+'s', CST)
        if len(self.certificate_uri):
            CERTURI = self._certificate_uri.encode('utf-8')
            CERTIFICATE += self._encode_struct('H+', CERTURI)

        return VERSION + SIGNATURE + CERTIFICATE

    _decode_min_payload_length = 7

    @classmethod
    def _decode_payload(cls, octets, errors):
        # Called from Record._decode with the PAYLOAD of an NDEF Signature
        # Record. Returns a new SignatureRecord instance initialized with
        # the decoded data fields. Raises a DecodeError if any of the
        # decoding steps failed.

        (VERSION, SUP_SST, SHT, SIGURI,
         CUP_CCF_CNC, CST_CERTURI) = cls._decode_struct('BBBH+B*', octets)

        # Version Field
        if not VERSION == cls._version and errors == 'strict':
            errmsg = "decoding of version {} is not supported"
            raise cls._decode_error(errmsg.format(VERSION))

        # Signature Field
        signature_uri_present = SUP_SST & 0b10000000
        signature_type = cls._get_name_signature_type(
                         SignatureRecord(), SUP_SST & 0b01111111)
        hash_type = cls._get_name_hash_type(SignatureRecord(), SHT)
        if signature_uri_present:
            signature = None
            try:
                signature_uri = SIGURI.decode('utf-8')
            except UnicodeDecodeError:
                raise cls._decode_error("Signature URI field is "
                                        "not valid UTF-8 data")
            if any([ord(char) <= 31 for char in signature_uri]):
                raise cls._decode_error("Signature URI field contains "
                                        "invalid characters")
        else:
            signature_uri = None
            signature = SIGURI

        # Certificate Field
        certificate_uri_present = CUP_CCF_CNC & 0b10000000
        certificate_format = cls._get_name_certificate_format(
            SignatureRecord(), (CUP_CCF_CNC & 0b01110000) >> 4)
        certificate_number_of_certificates = CUP_CCF_CNC & 0b00001111
        certificate_store = []
        for certificate_number in range(certificate_number_of_certificates):
            certificate, CST_CERTURI = cls._decode_struct('H+*', CST_CERTURI)
            certificate_store.append(certificate)
        if certificate_uri_present:
            CERTURI = cls._decode_struct('H+', CST_CERTURI)
            try:
                certificate_uri = CERTURI.decode('utf-8')
            except UnicodeDecodeError:
                raise cls._decode_error("Certificate URI field is "
                                        "not valid UTF-8 data")
            if any([ord(char) <= 31 for char in certificate_uri]):
                raise cls._decode_error("Certificate URI field contains "
                                        "invalid characters")
        else:
            certificate_uri = None

        return cls(signature_type, hash_type, signature, signature_uri,
                   certificate_format, certificate_store, certificate_uri)


Record.register_type(SignatureRecord)

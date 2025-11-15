# -*- coding: utf-8 -*-
"""Implementation of the message decoder and encoder generator functions.


"""
from __future__ import absolute_import, division

import io
from .record import Record, DecodeError


def message_decoder(stream_or_bytes, errors='strict',
                    known_types=Record._known_types):
    """The message_decoder generator function yields ndef.Record class or
    subclass instances from an encoded NDEF Message. The NDEF Message
    octets can be read either from a file-like, byte-oriented stream
    or from bytes or a bytearray.

    >>> import io
    >>> from ndef import message_decoder
    >>> hexstr = '900000100000500000'
    >>> octets = bytearray.fromhex(hexstr)
    >>> stream = io.BytesIO(octets)
    >>> list(message_decoder(octets)) == list(message_decoder(stream))
    True
    >>> for record in message_decoder(octets):
    ...     print(record)
    ...
    NDEF Record TYPE '' ID '' PAYLOAD 0 byte
    NDEF Record TYPE '' ID '' PAYLOAD 0 byte
    NDEF Record TYPE '' ID '' PAYLOAD 0 byte

    A decoding error will result in an ndef.DecodeError exception.  By
    default, errors are handled 'strict'. Minor errors, such as a
    missing message begin or end flag, will pass unnoticed if the
    errors argument is set to 'relax'. No exception is raised when
    errors is set to 'ignore' but the decoded records may not
    represent the complete message.

    The known_types argument, if supplied, must be a mapping of record
    type names to record classes. By default, if known_types is None,
    all registered record types are recognized.

    """
    if isinstance(stream_or_bytes, (io.RawIOBase, io.BufferedIOBase)):
        stream = stream_or_bytes
    elif isinstance(stream_or_bytes, (bytes, bytearray)):
        stream = io.BytesIO(stream_or_bytes)
    else:
        errstr = "a stream or bytes type argument is required, not {}"
        raise TypeError(errstr.format(type(stream_or_bytes).__name__))

    try:
        record, mb, me, cf = Record._decode(stream, errors, known_types)
    except DecodeError:
        if errors == 'ignore':
            return  # just stop decoding
        raise

    if record is not None and mb is False and errors == 'strict':
        raise DecodeError('MB flag not set in first record')

    if record is not None and known_types is Record._known_types:
        known_types = type(record)._known_types

    while record is not None:
        yield record
        if me is True:
            if cf is True and errors == 'strict':
                raise DecodeError('CF flag set in last record')
            record = None
        else:
            try:
                record, mb, me, cf = Record._decode(stream, errors,
                                                    known_types)
            except DecodeError:
                if errors == 'ignore':
                    return  # just stop decoding
                raise
            else:
                if record is None and errors == 'strict':
                    raise DecodeError('ME flag not set in last record')
                if mb is True and errors == 'strict':
                    raise DecodeError('MB flag set in middle record')


def message_encoder(message=None, stream=None):
    """The message_encoder generator function generates the encoded
    representation of an NDEF Message. The message argument is the
    iterable of ndef.record.Record class or subclass objects that
    shall be encoded. If a stream argument is supplied, encoded octets
    are written into the file-like, byte-oriented stream and the
    number of octets yielded for each record. If the stream argument
    is None, octets are yielded as bytes type for each record.

    >>> from ndef.message import message_encoder
    >>> from ndef.record import Record
    >>> import io
    >>> ostream = io.BytesIO()
    >>> message = [Record(), Record(), Record()]
    >>> list(message_encoder(message, ostream))
    [3, 3, 3]
    >>> ostream.getvalue().hex()
    '900000100000500000'
    >>> b''.join((message_encoder(message))).hex()
    '900000100000500000'

    If the message argument is None, the encoder expects client code
    to successively send() the records for encoding. As for all
    generators, the first value must be None. For the next value,
    which must then be a record object, the message encoder still
    yields None but for all further records the encoder yields the
    octets for the previous record. When the last record was send to
    the encoder, it requires a further None value to retrieve the
    octets for the final record.

    >>> from ndef.message import message_encoder
    >>> from ndef.record import Record
    >>> encoder = message_encoder()
    >>> results = list()
    >>> encoder.send(None)
    >>> encoder.send(Record())
    >>> results.append(encoder.send(Record()))
    >>> results.append(encoder.send(Record()))
    >>> results.append(encoder.send(None))
    >>> b''.join(results).hex()
    '900000100000500000'

    """
    encoder = _message_encoder(stream)
    if message is None:
        record = None
        while True:
            try:
                record = yield (encoder.send(record))
            except StopIteration:
                return
    else:
        itermsg = iter(message)
        encoder.send(None)
        try:
            encoder.send(next(itermsg))
        except StopIteration:
            return
        for record in itermsg:
            yield encoder.send(record)
        yield encoder.send(None)


def _message_encoder(stream):
    mb_flag = True
    this_record = yield
    next_record = yield
    while this_record:
        if not isinstance(this_record, Record):
            errstr = "an ndef.Record class instance is required, not {}"
            raise TypeError(errstr.format(type(this_record).__name__))
        me_flag = next_record is None
        cf_flag = not me_flag and next_record.type == 'unchanged'
        this_result = this_record._encode(mb_flag, me_flag, cf_flag, stream)
        this_record = next_record
        next_record = (yield this_result)
        mb_flag = False

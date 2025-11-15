from construct import *
from construct.lib import *
import binascii


def integer2bits(number, width, signed=False):
    r"""
    Converts an integer into its binary representation in a bit-string. Width is the amount of bits to generate. Each bit is represented as either \\x00 or \\x01. The most significant bit is first, big-endian. This is reverse to `bits2integer`.

    Examples:

        >>> integer2bits(19, 8)
        b'\x00\x00\x00\x01\x00\x00\x01\x01'
    """
    if not width >= 1:
        raise ValueError(f"width {width} must be positive")

    if signed:
        min = -(2 ** width // 2)
        max = 2 ** width // 2 - 1
    else:
        min = 0
        max = 2 ** width - 1
    if not min <= number <= max:
        raise ValueError(f"number {number} is out of range (min={min}, max={max})")

    if number < 0:
        number += 1 << width
    bits = bytearray(width)
    i = width - 1
    while number and i >= 0:
        bits[i] = number & 1
        number >>= 1
        i -= 1
    return bytes(bits)


def integer2bytes(number, width, signed=False):
    r"""
    Converts an integer into a byte-string. This is reverse to `bytes2integer`.

    Examples:

        >>> integer2bytes(19, 4)
        '\x00\x00\x00\x13'
    """
    # pypy does not check this in int.to_bytes, lazy fuckers
    if not width >= 1:
        raise ValueError(f"width {width} must be positive")

    try:
        return int.to_bytes(number, width, 'big', signed=signed)
    except OverflowError:
        raise ValueError(f"number {number} does not fit width {width}, signed {signed}")


def bits2integer(data, signed=False):
    r"""
    Converts a bit-string into an integer. Set signed to interpret the number as a 2-s complement signed integer. This is reverse to `integer2bits`.

    Examples:

        >>> bits2integer(b"\x01\x00\x00\x01\x01")
        19
    """
    if data == b"":
        raise ValueError("bit-string cannot be empty")

    number = 0
    for b in data:
        number = (number << 1) | b

    if signed and data[0]:
        bias = 1 << len(data)
        return number - bias
    else:
        return number


def bytes2integer(data, signed=False):
    r"""
    Converts a byte-string into an integer. This is reverse to `integer2bytes`.

    Examples:

        >>> bytes2integer(b'\x00\x00\x00\x13')
        19
    """
    if data == b"":
        raise ValueError("byte-string cannot be empty")

    return int.from_bytes(data, 'big', signed=signed)


BYTES2BITS_CACHE = {i:integer2bits(i,8) for i in range(256)}
def bytes2bits(data):
    r""" 
    Converts between bit-string and byte-string representations, both as bytes type.

    Example:

        >>> bytes2bits(b'ab')
        b"\x00\x01\x01\x00\x00\x00\x00\x01\x00\x01\x01\x00\x00\x00\x01\x00"
    """
    return b"".join(BYTES2BITS_CACHE[b] for b in data)


BITS2BYTES_CACHE = {bytes2bits(int2byte(i)):i for i in range(256)}
def bits2bytes(data):
    r""" 
    Converts between bit-string and byte-string representations, both as bytes type. Its length must be multiple of 8.

    Example:

        >>> bits2bytes(b"\x00\x01\x01\x00\x00\x00\x00\x01\x00\x01\x01\x00\x00\x00\x01\x00")
        b'ab'
    """
    if len(data) % 8 != 0:
        raise ValueError(f"data length {len(data)} must be a multiple of 8")
    return bytes(BITS2BYTES_CACHE[data[i:i+8]] for i in range(0,len(data),8))


def swapbytes(data):
    r"""
    Performs an endianness swap on byte-string.

    Example:

        >>> swapbytes(b'abcd')
        b'dcba'
    """
    return data[::-1]


def swapbytesinbits(data):
    r"""
    Performs an byte-swap within a bit-string. Its length must be multiple of 8.

    Example:

        >>> swapbytesinbits(b'0000000011111111')
        b'1111111100000000'
    """
    if len(data) % 8 != 0:
        raise ValueError(f"little-endianness is only defined if data length {len(data)} is multiple of 8")
    return b"".join(data[i:i+8] for i in reversed(range(0,len(data),8)))


SWAPBITSINBYTES_CACHE = {i:byte2int(bits2bytes(swapbytes(bytes2bits(int2byte(i))))) for i in range(256)}
def swapbitsinbytes(data):
    r"""
    Performs a bit-reversal on each byte within a byte-string.

    Example:

        >>> swapbitsinbytes(b"\xf0\x00")
        b"\x0f\x00"
    """
    return bytes(SWAPBITSINBYTES_CACHE[b] for b in data)


def hexlify(data):
    """Returns binascii.hexlify(data)."""
    return binascii.hexlify(data)


def unhexlify(data):
    """Returns binascii.unhexlify(data)."""
    return binascii.unhexlify(data)

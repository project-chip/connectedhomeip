from construct.lib.py3compat import *
import binascii


class HexDisplayedInteger(integertypes[0]):
    """Used internally."""
    def __str__(self):
        return "0x" + format(self, self.fmtstr).upper()

    @staticmethod
    def new(intvalue, fmtstr):
        obj = HexDisplayedInteger(intvalue)
        obj.fmtstr = fmtstr
        return obj

class HexDisplayedBytes(bytestringtype):
    """Used internally."""
    def __str__(self):
        if not hasattr(self, "render"):
            self.render = "unhexlify(%s)" % (trimstring(binascii.hexlify(self)), )
        return self.render

class HexDisplayedDict(dict):
    """Used internally."""
    def __str__(self):
        if not hasattr(self, "render"):
            self.render = "unhexlify(%s)" % (trimstring(binascii.hexlify(self["data"])), )
        return self.render

class HexDumpDisplayedBytes(bytestringtype):
    """Used internally."""
    def __str__(self):
        if not hasattr(self, "render"):
            self.render = hexdump(self, 16)
        return self.render

class HexDumpDisplayedDict(dict):
    """Used internally."""
    def __str__(self):
        if not hasattr(self, "render"):
            self.render = hexdump(self["data"], 16)
        return self.render


# Map an integer in the inclusive range 0-255 to its string byte representation
PRINTABLE = [bytes2str(int2byte(i)) if 32 <= i < 128 else '.' for i in range(256)]
HEXPRINT = [format(i, '02X') for i in range(256)]


def hexdump(data, linesize):
    r"""
    Turns bytes into a unicode string of the format:

    ::

        >>>print(hexdump(b'0' * 100, 16))
        hexundump(\"\"\"
        0000   30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30   0000000000000000
        0010   30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30   0000000000000000
        0020   30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30   0000000000000000
        0030   30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30   0000000000000000
        0040   30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30   0000000000000000
        0050   30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30   0000000000000000
        0060   30 30 30 30                                       0000
        \"\"\")
    """
    if len(data) < 16**4:
        fmt = "%%04X   %%-%ds   %%s" % (3*linesize-1,)
    elif len(data) < 16**8:
        fmt = "%%08X   %%-%ds   %%s" % (3*linesize-1,)
    else:
        raise ValueError("hexdump cannot process more than 16**8 or 4294967296 bytes")
    prettylines = []
    prettylines.append('hexundump("""')
    for i in range(0, len(data), linesize):
        line = data[i:i+linesize]
        hextext = " ".join(HEXPRINT[b] for b in line)
        rawtext = "".join(PRINTABLE[b] for b in line)
        prettylines.append(fmt % (i, str(hextext), str(rawtext)))
    prettylines.append('""")')
    prettylines.append("")
    return "\n".join(prettylines)


def hexundump(data, linesize):
    r"""
    Reverse of `hexdump`.
    """
    raw = []
    for line in data.split("\n")[1:-2]:
        line = line[line.find(" "):].lstrip()
        bytes = [int2byte(int(s,16)) for s in line[:3*linesize].split()]
        raw.extend(bytes)
    return b"".join(raw)

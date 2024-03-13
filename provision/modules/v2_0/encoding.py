from enum import Enum
from modules.parameters import Types, Formats, Parameter
import modules.util as _util


# Since Python automatically allicates the buffer,
# limit and spare are available only if a max size
# is provided, otherwise they are None
#
# begin            out             in               end
#   |---------------v---------------v----------------|
#   |.....offset....|......left.....|.....spare......|
#   |..............size.............|
#   |......................limit.....................|
#
class Buffer(object):

    def __init__(self, limit = None, data = None):
        self.max_size = limit
        self.data = (data is not None) and data or bytearray()
        self.out = 0

    def clear(self):
        self.data = bytearray()
        self.out = 0
    def limit(self):  return self.max_size
    def size(self):   return len(self.data)
    def offset(self): return self.out
    def left(self):   return self.size() - self.offset()
    def spare(self):
        if self.limit() is None: return None
        else: return self.limit() - self.size()
    def serialize(self): return self.data

    @staticmethod
    def encodeInt8u(x):
        return (x is not None) and int(x).to_bytes(1, 'big') or None

    @staticmethod
    def encodeInt16u(x):
        return (x is not None) and int(x).to_bytes(2, 'big') or None

    @staticmethod
    def encodeInt32u(x):
        return (x is not None) and int(x).to_bytes(4, 'big') or None

    @staticmethod
    def encodeBinary(x):
        return (x is not None) and bytearray(x)  or None

    @staticmethod
    def encodeString(x):
        return (x is not None) and x.encode('utf-8')  or None

    @staticmethod
    def decodeInt(x):
        return int.from_bytes(x, byteorder='big')

    def addInt8u(self, x):
        assert((self.spare() is None) or (self.spare() > 0))
        self.data.extend(Buffer.encodeInt8u((x is not None) and x or 0))
        return True

    def addInt16u(self, x):
        assert((self.spare() is None) or (self.spare() >= 2))
        self.data.extend(Buffer.encodeInt16u((x is not None) and x or 0))

    def addInt32u(self, x):
        assert((self.spare() is None) or (self.spare() >= 4))
        self.data.extend(Buffer.encodeInt32u((x is not None) and x or 0))

    def addBinary(self, x):
        if (x is not None) and (len(x) > 0):
            assert((self.spare() is None) or (self.spare() >= len(x)))
            self.data.extend(x)

    def addBuffer(self, x, size):
        if x is not None:
            self.addBinary(x.getBinary(size))

    def getInt8u(self):
        assert(self.left() > 0)
        x = Buffer.decodeInt(self.data[self.out:self.out + 1])
        self.out += 1
        return x

    def getInt16u(self):
        assert(self.left() >= 2)
        x = Buffer.decodeInt(self.data[self.out:self.out + 2])
        self.out += 2
        return x

    def getInt32u(self):
        assert(self.left() >= 2)
        x = Buffer.decodeInt(self.data[self.out:self.out + 4])
        self.out += 4
        return x

    def getInt32s(self):
        x = self.getUint32()
        if x & 0x80000000:
            return -(0x7fffffff & x);
        else:
            return x

    def getBinary(self, size):
        x = self.data[self.out:(self.out + size)]
        self.out += size
        return x

    def getString(self):
        return self.getBinary().decode('utf-8')

    @staticmethod
    def hex(arr):
        return str(len(arr)) + ':' + ' '.join(format(x, '02x') for x in arr)


class States(Enum):
    Flags = 1,
    Size = 2,
    Data = 3,
    Ready = 4,


class Context(Buffer):
    def __init__(self):
        super().__init__()
        self.reset()

    def __str__(self) -> str:
        value = (self.data is not None) and self.data[self.out:].hex() or '!'
        size = (self.data is not None) and len(value) or 0
        return "{}[{}]: ({}) {}".format(self.name, self.out, size, value)

    def reset(self):
        self.clear()
        self.state = States.Flags
        # self.arg = None
        self.id = None
        self.type = Types.BINARY
        self.data_size = None
        self.size_len = None
        self.is_null = False
        self.is_binary = False
        self.is_known = False
        self.value = None


class Coder:
    # Command flags
    kResponseFlag  = 0x80
    # Payload flags
    kCustomIdMin   = 0x0000
    kCustomIdMax   = 0x00ff
    kKnownIdMin    = 0x0100
    kKnownIdMax    = 0x01ff
    kIdMask        = 0x01ff
    kWellKnownMask = 0x0100
    kSizeBit       = 10
    kSizeMask      = 0x0c00
    kFeedbackMask  = 0x0200
    kTypeBit       = 12
    kTypeMask      = 0xf000

    @staticmethod
    def encode(arg, val, feedback = False, out = Buffer()):
        out.clear()
        value = Coder.encodeValue(arg, val)
        size = len(value)
        # ID + Flags
        size_len = Coder.calcSizeLength(size)
        flags  = (arg.id & Coder.kIdMask)
        flags |= (feedback and Coder.kFeedbackMask or 0)
        flags |= ((size_len << Coder.kSizeBit) & Coder.kSizeMask)
        flags |= ((arg.type.value << Coder.kTypeBit) & Coder.kTypeMask)
        out.addInt16u(flags)
        # Size (if binary)
        if (Types.BINARY == arg.type) and (size_len > 0):
            if 1 == size_len:
                out.addInt8u(size)
            elif 2 == size_len:
                out.addInt16u(size)
            else:
                out.addInt32u(size)
        # Value
        if size > 0:
            out.addBinary(value)
        return out.serialize()

    @staticmethod
    def decode(ctx, input):

        while (input.left() > 0) and (States.Ready != ctx.state):

            if States.Flags == ctx.state:
                # ID + flags
                if not Coder.defragment(ctx, input, 2): break
                Coder.decodeFlags(ctx, ctx)
                ctx.state = ctx.is_null and States.Ready or States.Size

            elif States.Size == ctx.state:
                if ctx.is_binary:
                    # Decode variable size
                    if not Coder.defragment(ctx, input, ctx.size_len): break
                    ctx.data_size = Coder.decodeVariableSize(ctx, ctx)
                else:
                    # Fixed-length argument
                    ctx.data_size = Coder.decodeFixedSize(ctx.type)
                ctx.state = States.Data

            elif States.Data == ctx.state:
                assert not ctx.is_null, 'Decode error: Unexpected null'
                assert ctx.data_size > 0, 'Decode error: Non-null with zero-size'
                # De-fragment value
                if not Coder.defragment(ctx, input, ctx.data_size): break
                # Decode value
                ctx.value = Coder.decodeValue(ctx, ctx)
                ctx.state = States.Ready
            else:
                _util.fail("Invalid state: {}".format(ctx.state))

        return States.Ready == ctx.state

    @staticmethod
    def calcSizeLength(size):
        if size < 1:
            return 0
        elif size < 0xff:
            return 1
        elif size < 0xffff:
            return 2
        else:
            return 3

    @staticmethod
    def encodeValue(a, x):
        # print("[{}]: {}".format(a.name, x))
        if x is None: return bytearray()
        if Types.INT8U == a.type:
           return Buffer.encodeInt8u(x)
        elif Types.INT16U == a.type:
            return Buffer.encodeInt16u(x)
        elif Types.INT32U == a.type:
            return Buffer.encodeInt32u(x)
        elif Types.BINARY == a.type:
            if isinstance(x, str):
                return Buffer.encodeString(x)
            elif isinstance(x, bytearray):
                return x
        _util.fail("Endoding: Unsupported type: \"{}\": {} {}".format(a.name, a.type, a.format))

    @staticmethod
    def defragment(ctx, input, size):
        if ctx.left() < size:
            ctx.addBuffer(input, min(input.left(), size - ctx.left()))
        return ctx.left() >= size

    @staticmethod
    def decodeFlags(ctx, input):
        flags         = input.getInt16u()
        ctx.id        = (flags & Coder.kIdMask)
        ctx.type      = Types((flags & Coder.kTypeMask) >> Coder.kTypeBit)
        ctx.size_len  = Coder.decodeSizeLength(flags)
        ctx.data_size      = 0
        ctx.is_null   = (0 == ctx.size_len)
        ctx.is_binary = (Types.BINARY == ctx.type)
        ctx.is_known  = (flags & Coder.kWellKnownMask) > 0

    @staticmethod
    def decodeSizeLength(flags):
        sz = (flags & Coder.kSizeMask) >> Coder.kSizeBit
        if sz < 3:
            return sz
        else:
            return 4

    @staticmethod
    def decodeVariableSize(ctx, input):
        if 1 == ctx.size_len:
            return input.getInt8u()
        elif 2 == ctx.size_len:
            return input.getInt16u()
        else:
            return input.getInt32u()

    @staticmethod
    def decodeFixedSize(t):
        if Types.INT8U == t: return 1
        if Types.INT16U == t: return 2
        if Types.INT32U == t: return 4
        return 0

    @staticmethod
    def decodeValue(ctx, input):
        if Types.INT8U == ctx.type:
            return input.getInt8u()
        elif Types.INT16U == ctx.type:
            return input.getInt16u()
        elif Types.INT32U == ctx.type:
            return input.getInt32u()
        else:
            return input.getBinary(ctx.data_size)
        # return Coder.validate(ctx, value)

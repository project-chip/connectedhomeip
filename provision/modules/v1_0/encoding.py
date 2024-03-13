from enum import Enum


class Types(Enum):
    NONE  = 0x00
    INT8U   = 0x01
    INT16U  = 0x02
    INT32U  = 0x03
    ARRAY  = 0x04


class Encoder(object):

    def __init__(self):
        self.data = bytearray()
        self.offset = 0


    def reset(self, data):
        self.data = data
        self.offset = 0


    def addType(self, x):
        b = int(x.value).to_bytes(1, 'big')
        self.data.extend(b)

    def getType(self):
        t = int.from_bytes(self.data[self.offset:self.offset + 1], byteorder='big')
        self.offset = self.offset + 1
        return Types(t)


    def addInt8u(self, x):
        if x is None: x = 0
        self.addType(Types.INT8U)
        self.data.extend(int(x).to_bytes(1, 'big'))

    def getInt8u(self):
        assert(Types.INT8U == self.getType())
        x = int.from_bytes(self.data[self.offset:self.offset + 1], byteorder='big')
        self.offset = self.offset + 1
        return x


    def addInt16u(self, x):
        if x is None: x = 0
        self.addType(Types.INT16U)
        self.data.extend(int(x).to_bytes(2, 'big'))

    def getInt16u(self):
        assert(Types.INT16U == self.getType())
        x = int.from_bytes(self.data[self.offset:self.offset + 2], byteorder='big')
        self.offset = self.offset + 2
        return x


    def addInt32u(self, x):
        if x is None: x = 0
        self.addType(Types.INT32U)
        self.data.extend(int(x).to_bytes(4, 'big'))

    def getInt32u(self):
        assert(Types.INT32U == self.getType())
        x = int.from_bytes(self.data[self.offset:self.offset + 4], byteorder='big')
        self.offset = self.offset + 4
        return x

    def getInt32(self):
        x = self.getInt32u()
        if x & 0x80000000:
            return -(0x7fffffff & x);
        else:
            return x

    def addArray(self, x):
        if x is None: x = bytearray()
        if isinstance(x, str): x = x.encode('utf-8')
        self.addType(Types.ARRAY)
        self.data.extend(int(len(x)).to_bytes(2, 'big'))
        self.data.extend(x)

    def getArray(self):
        assert(Types.ARRAY == self.getType())
        z = int.from_bytes(self.data[self.offset:self.offset + 2], byteorder='big')
        a = self.data[self.offset + 2:self.offset + 2 + z]
        self.offset += (2 + z)
        return bytearray(a)


    def addString(self, x):
        if x is None:
            return self.addArray(x)
        else:
            return self.addArray(x.encode('utf-8'))

    def getString(self):
        a = self.getArray()
        return a.decode('utf-8')

    def serialize(self):
        return self.data

    @staticmethod
    def hex(arr):
        return str(len(arr)) + ':' + ' '.join(format(x, '02x') for x in arr)

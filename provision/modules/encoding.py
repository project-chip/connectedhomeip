
class Encoder(object):
    UINT8 = 1
    UINT16 = 2
    UINT32 = 3
    ARRAY = 4

    def __init__(self, data = None):
        if data is None:
            self.data = bytearray()
        else:
            self.data = data
        self.offset = 0


    def addType(self, x):
        self.data.extend(int(x).to_bytes(1, 'big'))

    def getType(self):
        t = int.from_bytes(self.data[self.offset:self.offset + 1], byteorder='big')
        self.offset = self.offset + 1
        return t


    def addUint8(self, x):
        self.addType(Encoder.UINT8)
        self.data.extend(int(x).to_bytes(1, 'big'))

    def getUint8(self):
        assert(Encoder.UINT8 == self.getType())
        x = int.from_bytes(self.data[self.offset:self.offset + 1], byteorder='big')
        self.offset = self.offset + 1
        return x


    def addUint16(self, x):
        self.addType(Encoder.UINT16)
        self.data.extend(int(x).to_bytes(2, 'big'))

    def getUint16(self):
        assert(Encoder.UINT16 == self.getType())
        x = int.from_bytes(self.data[self.offset:self.offset + 2], byteorder='big')
        self.offset = self.offset + 2
        return x


    def addUint32(self, x):
        self.addType(Encoder.UINT32)
        self.data.extend(int(x).to_bytes(4, 'big'))

    def getUint32(self):
        assert(Encoder.UINT32 == self.getType())
        x = int.from_bytes(self.data[self.offset:self.offset + 4], byteorder='big')
        self.offset = self.offset + 4
        return x

    def getInt32(self):
        x = self.getUint32()
        if x & 0x80000000:
            return -(0x7fffffff & x);
        else:
            return x

    def addArray(self, x):
        # a = x.encode()
        if x is None:
            x = bytearray()
        self.addType(Encoder.ARRAY)
        self.data.extend(int(len(x)).to_bytes(2, 'big'))
        self.data.extend(x)

    def getArray(self):
        assert(Encoder.ARRAY == self.getType())
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

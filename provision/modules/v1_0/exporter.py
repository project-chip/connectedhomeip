import modules.exporter as _base


class Exporter(_base.Exporter):

    def __init__(self, paths, args):
        super().__init__(paths, args)

    def encodeInt8u(self, a):
        self.data.extend(a.int().to_bytes(1, 'big'))

    def encodeInt16u(self, a):
        self.data.extend(a.int().to_bytes(2, 'big'))

    def encodeInt16u(self, a):
        self.data.extend(a.int().to_bytes(4, 'big'))

    def encodeBinary(self, a, v, max_size):
        size = len(v)
        # Size
        self.data.extend(int(size).to_bytes(2, 'big'))
        # Value
        self.data.extend(v)
        # Padding
        if size < max_size:
            self.data.extend(bytearray(max_size - size))

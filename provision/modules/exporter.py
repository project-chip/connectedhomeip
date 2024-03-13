import modules.util as _util
import modules.tools as _tools
from modules.parameters import Types, Formats, ID


class Exporter(object):

    kVersionFieldLengthInBits = 3
    kVendorIDFieldLengthInBits = 16
    kProductIDFieldLengthInBits = 16
    kCommissioningFlowFieldLengthInBits = 2
    kRendezvousInfoFieldLengthInBits = 8
    kPayloadDiscriminatorFieldLengthInBits = 12
    kSetupPINCodeFieldLengthInBits = 27
    kPaddingFieldLengthInBits = 4

    def __init__(self, paths, args):
        self.paths = paths
        self.args = args
        self.data = bytearray()

    def __str__(self):
        return "({}){}".format(len(self.data), self.data.hex())

    def reset(self):
        self.data = bytearray()

    def export(self):
        self.reset()
        # Well-known arguments
        self.put(ID.kSerialNumber)
        self.put(ID.kVendorId)
        self.put(ID.kVendorName)
        self.put(ID.kProductId)
        self.put(ID.kProductName)
        self.put(ID.kProductLabel)
        self.put(ID.kProductUrl)
        self.put(ID.kPartNumber)
        self.put(ID.kHwVersion)
        self.put(ID.kHwVersionStr)
        self.put(ID.kManufacturingDate)
        self.put(ID.kUniqueId)
        self.put(ID.kDiscriminator)
        self.put(ID.kSpake2pIterations)
        self.put(ID.kSpake2pSalt)
        self.put(ID.kSpake2pVerifier)
        self.put(ID.kFirmwareInfo)
        self.put(ID.kCertification)
        self.put(ID.kPaiCert)
        self.put(ID.kDacCert)
        self.put(ID.kDacKey)
        # Setup payload
        sp = self.args.get(ID.kSetupPayload)
        sp.set(_tools.QrCode.generateBits(self.args))
        self.add(sp)
        # Export to file
        ser = self.serialize()
        self.save(ser)
        print("\n* Exported({}): {}".format(len(ser), self.args.str(ID.kOutputPath)))

    def serialize(self):
        return self.data

    def save(self, data):
        output = self.args.get(ID.kOutputPath)
        path = output.str()
        if path is None:
            path = self.paths.temp('binary.bin')
            output.set(path)
        _util.BinaryFile(path).write(data)

    def put(self, k):
        self.add(self.args.get(k))

    def add(self, a):
        # print("++{}: {}; {}; [[{}]]".format(a.name, a.type, a.range(), a.value))
        if Types.INT8U == a.type:
            self.validateInt(a)
            self.encodeInt8u(a)
        elif Types.INT16U == a.type:
            self.validateInt(a)
            self.encodeInt16u(a)
        elif Types.INT32U == a.type:
            self.validateInt(a)
            self.encodeInt32u(a)
        elif Types.BINARY == a.type:
            if Formats.PATH == a.format:
                path = a.str()
                if path is None: _util.fail("Missing path for \"{}\"".format(a.name))
                self.addBinary(a, _util.BinaryFile(path).read())
            elif Formats.STRING == a.format:
                s = a.str()
                self.addBinary(a, (s is not None) and s.encode('utf-8') or None)
            else:
                self.addBinary(a, a.value)
        else:
            _util.fail("Export: Unsupported type {} for \"{}\"".format(a.type, a.name))

    def validateInt(self, a):
        i = a.int()
        r = a.range()
        # print("  {}? {}; {}".format(a.name, i, r))
        if (r is not None) and (i is not None) and (i not in r):
            _util.fail("Invalid value for \"{}\": {} [{}, {}]".format(a.name, i, r[0], r[-1]))

    def addBinary(self, a, b):
        if b is None: b = bytearray()
        r = a.range()
        if r is None:
            _util.fail("Missing bounds for \"{}\"".format(a.name))
        sz = len(b)
        mx = r[-1]
        if (r is not None) and (sz not in r):
            _util.fail("Invalid size for \"{}\": {} > {}".format(a.name, sz, mx))
        self.encodeBinary(a, b, mx)

    def encodeInt8u(self, a):
        pass

    def encodeInt16u(self, a):
        pass

    def encodeInt16u(self, a):
        pass

    def encodeBinary(self, a, v, max_size):
        pass

    # This method extracts the raw private-key from the DER file using the OpenSSL ASN.1 parser
    # Currently unused since the flash-only implementation reads DER
    @staticmethod
    def extractKey(path):
        TAG = 'HEX DUMP'
        ps = subprocess.Popen(('openssl', 'asn1parse', '-inform', 'der', '-in', path), stdout=subprocess.PIPE)
        out = subprocess.check_output(('grep', TAG), stdin=ps.stdout)
        line = out.decode(sys.stdout.encoding).strip()
        off = line.find(TAG) + len(TAG) + 2 # [HEX DUMP]:xxxx...
        return bytearray.fromhex(line[off:])

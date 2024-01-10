from modules.util import *
import subprocess
import base64

class Exporter(object):

    kSerialNumberLengthMax          = 32
    kVendorNameLengthMax            = 32
    kProductNameLengthMax           = 32
    kProductLabelLengthMax          = 32
    kProductUrlLengthMax            = 32
    kPartNumberLengthMax            = 32
    kHardwareVersionStrLengthMax    = 32
    kManufacturingDateLengthMax     = 11 # yyyy-mm-dd + \0
    kUniqueIdLengthMax              = 16
    kVerifierLengthMax              = 150
    kSpake2pSaltLengthMax           = 32
    kFirmwareInfoSizeMax            = 32
    kCertificateSizeMax             = 500
    kCertificationSizeMax           = 350
    kDeviceAttestationKeySizeMax    = 128
    kSetupPayloadSizeMax            = 32

    def __init__(self):
        self.reset()

    def __str__(self):
        return "({}){}".format(len(self.data), self.data.hex())

    def reset(self):
        self.data = bytearray()
        self.offset = 0

    def addUint16(self, x, tag = None):
        self.data.extend(int(x).to_bytes(2, 'big'))

    def addUint32(self, x, tag = None):
        self.data.extend(int(x).to_bytes(4, 'big'))

    def addArray(self, x, max, tag = None):
        if x is None:
            x = bytearray()
        # print("* {}({}/{}) @{}".format(tag or '?', len(x), max, len(self.data)))
        sz = len(x)
        assert(sz <= max)
        self.data.extend(int(sz).to_bytes(2, 'big'))
        self.data.extend(x)
        if sz < max:
            self.data.extend(bytearray(max - sz))

    def addString(self, x, max, tag = None):
        if x is None:
            return self.addArray(x, max, tag)
        else:
            return self.addArray(x.encode('utf-8'), max, tag)

    # This method extracts the raw private-key from the DER file using the OpenSSL ASN.1 parser
    # Currently unused since the flash-only implementation reads DER
    def extractKey(self, path):
        TAG = 'HEX DUMP'
        ps = subprocess.Popen(('openssl', 'asn1parse', '-inform', 'der', '-in', path), stdout=subprocess.PIPE)
        out = subprocess.check_output(('grep', TAG), stdin=ps.stdout)
        line = out.decode(sys.stdout.encoding).strip()
        off = line.find(TAG) + len(TAG) + 2 # [HEX DUMP]:xxxx...
        return bytearray.fromhex(line[off:])

    def export(self, args, paths):
        bitmap = 0xfffffffe
        self.addUint32(bitmap, '')
        self.addString(args.serial_number, Exporter.kSerialNumberLengthMax, 'serial_number')
        self.addUint16(args.vendor_id, 'vendor_id')
        self.addString(args.vendor_name, Exporter.kVendorNameLengthMax, 'vendor_name')
        self.addUint16(args.product_id, 'product_id')
        self.addString(args.product_name, Exporter.kProductNameLengthMax, 'product_name')
        self.addString(args.product_label, Exporter.kProductLabelLengthMax, 'product_label')
        self.addString(args.product_url, Exporter.kProductUrlLengthMax, 'product_url')
        self.addString(args.part_number, Exporter.kPartNumberLengthMax, 'part_number')
        self.addUint16(args.hw_version, 'hw_version')
        self.addString(args.hw_version_str, Exporter.kHardwareVersionStrLengthMax, 'hw_version_str')
        self.addString(args.manufacturing_date, Exporter.kManufacturingDateLengthMax, 'manufacturing_date')
        self.addArray(args.unique_id, Exporter.kUniqueIdLengthMax, 'unique_id')
        self.addUint16(args.discriminator, 'discriminator')
        self.addUint32(args.spake2p.iterations, 'iterations')
        self.addString(args.spake2p.salt, Exporter.kSpake2pSaltLengthMax, 'salt')
        self.addString(args.spake2p.verifier, Exporter.kVerifierLengthMax, 'verifier')
        self.addArray(bytearray(), Exporter.kFirmwareInfoSizeMax, 'fw_info') # TODO; FirmwareInfo
        # self.addUint32(0) # Base Address
        self.addArray(self.read(paths.cd), Exporter.kCertificationSizeMax, 'cd')
        self.addArray(self.read(paths.pai_cert_der), Exporter.kCertificateSizeMax, 'pai')
        self.addArray(self.read(paths.dac_cert_der), Exporter.kCertificateSizeMax, 'dac')
        self.addArray(self.read(paths.dac_key_der), Exporter.kDeviceAttestationKeySizeMax, 'key')
        self.addArray(self.generateQrCodeBitSet(args), Exporter.kSetupPayloadSizeMax, 'payload')

        with open("{}".format(args.binary), 'wb') as f:
            f.write(self.data)

        print("\n◆ Exported({}): {}".format(len(self.data), args.binary))

    def read(self, path):
        contents = bytearray()
        with open(path, 'rb') as f:
            contents = bytearray(f.read())
        return contents

    def generateQrCodeBitSet(self, args):
        kVersionFieldLengthInBits = 3
        kVendorIDFieldLengthInBits = 16
        kProductIDFieldLengthInBits = 16
        kCommissioningFlowFieldLengthInBits = 2
        kRendezvousInfoFieldLengthInBits = 8
        kPayloadDiscriminatorFieldLengthInBits = 12
        kSetupPINCodeFieldLengthInBits = 27
        kPaddingFieldLengthInBits = 4

        kTotalPayloadDataSizeInBits = (kVersionFieldLengthInBits + kVendorIDFieldLengthInBits + kProductIDFieldLengthInBits +
                                       kCommissioningFlowFieldLengthInBits + kRendezvousInfoFieldLengthInBits + kPayloadDiscriminatorFieldLengthInBits +
                                       kSetupPINCodeFieldLengthInBits + kPaddingFieldLengthInBits)

        offset = 0
        fillBits = [0] * int(kTotalPayloadDataSizeInBits / 8)
        offset = self.WriteBits(fillBits, offset, 0, kVersionFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, args.vendor_id, kVendorIDFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, args.product_id, kProductIDFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, args.commissioning_flow,
                                kCommissioningFlowFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, args.rendezvous_flags,
                                kRendezvousInfoFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, args.discriminator,
                                kPayloadDiscriminatorFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, args.spake2p.passcode, kSetupPINCodeFieldLengthInBits, kTotalPayloadDataSizeInBits)
        offset = self.WriteBits(fillBits, offset, 0, kPaddingFieldLengthInBits, kTotalPayloadDataSizeInBits)

        return bytes(fillBits)

    # Populates numberOfBits starting from LSB of input into bits, which is assumed to be zero-initialized
    def WriteBits(self, bits, offset, input, numberOfBits, totalPayloadSizeInBits):
        if ((offset + numberOfBits) > totalPayloadSizeInBits):
            print("THIS IS NOT VALID")
            return
        # input < 1u << numberOfBits);

        index = offset
        offset += numberOfBits
        while (input != 0):
            if (input & 1):
                bits[int(index / 8)] |= (1 << (index % 8))
            index += 1
            input >>= 1

        return offset
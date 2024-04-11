import os
import time
import shutil
import datetime
import base64
import hashlib
import struct
from ecdsa.curves import NIST256p
import modules.util as _util
from modules.parameters import ID


class Commander:

    def __init__(self, args, conn):
        self.device = args.str(ID.kDevice)
        self.conn = conn

    def execute(self, args, output = True, check = True):
        args.insert(0, 'commander')
        if self.device is not None:
            args.extend(['--device', self.device])
        if self.conn is None:
            pass
        elif self.conn.serial_num:
            args.extend(['--serialno', self.conn.serial_num])
        elif self.conn.ip_addr:
            if self.conn.port:
                args.extend(['--ip', '{}:{}'.format(self.conn.ip_addr, self.conn.port)])
            else:
                args.extend(['--ip', self.conn.ip_addr])
        cmd = ' '.join(args)
        return _util.execute(args, output, check)

    def info(self):
        res = self.execute(['device', 'info'], True, False)
        if res is None: _util.fail("Cannot retrieve device info")
        return DeviceInfo(res)

    def flash(self, image_path):
        _, ext = os.path.splitext(image_path)
        if '.rps' == ext:
            self.execute(['rps', 'load', image_path], False, True)
            # Si917 needs time to start
            time.sleep(1)
        else:
            self.execute(['flash' , image_path], False, True)
            self.execute(['device', 'reset'], False, True)


class DeviceInfo:

    def __init__(self, text):
        if text is None: _util.fail("Missing device info")
        d = self.parseLines(text.decode('utf-8').splitlines())
        self.part = self.parseField(d, 'Part Number')
        self.uid = self.parseField(d, 'Unique ID')
        self.revision = self.parseField(d, 'Die Revision')
        self.version = self.parseField(d, 'Production Ver')
        self.flash_size = self.parseSize(d, 'Flash Size')
        self.family = self.part[0:9].lower()

    def parseLines(self, lines):
        m = {}
        for l in lines:
            pair = l.split(':')
            if len(pair) > 1:
                m[pair[0].strip()] = pair[1].strip().lower()
        return m

    def parseField(self, d, tag, default_value = '?'):
        v =  tag in d and d[tag] or default_value
        return isinstance(v, str) and v.lower() or v

    def parseSize(self, d, tag):
        text = self.parseField(d, tag, '0')
        if text is None: return 0
        parts = text.split()
        value = int(parts[0])
        multiplier = 1
        if len(parts) > 0 and ('kb' == parts[1].lower()):
            multiplier = 1024
        return value * multiplier

    def __str__(self):
        text =  "{}+ part: '{}'\n".format(_util.MARGIN, self.part)
        text += "{}+ family: '{}'\n".format(_util.MARGIN, self.family)
        text += "{}+ version: '{}'\n".format(_util.MARGIN, self.version)
        text += "{}+ revision: '{}'\n".format(_util.MARGIN, self.revision)
        text += "{}+ flash_size: 0x{:08x}\n".format(_util.MARGIN, self.flash_size)
        return text


class CertTool:

    def __init__(self, tool_path, vid, pid):
        self.lifetime = 3660
        self.tool = tool_path
        self.vid = vid
        self.pid = pid

    def generateCD(self, cdc, cdk, cd):
        version = 0x101
        security_level = 0
        security_info = 0
        serial_num = self.generateSerial()
        # Remove existing CD
        if os.path.exists(cd):
            os.remove(cd)
        # Generate CD
        self.execute(['gen-cd', '-f', '1', '-V', self.vid, '-p', self.pid, '-d', '0x0016', '-c', 'ZIG20142ZB330003-24', '-l', security_level, '-i', security_info, '-n', version, '-t', '0', '-o', self.vid, '-r' , self.pid, '-C', cdc, '-K', cdk, '-O', cd ])

    def generatePAA(self, paa_cert, paa_key):
        # Remove existing PAA
        if os.path.exists(paa_cert):
            os.remove(paa_cert)
        if os.path.exists(paa_key):
            os.remove(paa_key)
        # Generate PAA
        self.execute(['gen-att-cert', '-t', 'a', '-l', self.lifetime, '-c', '"Matter PAA"', '-V', self.vid, '-o', paa_cert, '-O', paa_key])

    def generatePAI(self, paa_cert, paa_key, pai_cert, pai_key):
        # Remove existing PAI
        if os.path.exists(pai_cert):
            os.remove(pai_cert)
        if os.path.exists(pai_key):
            os.remove(pai_key)
        # Generate PAI
        self.execute(['gen-att-cert', '-t', 'i', '-l', self.lifetime, '-c', '"Matter PAI"', '-V', self.vid, '-P', self.pid, '-C', paa_cert, '-K', paa_key, '-o', pai_cert, '-O', pai_key])

    def generateDAC(self, pai_cert, pai_key, dac_cert, dac_key, common_name = 'Matter DAC'):
        # Remove existing DAC
        if os.path.exists(dac_cert):
            os.remove(dac_cert)
        if os.path.exists(dac_key):
            os.remove(dac_key)
        # Generate DAC
        cn = "\"{}\"".format(common_name)
        self.execute(['gen-att-cert', '-t', 'd', '-l', self.lifetime, '-c', cn, '-V', self.vid, '-P', self.pid, '-C', pai_cert, '-K', pai_key, '-o', dac_cert, '-O', dac_key])

    def generateSerial(self):
        base_time = datetime.datetime(2000, 1, 1)
        delta = datetime.datetime.now() - base_time
        return delta.seconds

    def execute(self, args):
        if self.tool is None:
            raise ValueError("Missing Cert Tool");
        if (not os.path.exists(self.tool)) or (not os.path.isfile(self.tool)):
            raise ValueError("Invalid Cert Tool: {}".format(self.tool));
        _util.execute([ self.tool ] + args)

class Spake2p:
    INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444,
                            55555555, 66666666, 77777777, 88888888, 99999999, 12345678, 87654321]
    kSaltMin = 16
    kSaltMax = 32
    kIterationsMin = 1000
    kIterationsMax = 100000

    def __init__(self):
        pass

    @staticmethod
    def generateVerifier(passcode, iterations, salt_b64):
        if(passcode is None): _util.fail("Missing SPAKE2+ passcode")
        if(iterations is None): _util.fail("Missing SPAKE2+ iteration count")
        if(salt_b64 is None): _util.fail("Missing SPAKE2+ salt")

        salt = base64.b64decode(salt_b64)
        salt_length = len(salt)
        if salt_length < Spake2p.kSaltMin:
            fail("Invalid SPAKE2+ salt length: {} < {}".format(salt_length, Spake2p.kSaltMin))
        if salt_length > Spake2p.kSaltMax:
            fail("Invalid SPAKE2+ salt length: {} > {}".format(salt_length, Spake2p.kSaltMax))

        WS_LENGTH = NIST256p.baselen + 8
        ws = hashlib.pbkdf2_hmac('sha256', struct.pack('<I', passcode), salt, iterations, WS_LENGTH * 2)
        w0 = int.from_bytes(ws[:WS_LENGTH], byteorder='big') % NIST256p.order
        w1 = int.from_bytes(ws[WS_LENGTH:], byteorder='big') % NIST256p.order
        L = NIST256p.generator * w1
        verifier = w0.to_bytes(NIST256p.baselen, byteorder='big') + L.to_bytes('uncompressed')
        verifier_b64 = base64.b64encode(verifier).decode('utf-8')
        return verifier_b64


class QrCode(object):

    kVersionFieldLengthInBits = 3
    kVendorIDFieldLengthInBits = 16
    kProductIDFieldLengthInBits = 16
    kCommissioningFlowFieldLengthInBits = 2
    kRendezvousInfoFieldLengthInBits = 8
    kPayloadDiscriminatorFieldLengthInBits = 12
    kSetupPINCodeFieldLengthInBits = 27
    kPaddingFieldLengthInBits = 4

    @staticmethod
    def generateBits(args):
        # Vendor ID
        vendor_id = args.int(ID.kVendorId)
        if vendor_id is None: _util.fail("Missing verndor_id")
        # Product ID
        product_id = args.int(ID.kProductId)
        if product_id is None: _util.fail("Missing product_id")
        # Commissioning Flow
        commissioning_flow = args.int(ID.kCommissioningFlow)
        if commissioning_flow is None: _util.fail("Missing commissioning_flow")
        # Rendezvous Flags
        rendezvous_flags = args.int(ID.kRendezvousFlags)
        if rendezvous_flags is None: _util.fail("Missing rendezvous_flags")
        # Discriminator
        discriminator = args.int(ID.kDiscriminator)
        if discriminator is None: _util.fail("Missing discriminator")
        # SPAKE2+ passcode
        spake2p_passcode = args.int(ID.kSpake2pPasscode)
        if spake2p_passcode is None: _util.fail("Missing SPAKE2+ passcode")
        # Total payload size (in bits)
        total_payload_data_bits = (QrCode.kVersionFieldLengthInBits +
                                   QrCode.kVendorIDFieldLengthInBits +
                                   QrCode.kProductIDFieldLengthInBits +
                                   QrCode.kCommissioningFlowFieldLengthInBits +
                                   QrCode.kRendezvousInfoFieldLengthInBits +
                                   QrCode.kPayloadDiscriminatorFieldLengthInBits +
                                   QrCode.kSetupPINCodeFieldLengthInBits +
                                   QrCode.kPaddingFieldLengthInBits)

        offset = 0
        bits = [0] * int(total_payload_data_bits / 8)
        offset = QrCode.writeBits(bits, offset, 0, QrCode.kVersionFieldLengthInBits, total_payload_data_bits)
        offset = QrCode.writeBits(bits, offset, vendor_id, QrCode.kVendorIDFieldLengthInBits, total_payload_data_bits)
        offset = QrCode.writeBits(bits, offset, product_id, QrCode.kProductIDFieldLengthInBits, total_payload_data_bits)
        offset = QrCode.writeBits(bits, offset, commissioning_flow,
                                  QrCode.kCommissioningFlowFieldLengthInBits, total_payload_data_bits)
        offset = QrCode.writeBits(bits, offset, rendezvous_flags,
                                  QrCode.kRendezvousInfoFieldLengthInBits, total_payload_data_bits)
        offset = QrCode.writeBits(bits, offset, discriminator,
                                  QrCode.kPayloadDiscriminatorFieldLengthInBits, total_payload_data_bits)
        offset = QrCode.writeBits(bits, offset, spake2p_passcode, QrCode.kSetupPINCodeFieldLengthInBits, total_payload_data_bits)
        offset = QrCode.writeBits(bits, offset, 0, QrCode.kPaddingFieldLengthInBits, total_payload_data_bits)

        return bytes(bits)


    # Populates numberOfBits starting from LSB of input into bits, which is assumed to be zero-initialized
    @staticmethod
    def writeBits(bits, offset, input, bit_size, total_payload_bits):
        if ((offset + bit_size) > total_payload_bits):
            _util.fail("Invalid QR code bits: {} > {}".format(offset + bit_size, total_payload_bits))
            return

        index = offset
        offset += bit_size
        while (input != 0):
            if (input & 1):
                bits[int(index / 8)] |= (1 << (index % 8))
            index += 1
            input >>= 1

        return offset

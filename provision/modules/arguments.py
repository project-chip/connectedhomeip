from .util import *
import json
import os
import argparse
import base64
from datetime import datetime


def parseInt(i):
    return int(i, 0)

def decode(d, k, override_val = None, def_value = None):
    if override_val is not None:
        return override_val
    elif k in d:
        return d[k]
    else:
        return def_value

def encode(d, k, v):
    if v is not None:
        d[k] = v


class BaseArguments:

    def load(self):
        parser = argparse.ArgumentParser(description='Provision')
        self.configure(parser)
        args = parser.parse_args()

        # Load configuration file, if any
        if args.config is not None:
            args = self.read(args.config, args)
        # Validate
        self.process(args)


    def configure(self, parser):
        parser.add_argument('-c', '--config', type=str, help='[string] Path to configuration file.')


    def process(self, args):
        pass


    def read(self, filename, args):
        if not os.path.exists(filename):
            fail("Invalid configuration path: '{}'".format(filename))

        d = {}
        with open(filename, 'r') as f:
            d = json.loads(f.read())

        self.version = decode(d, 'version', None, self.version)
        matter = decode(d, 'matter', None, {})
        return self.decode(matter, args)


    def write(self, filename):

        d = {
            'version': self.version,
            'matter': self.encode()
        }
        with open(filename, 'w') as outfile:
            json.dump(d, outfile, indent=4)


class Config:
    def __init__(self):
        self.stop = False
        self.binary = False

class Arguments(BaseArguments):
    VERSION = "1.0"
    kMaxVendorNameLength = 32
    kMaxProductNameLength = 32
    kMaxHardwareVersionStringLength = 64
    kMaxSerialNumberLength = 32
    kUniqueIDLength = 16
    kMaxProductUrlLenght = 256
    kMaxPartNumberLength = 32
    kMaxProductLabelLength = 64
    kDefaultCommissioningFlow = 0
    kDefaultRendezvousFlags = 2
    kDefaultKeyId = 0

    def __init__(self):
        super().__init__()
        self.version = Arguments.VERSION
        self.conn = None
        self.temp = None
        self.binary = None
        self.generate = None
        self.stop = None
        self.csr = None
        self.gen_fw = None
        self.prod_fw = None
        self.serial_number = None
        self.vendor_id = None
        self.vendor_name = None
        self.product_id = None
        self.product_name = None
        self.product_label = None
        self.product_url = None
        self.part_number = None
        self.hw_version = None
        self.hw_version_str = None
        self.manufacturing_date = None
        self.unique_id = None
        self.commissioning_flow = None
        self.rendezvous_flags = None
        self.discriminator = None
        self.spake2p = None
        self.rendezvous_flags = None

    def configure(self, parser):
        super().configure(parser)
        parser.add_argument('-t', '--temp', type=str, help='[string] Temporary folder.')
        parser.add_argument('-b', '--binary', type=str, help='[string] Binary export.')
        parser.add_argument('-j', '--jlink', type=str, help='[string] J-Link connection.')
        parser.add_argument('-l', '--pylink_lib', type=str, help='[string] Path to the PyLink library.')
        parser.add_argument('-g', '--generate', action='store_true', help='[boolean] Generate certificates.', default=None)
        parser.add_argument('-s', '--stop', action='store_true', help='[string] Generate JSON file and stop', default=None)
        parser.add_argument('-r', '--csr', action='store_true', help='[boolean] Generate Certificate Signing Request', default=None)
        parser.add_argument('-gf', '--gen_fw', type=str, help='[string] Path to the Generator Firmware image')
        parser.add_argument('-pf', '--prod_fw', type=str, help='[string] Path to the Production Firmware image')
        parser.add_argument('-S',  '--serial_number', type=str, help='[string] SerialNumber')
        parser.add_argument('-v',  '--vendor_id', type=parseInt, help='[int] vendor ID')
        parser.add_argument('-V',  '--vendor_name', type=str, help='[string] vendor name')
        parser.add_argument('-p',  '--product_id', type=parseInt, help='[int] Product ID')
        parser.add_argument('-P',  '--product_name', type=str, help='[string] product name')
        parser.add_argument('-pl', '--product_label', type=str, help='[string] Provide the product label')
        parser.add_argument('-pu', '--product_url', type=str, help='[string] Provide the product url')
        parser.add_argument('-pn', '--part_number', type=str, help='[string] Part number')
        parser.add_argument('-hv', '--hw_version', type=parseInt, help='[int] Hardware version value')
        parser.add_argument('-hs', '--hw_version_str', type=str, help='[string] Hardware version string')
        parser.add_argument('-md', '--manufacturing_date', type=str, help='[string] Manufacturing date in YYYY-MM-DD format')
        parser.add_argument('-u',  '--unique_id', type=str,help='[hex_string] 128 bits hex string unique id (without 0x)')
        parser.add_argument('-cf', '--commissioning_flow', type=parseInt, help='[int] Commissioning Flow: 0=Standard, 1=kUserActionRequired, 2=Custom (Default:Standard)')
        parser.add_argument('-rf', '--rendezvous_flags', type=parseInt, help='[int] Rendez-vous flag: 1=SoftAP, 2=BLE 4=OnNetwork (Default=BLE Only)')
        parser.add_argument('-d',  '--discriminator', type=parseInt, help='[int] BLE pairing discriminator.')
        # Attestation
        parser.add_argument('-ct', '--cert_tool', type=str, help='[boolean] Path to the `chip-cert` tool.')
        parser.add_argument('-ki', '--key_id', type=parseInt, help='[int] Key ID')
        parser.add_argument('-kp', '--key_pass', type=str, help='[string] PKCS#12 attestation certificates key_pass')
        parser.add_argument('-xc', '--att_certs', type=str, help='[string] PKCS#12 attestation certificates path')
        parser.add_argument('-ac', '--paa_cert', type=str, help='[string] PAA certificate path')
        parser.add_argument('-ak', '--paa_key', type=str, help='[string] PAA key path')
        parser.add_argument('-ic', '--pai_cert', type=str, help='[string] PAI certificate path')
        parser.add_argument('-ik', '--pai_key', type=str, help='[string] PAI key path')
        parser.add_argument('-dc', '--dac_cert', type=str, help='[string] PAI certificate path')
        parser.add_argument('-dk', '--dac_key', type=str, help='[string] PAI key path')
        parser.add_argument('-cd', '--certification', type=str, help='[string] Certification Declaration')
        parser.add_argument('-cn', '--common_name', type=str, help='[string] DAC Common Name')
        # SPAKE2+
        parser.add_argument('-sv', '--spake2p_verifier', type=str, help='[string] SPAKE2+ verifier without generating it')
        parser.add_argument('-sp', '--spake2p_passcode', type=parseInt, help='[int] Default PASE session passcode')
        parser.add_argument('-ss', '--spake2p_salt', type=str, help='[string] Provide SPAKE2+ salt')
        parser.add_argument('-si', '--spake2p_iterations', type=parseInt, help='[int] SPAKE2+ iteration count')

    def decode(self, d, args):
        c = Config()
        c.temp = decode(d, 'temp', args.temp)
        c.binary = decode(d, 'binary', args.binary)
        c.jlink = decode(d, 'jlink', args.jlink)
        c.pylink_lib = decode(d, 'pylink_lib', args.pylink_lib)
        c.generate = decode(d, 'generate', args.generate)
        c.stop = decode(d, 'stop', args.stop)
        c.csr = decode(d, 'csr', args.csr)
        c.gen_fw = decode(d, 'gen_fw', args.gen_fw)
        c.prod_fw = decode(d, 'prod_fw', args.prod_fw)
        c.serial_number = decode(d, 'serial_number', args.serial_number)
        c.vendor_id = decode(d, 'vendor_id', args.vendor_id)
        c.vendor_name = decode(d, 'vendor_name', args.vendor_name)
        c.product_id = decode(d, 'product_id', args.product_id)
        c.product_name = decode(d, 'product_name', args.product_name)
        c.product_label = decode(d, 'product_label', args.product_label)
        c.product_url = decode(d, 'product_url', args.product_url)
        c.part_number = decode(d, 'part_number', args.part_number)
        c.hw_version = decode(d, 'hw_version', args.hw_version)
        c.hw_version_str = decode(d, 'hw_version_str', args.hw_version_str)
        c.manufacturing_date = decode(d, 'manufacturing_date', args.manufacturing_date)
        c.unique_id = decode(d, 'unique_id', args.unique_id)
        c.commissioning_flow = decode(d, 'commissioning_flow', args.commissioning_flow, Arguments.kDefaultCommissioningFlow)
        c.rendezvous_flags = decode(d, 'rendezvous_flags', args.rendezvous_flags, Arguments.kDefaultRendezvousFlags)
        c.discriminator = decode(d, 'discriminator', args.discriminator)
        # Attestation
        attest = decode(d, 'attestation', None, {})
        c.cert_tool = decode(attest, 'cert_tool', args.cert_tool)
        c.key_id = decode(attest, 'key_id', args.key_id, Arguments.kDefaultKeyId)
        c.att_certs = decode(attest, 'pkcs12', args.att_certs)
        c.key_pass = decode(attest, 'key_pass', args.key_pass)
        c.paa_cert = decode(attest, 'paa_cert', args.paa_cert)
        c.paa_key = decode(attest, 'paa_key', args.paa_key)
        c.pai_cert = decode(attest, 'pai_cert', args.pai_cert)
        c.pai_key = decode(attest, 'pai_key', args.pai_key)
        c.dac_cert = decode(attest, 'dac_cert', args.dac_cert)
        c.dac_key = decode(attest, 'dac_key', args.dac_key)
        c.certification = decode(attest, 'certification', args.certification)
        c.common_name = decode(attest, 'common_name', args.common_name)
        # SPAKE2+
        spake = decode(d, 'spake2p', None, {})
        c.spake2p_iterations = decode(spake, 'iterations', args.spake2p_iterations)
        c.spake2p_passcode = decode(spake, 'passcode', args.spake2p_passcode)
        c.spake2p_salt = decode(spake, 'salt', args.spake2p_salt)
        c.spake2p_verifier = decode(spake, 'verifier', args.spake2p_verifier)
        return c


    def encode(self):
        d = {}
        encode(d, 'jlink', self.conn.encode())
        encode(d, 'pylink_lib', self.conn.lib_path)
        encode(d, 'temp', self.temp)
        encode(d, 'generate', self.generate)
        encode(d, 'csr', self.csr)
        encode(d, 'gen_fw', self.gen_fw)
        encode(d, 'prod_fw', self.prod_fw)
        encode(d, 'serial_number', self.serial_number)
        encode(d, 'vendor_id', self.vendor_id)
        encode(d, 'vendor_name', self.vendor_name)
        encode(d, 'product_id', self.product_id)
        encode(d, 'product_name', self.product_name)
        encode(d, 'product_label', self.product_label)
        encode(d, 'product_url', self.product_url)
        encode(d, 'part_number', self.part_number)
        encode(d, 'hw_version', self.hw_version)
        encode(d, 'hw_version_str', self.hw_version_str)
        encode(d, 'manufacturing_date', self.manufacturing_date)
        if self.unique_id is not None:
            encode(d, 'unique_id', self.unique_id.hex())
        encode(d, 'commissioning_flow', self.commissioning_flow)
        encode(d, 'rendezvous_flags', self.rendezvous_flags)
        encode(d, 'discriminator', self.discriminator)
        # Attestation
        attest = {}
        encode(attest, 'cert_tool', self.attest.cert_tool)
        encode(attest, 'key_id', self.attest.key_id)
        encode(attest, 'pkcs12', self.attest.pkcs12)
        encode(attest, 'key_pass', self.attest.key_pass)
        encode(attest, 'paa_cert', self.attest.paa_cert)
        encode(attest, 'paa_key', self.attest.paa_key)
        encode(attest, 'pai_cert', self.attest.pai_cert)
        encode(attest, 'pai_key', self.attest.pai_key)
        encode(attest, 'dac_cert', self.attest.dac_cert)
        encode(attest, 'dac_key', self.attest.dac_key)
        encode(attest, 'certification', self.attest.cd)
        encode(attest, 'common_name', self.attest.cn)
        d["attestation"] = attest
        # SPAKE2+
        spake = {}
        encode(spake, 'iterations', self.spake2p.iterations)
        encode(spake, 'passcode', self.spake2p.passcode)
        encode(spake, 'verifier', self.spake2p.verifier)
        encode(spake, 'salt', self.spake2p.salt)
        d["spake2p"] = spake
        return d


    def process(self, args):

        # Connection
        self.conn = ConnectionArguments()
        self.conn.decode(args)

        self.temp = args.temp
        self.binary = args.binary
        self.generate = args.generate
        self.stop = args.stop
        self.csr = args.csr

        self.gen_fw = args.gen_fw
        self.prod_fw = args.prod_fw
        if args.serial_number is None:
            self.serial_number = None
        else:
            self.serial_number = args.serial_number

        if args.vendor_id is None:
            fail("Missing Vendor ID (--vendor_id)")
        self.vendor_id = args.vendor_id

        if args.vendor_name is None:
            self.vendor_name = None
        else:
            assert (len(args.vendor_name) <= Arguments.kMaxVendorNameLength), "Vendor name exceeds the size limit"
            self.vendor_name = args.vendor_name

        if args.product_id is None:
            fail("Missing Product ID (--product_id)")
        self.product_id = args.product_id

        if args.product_name is None:
            self.product_name = None
        else:
            assert (len(args.product_name) <= Arguments.kMaxProductNameLength), "Product name exceeds the size limit"
            self.product_name = args.product_name

        if args.product_label is None:
            self.product_label = None
        else:
            assert (len(args.product_label) <= Arguments.kMaxProductLabelLength), "Product Label exceeds the size limit"
            self.product_label = args.product_label

        if args.product_url is None:
            self.product_url = None
        else:
            assert (len(args.product_url) <= Arguments.kMaxProductUrlLenght), "Product URL exceeds the size limit"
            self.product_url = args.product_url

        if args.part_number is None:
            self.part_number = None
        else:
            assert (len(args.part_number) <= Arguments.kMaxPartNumberLength), "Part number exceeds the size limit"
            self.part_number = args.part_number

        self.hw_version = args.hw_version or 0

        if args.hw_version_str is None:
            args.hw_version_str = None
        else:
            self.hw_version_str = args.hw_version_str
            assert (len(args.hw_version_str) <= Arguments.kMaxHardwareVersionStringLength), "Hardware version string exceeds the size limit"

        if args.manufacturing_date:
            try:
                # self.manufacturing_date = date.fromisoformat(args.manufacturing_date)
                datetime.strptime(args.manufacturing_date, '%Y-%m-%d')
            except ValueError:
                raise ValueError("Incorrect manufacturing data format, should be YYYY-MM-DD")
            self.manufacturing_date = args.manufacturing_date
        else:
            self.manufacturing_date = datetime.now().strftime('%Y-%m-%d')

        if args.unique_id is None:
            self.unique_id = None
        else:
            assert (len(bytearray.fromhex(args.unique_id)) == Arguments.kUniqueIDLength), "Provide a 16 bytes rotating id"
            self.unique_id = bytearray.fromhex(args.unique_id)

        if args.commissioning_flow is None:
            self.commissioning_flow = Arguments.kDefaultCommissioningFlow
        else:
            assert (args.commissioning_flow <= 3), "Invalid commissioning flow value"
            self.commissioning_flow = args.commissioning_flow

        if args.rendezvous_flags is None:
            self.rendezvous_flags = Arguments.kDefaultRendezvousFlags
        else:
            assert (args.rendezvous_flags <= 7), "Invalid rendez-vous flag value"
            self.rendezvous_flags = args.rendezvous_flags

        if args.discriminator is not None:
            assert (int(args.discriminator) < 0xffff), "Invalid discriminator > 0xffff"
        self.discriminator = args.discriminator or 0

        # Attestation Files
        self.attest = AttestationArguments()
        self.attest.validate(args)

        # SPAKE2+
        self.spake2p = Spake2pArguments()
        self.spake2p.validate(args)

        return True


class ConnectionArguments:

    def __init__(self):
        self.lib_path = None
        self.serial_num = None
        self.ip_addr = None
        self.port = None

    def __str__(self):
        if self.serial_num is not None:
            return str(self.serial_num)
        if self.ip_addr is not None:
            if self.port is not None:
                return "{}:{}".format(self.ip_addr, self.port)
            else:
                return self.ip_addr
        else:
            return ''

    def decode(self, args):

        if args.pylink_lib is not None:
            self.lib_path = args.pylink_lib

        if args.jlink is None:
            return

        if args.jlink.find('.') < 0:
            # Serial port
            self.serial_num = args.jlink
        else:
            # IP address
            tuple = args.jlink.split(':')
            if len(tuple) > 1:
                self.port = int(tuple[1])
            self.ip_addr = tuple[0]

    def encode(self):
        if (self.serial_num is None) and (self.ip_addr is None) and (self.port is None):
            return None
        return str(self)


class AttestationArguments:

    kDefaultKeyId = 0

    def __init__(self):
        self.cert_tool = None
        self.key_id = None
        self.pkcs12 = None
        self.key_pass = None
        self.paa_cert = None
        self.paa_key = None
        self.pai_cert = None
        self.pai_key = None
        self.dac_cert = None
        self.dac_key = None
        self.cd = None
        self.cn = None

    def validate(self, args):

        # CD
        if not args.generate:
            if args.certification is None:
                fail("Missing Certification Declaration path (--certification)")
            else:
                if not os.path.exists(args.certification):
                    fail("Invalid Certification Declaration path: '{}'".format(args.certification))

        if args.csr:
            # Use CSR
            if args.common_name is None:
                fail("Missing DAC Common Name (--common_name)")

        elif args.generate:
            # Generate test certificates (PAA, PAI, DAC, CD)

            # PAA
            if (args.paa_cert is not None) and (not os.path.exists(args.paa_cert)):
                fail("Invalid PAA certificate path: '{}'".format(args.paa_cert))
            if (args.paa_key is not None) and (not os.path.exists(args.paa_key)):
                fail("Invalid PAA key path: '{}'".format(args.paa_key))
            # PAI
            if (args.pai_cert is not None) and (not os.path.exists(args.pai_cert)):
                fail("Invalid PAI certificate path: '{}'".format(args.pai_cert))
            if (args.pai_key is not None) and (not os.path.exists(args.pai_key)):
                fail("Invalid PAI key path: '{}'".format(args.pai_key))
            # DAC
            if (args.dac_cert is not None) and (not os.path.exists(args.dac_cert)):
                fail("Invalid DAC certificate path: '{}'".format(args.dac_cert))
            # CD
            if (args.certification is not None) and (not os.path.exists(args.certification)):
                fail("Invalid Certification Declaration path: '{}'".format(args.certification))

            if args.cert_tool and (not os.path.exists(args.cert_tool)):
                fail("Invalid chip-cert tool path: '{}'".format(args.cert_tool))

        else:
            # Use existing certificates (PAI, DAC, DAC-key)

            if args.att_certs:
                # Single file
                if not os.path.exists(args.att_certs):
                    fail("Invalid certificates path: '{}'".format(args.att_certs))
            else:
                # Separate files
                if args.pai_cert is None:
                    fail("Missing PAI certificate path (--pai_cert)")
                elif not os.path.exists(args.pai_cert):
                    fail("Invalid PAI certificate path: '{}'".format(args.pai_cert))

                if args.dac_cert is None:
                    fail("Missing DAC certificate path (--dac_cert)")
                elif not os.path.exists(args.dac_cert):
                    fail("Invalid DAC certificate path: '{}'".format(args.dac_cert))

                if args.dac_key is None:
                    fail("Missing DAC key path (--dac_key)")
                elif not os.path.exists(args.dac_key):
                    fail("Invalid DAC key path: '{}'".format(args.dac_key))

        self.cert_tool = args.cert_tool
        self.key_id = args.key_id or 0
        self.pkcs12 = args.att_certs
        self.key_pass = args.key_pass or ''
        self.paa_cert = args.paa_cert
        self.paa_key = args.paa_key
        self.pai_cert = args.pai_cert
        self.dac_cert = args.dac_cert
        self.dac_key = args.dac_key
        self.cd = args.certification
        self.cn = args.common_name


class Spake2pArguments:
    INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444,
                            55555555, 66666666, 77777777, 88888888, 99999999, 12345678, 87654321]

    kPasscodeDefault = 95145455
    kSaltMin = 16
    kSaltMax = 32
    kSaltDefault = '95834coRGvFhCB69IdmJyr5qYIzFgSirw6Ja7g5ySYA='
    kIterationsDefault = 15000
    kIterationsMin = 1000
    kIterationsMax = 100000

    def __init__(self):
        self.verifier = None
        self.passcode = None
        self.salt = None
        self.iterations = None

    def validate(self, args):
        # Passcode
        self.passcode = args.spake2p_passcode or Spake2pArguments.kPasscodeDefault
        if(self.passcode in Spake2pArguments.INVALID_PASSCODES):
            fail("The provided passcode is invalid")
        # Salt
        try:
            self.salt = args.spake2p_salt or Spake2pArguments.kSaltDefault
        except:
            fail("Invalid SPAKE2+ salt (base64): {}".format(args.spake2p.salt))
        # Iterations
        self.iterations = args.spake2p_iterations or Spake2pArguments.kIterationsDefault
        if not self.iterations:
            fail("SPAKE2+ iteration count required (--spake2p_iterations)")
        if (self.iterations < Spake2pArguments.kIterationsMin) or (self.iterations > Spake2pArguments.kIterationsMax):
            fail("Invalid SPAKE2+ iteration count: {} [{}, {}]".format(self.iterations, Spake2pArguments.kIterationsMin, Spake2pArguments.kIterationsMax))

        if args.spake2p_verifier:
            # Verifier provided
            self.verifier = args.spake2p_verifier

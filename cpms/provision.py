#! /usr/bin/python3
from modules.serial_comm import *
from modules.commander import *
from modules.commands import *
from modules.util import *
from modules.signing_server import *
from datetime import datetime
from ecdsa.curves import NIST256p
import subprocess
import argparse
import base64
import hashlib
import struct
import argparse
import json
import os
import sys

def parseInt(i): return int(i, 0)
def parseBool(x):
    if x is None:
        return False
    low = x.lower()
    return ('true' == low) or ('1' == low)

#-------------------------------------------------------------------------------

class AttestationArguments:

    kDefaultToolPath = './out/debug/chip-cert'
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
        self.key_id = args.key_id
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
        if not self.passcode:
            fail("SPAKE2+ passcode required (--spake2p_passcode)")
        if(self.passcode in Spake2pArguments.INVALID_PASSCODES):
            fail("The provided passcode is invalid")
        # Salt
        self.salt = args.spake2p_salt or Spake2pArguments.kSaltDefault
        if not self.salt:
            fail("SPAKE2+ salt required (--spake2p_salt)")
        # Iterations
        self.iterations = args.spake2p_iterations or Spake2pArguments.kIterationsDefault
        if not self.iterations:
            fail("SPAKE2+ iteration count required (--spake2p_iterations)")
        if (self.iterations < Spake2pArguments.kIterationsMin) or (self.iterations > Spake2pArguments.kIterationsMax):
            fail("Invalid SPAKE2+ iteration count: {} [{}, {}]".format(self.iterations, Spake2pArguments.kIterationsMin, Spake2pArguments.kIterationsMax))

        if args.spake2p_verifier:
            # Verifier provided
            self.verifier = args.spake2p_verifier

class Config(object):
    pass

class Arguments:
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

    def __init__(self):
        self.generate = None
        self.cpms = None
        self.csr = None
        self.jtag = None
        self.gen_fw = None
        self.prod_fw = None
        self.vendor_id = None
        self.vendor_name = None
        self.product_id = None
        self.product_name = None
        self.part_number = None
        self.hw_version = None
        self.hw_version_str = None
        self.discriminator = None
        self.spake2p = None
        self.unique_id = None
        self.manufacturing_date = None
        self.rendezvous_flags = None

    def parse(self):
        parser = argparse.ArgumentParser(description='CPMS Provisioner')

        # NOTE: Don't put defaults here, they precedence over the JSON configuration, making it inefective

        parser.add_argument('-c', '--config', type=str, help='[string] Path to configuration file.')
        parser.add_argument('-g', '--generate', action='store_true', help='[boolean] Generate certificates.', default=None)
        parser.add_argument('-m', '--cpms', action='store_true', help='[string] Generate JSON file only', default=None)
        parser.add_argument('-r', '--csr', action='store_true', help='[boolean] Generate Certificate Signing Request', default=None)
        parser.add_argument('-j', '--jtag', type=str, help='[string] JTAG serial number.')
        parser.add_argument('-gf', '--gen_fw', type=str, help='[string] Path to the Generator Firmware image')
        parser.add_argument('-pf', '--prod_fw', type=str, help='[string] Path to the Production Firmware image')

        parser.add_argument('-v',  '--vendor_id', type=parseInt, help='[int] vendor ID')
        parser.add_argument('-V',  '--vendor_name', type=str, help='[string] vendor name')
        parser.add_argument('-p',  '--product_id', type=parseInt, help='[int] Product ID')
        parser.add_argument('-P',  '--product_name', type=str, help='[string] product name')
        parser.add_argument('-pl', '--product_label', type=str, help='[string] Provide the product label [optional]')
        parser.add_argument('-pu', '--product_url', type=str, help='[string] Provide the product url [optional]')
        parser.add_argument('-pn', '--part_number', type=str, help='[string] Part number')
        parser.add_argument('-hv', '--hw_version', type=parseInt, help='[int] Hardware version value')
        parser.add_argument('-hs', '--hw_version_str', type=str, help='[string] Hardware version string')
        parser.add_argument('-md', '--manufacturing_date', type=str, help='[string] Manufacturing date in YYYY-MM-DD format')
        parser.add_argument('-u',  '--unique_id', type=str,help='[hex_string] 128 bits hex string unique id (without 0x)')
        parser.add_argument('-cf', '--commissioning_flow', type=parseInt, help='[int] Commissioning Flow: 0=Standard, 1=kUserActionRequired, 2=Custom (Default:Standard)')
        parser.add_argument('-rf', '--rendezvous_flags', type=parseInt, help='[int] Rendez-vous flag: 1=SoftAP, 2=BLE 4=OnNetwork (Default=BLE Only)')
        parser.add_argument('-d',  '--discriminator', type=parseInt, help='[int] BLE pairing discriminator.')

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

        parser.add_argument('-sv', '--spake2p_verifier', type=str, help='[string] SPAKE2+ verifier without generating it')
        parser.add_argument('-sp', '--spake2p_passcode', type=parseInt, help='[int] Default PASE session passcode')
        parser.add_argument('-ss', '--spake2p_salt', type=str, help='[string] Provide SPAKE2+ salt')
        parser.add_argument('-si', '--spake2p_iterations', type=parseInt, help='[int] SPAKE2+ iteration count')

        args = parser.parse_args()
        if args.config:
            return self.imports(args.config, args)
        else:
            return self.validate(args)


    def validate(self, args):
        self.generate = args.generate
        if self.generate is None:
            self.generate = False

        self.cpms = args.cpms
        if self.cpms is None:
            self.cpms = False

        self.csr = args.csr
        if self.csr is None:
            self.csr = False

        self.jtag = args.jtag
        self.gen_fw = args.gen_fw
        self.prod_fw = args.prod_fw

        if args.vendor_id is None:
            fail("Missing Vendor ID (--vendor_id)")
        self.vendor_id = args.vendor_id

        if args.vendor_name:
            assert (len(args.vendor_name) <= Arguments.kMaxVendorNameLength), "Vendor name exceeds the size limit"
        self.vendor_name = args.vendor_name

        if args.product_id is None:
            fail("Missing Product ID (--product_id)")
        self.product_id = args.product_id

        if args.product_name:
            assert (len(args.product_name) <= Arguments.kMaxProductNameLength), "Product name exceeds the size limit"
        self.product_name = args.product_name

        if args.product_label:
            assert (len(args.product_label) <= Arguments.kMaxProductLabelLength), "Product Label exceeds the size limit"
        self.product_label = args.product_label

        if args.product_url:
            assert (len(args.product_url) <= Arguments.kMaxProductUrlLenght), "Product URL exceeds the size limit"
        self.product_url = args.product_url

        if args.part_number:
            assert (len(args.part_number) <= Arguments.kMaxPartNumberLength), "Part number exceeds the size limit"
        self.part_number = args.part_number

        if args.hw_version_str:
            assert (len(args.hw_version_str) <= Arguments.kMaxHardwareVersionStringLength), "Hardware version string exceeds the size limit"
        self.hw_version_str = args.hw_version_str

        if args.manufacturing_date:
            print("DATE: '{}'".format(args.manufacturing_date))
            try:
                # self.manufacturing_date = date.fromisoformat(args.manufacturing_date)
                datetime.strptime(args.manufacturing_date, '%Y-%m-%d')
            except ValueError:
                raise ValueError("Incorrect manufacturing data format, should be YYYY-MM-DD")
            self.manufacturing_date = args.manufacturing_date
        else:
            self.manufacturing_date = datetime.now().strftime('%Y-%m-%d')

        if args.unique_id:
            assert (len(bytearray.fromhex(args.unique_id)) == Arguments.kUniqueIDLength), "Provide a 16 bytes rotating id"
        self.unique_id = args.unique_id

        if args.commissioning_flow is None:
            args.commissioning_flow = Arguments.kDefaultCommissioningFlow
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
        self.discriminator = args.discriminator

        # Attestation Files
        self.attest = AttestationArguments()
        self.attest.validate(args)

        # SPAKE2+
        self.spake2p = Spake2pArguments()
        self.spake2p.validate(args)

        return True

    def exports(self, filename):

        d = {
            'generate': self.generate,
            'cpms': self.cpms,
            'csr': self.csr,
            'jtag': self.jtag,
            'gen_fw': self.gen_fw,
            'prod_fw': self.prod_fw,
            'vendor_id': self.vendor_id,
            'vendor_name': self.vendor_name,
            'product_id': self.product_id,
            'product_name': self.product_name,
            'product_label': self.product_label,
            'product_url': self.product_url,
            'part_number': self.part_number,
            'hw_version': self.hw_version,
            'hw_version_str': self.hw_version_str,
            'manufacturing_date': self.manufacturing_date,
            'unique_id': self.unique_id,
            'commissioning_flow': self.commissioning_flow,
            'rendezvous_flags': self.rendezvous_flags,
            'discriminator': self.discriminator,        
            'attestation': {
                'cert_tool': self.attest.cert_tool,
                'key_id': self.attest.key_id,
                'pkcs12': self.attest.pkcs12,
                'key_pass': self.attest.key_pass,
                'paa_cert': self.attest.paa_cert,
                'paa_key': self.attest.paa_key,
                'pai_cert': self.attest.pai_cert,
                'pai_key': self.attest.pai_key,
                'dac_cert': self.attest.dac_cert,
                'dac_key': self.attest.dac_key,
                'certification': self.attest.cd,
                'common_name': self.attest.cn,
            },
            'spake2p': {
                'verifier': self.spake2p.verifier,
                'passcode': self.spake2p.passcode,
                'salt': self.spake2p.salt,
                'iterations': self.spake2p.iterations,
            },
        }
        with open(filename, 'w') as outfile:
            json.dump(d, outfile, indent=4)

    def imports(self, filename, args):

        if not os.path.exists(filename):
            fail("Invalid configuration path: '{}'".format(filename))
        
        d = {}
        with open(filename, 'r') as f:
            d = json.loads(f.read())

        print("\n{}\n".format(json.dumps(d, indent=True)))
        attest = ('attestation' in d and d['attestation']) or {}
        spake = ('spake2p' in d and d['spake2p']) or {}
        conf = Config()
        conf.generate = self.config(d, 'generate', args.generate)
        conf.cpms = self.config(d, 'cpms', args.cpms)
        conf.csr = self.config(d, 'csr', args.csr)
        conf.jtag = self.config(d, 'jtag', args.jtag)
        conf.gen_fw = self.config(d, 'gen_fw', args.gen_fw)
        conf.prod_fw = self.config(d, 'prod_fw', args.prod_fw)
        conf.vendor_id = self.config(d, 'vendor_id', args.vendor_id)
        conf.vendor_name = self.config(d, 'vendor_name', args.vendor_name)
        conf.product_id = self.config(d, 'product_id', args.product_id)
        conf.product_name = self.config(d, 'product_name', args.product_name)
        conf.product_label = self.config(d, 'product_label', args.product_label)
        conf.product_url = self.config(d, 'product_url', args.product_url)
        conf.part_number = self.config(d, 'part_number', args.part_number)
        conf.hw_version = self.config(d, 'hw_version', args.hw_version)
        conf.hw_version_str = self.config(d, 'hw_version_str', args.hw_version_str)
        conf.manufacturing_date = self.config(d, 'manufacturing_date', args.manufacturing_date)
        conf.unique_id = self.config(d, 'unique_id', args.unique_id)
        conf.commissioning_flow = self.config(d, 'commissioning_flow', args.commissioning_flow, Arguments.kDefaultCommissioningFlow)
        conf.rendezvous_flags = self.config(d, 'rendezvous_flags', args.rendezvous_flags, Arguments.kDefaultRendezvousFlags)
        conf.discriminator = self.config(d, 'discriminator', args.discriminator)      
        # Attestation
        conf.cert_tool = self.config(attest, 'cert_tool', args.cert_tool)
        conf.key_id = self.config(attest, 'key_id', args.key_id, AttestationArguments.kDefaultKeyId)
        conf.att_certs = self.config(attest, 'pkcs12', args.att_certs)
        conf.key_pass = self.config(attest, 'key_pass', args.key_pass)
        conf.paa_cert = self.config(attest, 'paa_cert', args.paa_cert)
        conf.paa_key = self.config(attest, 'paa_key', args.paa_key)
        conf.pai_cert = self.config(attest, 'pai_cert', args.pai_cert)
        conf.pai_key = self.config(attest, 'pai_key', args.pai_key)
        conf.dac_cert = self.config(attest, 'dac_cert', args.dac_cert)
        conf.dac_key = self.config(attest, 'dac_key', args.dac_key)
        conf.certification = self.config(attest, 'certification', args.certification)
        conf.common_name = self.config(attest, 'common_name', args.common_name)
        # SPAKE2+
        conf.spake2p_verifier = self.config(spake, 'verifier', args.spake2p_verifier)
        conf.spake2p_passcode = self.config(spake, 'passcode', args.spake2p_passcode)
        conf.spake2p_salt = self.config(spake, 'salt', args.spake2p_salt)
        conf.spake2p_iterations = self.config(spake, 'iterations', args.spake2p_iterations)
        return self.validate(conf)

    def config(self, d, k, override_val = None, def_value = None):
        if override_val is not None:
            return override_val
        elif k in d:
            return d[k]
        else:
            return def_value

class Paths:

    def __init__(self, info, args):
        self.cpms = os.path.normpath(os.path.dirname(__file__))
        self.root = self.cpms + '/..'
        self.debug = self.root + '/out/debug'
        self.temp = self.cpms + '/temp'
        self.out_default = "{}/paa_cert.pem".format(self.cpms)
        self.att_certs = args.attest.paa_cert or "{}/att_certs.pfx".format(self.temp)
        self.paa_cert_pem = args.attest.paa_cert or "{}/paa_cert.pem".format(self.temp)
        self.paa_cert_der = "{}/paa_cert.der".format(self.temp)
        self.paa_key_pem = args.attest.paa_key or "{}/paa_key.pem".format(self.temp)
        self.paa_key_der = "{}/paa_key.der".format(self.temp)
        self.pai_cert_pem = "{}/pai_cert.pem".format(self.temp)
        self.pai_cert_der = "{}/pai_cert.der".format(self.temp)
        self.pai_key_pem = "{}/pai_key.pem".format(self.temp)
        self.pai_key_der = "{}/pai_key.der".format(self.temp)
        self.dac_cert_pem = "{}/dac_cert.pem".format(self.temp)
        self.dac_cert_der = "{}/dac_cert.der".format(self.temp)
        self.dac_key_pem = "{}/dac_key.pem".format(self.temp)
        self.dac_key_der = "{}/dac_key.der".format(self.temp)
        self.cert_tool = os.path.normpath("{}/chip-cert".format(self.debug))
        self.config = "{}/config/latest.json".format(self.cpms)
        self.cd = "{}/cd.der".format(self.temp)
        self.csr_pem = self.temp + '/csr.pem'
        self.gen_fw = "{}/images/{}.s37".format(self.cpms, info.family)
        self.template = "{}/silabs_creds.tmpl".format(self.cpms)
        self.header = "{}/silabs_creds.h".format(self.temp)
        execute(["mkdir", "-p", self.temp ])


def generateSPAKE2pVerifier(args, paths):
    print("\n◆ SPAKE2+ Verifier")
    try:
        salt = base64.b64decode(args.spake2p.salt)
    except:
        fail("Invalid SPAKE2+ salt (base64): {}".format(args.spake2p.salt))
    salt_length = len(salt)
    if salt_length < Spake2pArguments.kSaltMin:
        fail("Invalid SPAKE2+ salt length: {} < {}".format(salt_length, Spake2pArguments.kSaltMin))
    if salt_length > Spake2pArguments.kSaltMax:
        fail("Invalid SPAKE2+ salt length: {} > {}".format(salt_length, Spake2pArguments.kSaltMax))
    WS_LENGTH = NIST256p.baselen + 8
    ws = hashlib.pbkdf2_hmac('sha256', struct.pack('<I', args.spake2p.passcode), salt, args.spake2p.iterations, WS_LENGTH * 2)
    w0 = int.from_bytes(ws[:WS_LENGTH], byteorder='big') % NIST256p.order
    w1 = int.from_bytes(ws[WS_LENGTH:], byteorder='big') % NIST256p.order
    L = NIST256p.generator * w1
    verifier = w0.to_bytes(NIST256p.baselen, byteorder='big') + L.to_bytes('uncompressed')
    args.spake2p.verifier = base64.b64encode(verifier).decode('ascii')
    print("  ∙ pass: {}\n  ∙ salt: {}\n  ∙ iter: {}\n  ▪︎ {}"
        .format(args.spake2p.passcode, args.spake2p.salt, args.spake2p.iterations, args.spake2p.verifier))


def collectCerts(args, paths):
    # CD
    if args.attest.cd and os.path.exists(args.attest.cd) and not os.path.exists(paths.cd):
        execute(['cp', args.attest.cd, paths.cd])
    # PKCS#12
    if args.attest.pkcs12 and os.path.exists(args.attest.pkcs12) and not os.path.exists(paths.att_certs):
        execute(['cp', args.attest.pkcs12, paths.att_certs])
    # PAI
    x509Copy('cert', args.attest.pai_cert, paths.temp, 'pai_cert')
    # DAC
    x509Copy('cert', args.attest.dac_cert, paths.temp, 'dac_cert')
    x509Copy('key', args.attest.dac_key, paths.temp, 'dac_key')


def generateCerts(args, paths):

    serial_num = generateSerial()
    vid = hex(args.vendor_id)
    pid = hex(args.product_id)
    version = 0x101
    security_level = 0
    security_info = 0
    lifetime = 3660

    generate_cd = not os.path.exists(paths.cd)
    generate_paa = not os.path.exists(paths.paa_cert_pem)
    generate_pai = generate_paa or not os.path.exists(paths.pai_cert_pem)
    generate_dac = generate_paa or generate_pai or not os.path.exists(paths.dac_cert_pem)

    cert_tool = args.attest.cert_tool or paths.cert_tool
    if not os.path.exists(cert_tool):
        fail("Invalid chip-cert path (--cert_tool): '{}'".format(cert_tool))

    if generate_cd:
        # Generate CD
        signing_cert = "{}/credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem".format(paths.root)
        signing_key = "{}/credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem".format(paths.root)
        execute([ cert_tool, 'gen-cd', '-f', '1', '-V', vid, '-p', pid, '-d', '0x0016', '-c', 'ZIG20142ZB330003-24', '-l', security_level, '-i', security_info, '-n', version, '-t', '0', '-o', vid, '-r' , pid, '-C', signing_cert, '-K', signing_key, '-O', paths.cd ])
    if generate_paa:
        # Generate PAA
        execute([ 'rm', '-f', paths.paa_cert_pem ])
        execute([ cert_tool, 'gen-att-cert', '-t', 'a', '-l', lifetime, '-c', '"Matter PAA"', '-V', vid, '-o', paths.paa_cert_pem, '-O', paths.paa_key_pem])
        x509Translate('cert', paths.paa_cert_pem)
        x509Translate('key', paths.paa_key_pem)
    if generate_pai:
        # Generate PAI
        execute([ 'rm', '-f', paths.pai_cert_pem ])
        execute([ cert_tool, 'gen-att-cert', '-t', 'i', '-l', lifetime, '-c', '"Matter PAI"', '-V', vid, '-P', pid, '-C', paths.paa_cert_pem, '-K', paths.paa_key_pem, '-o', paths.pai_cert_pem, '-O', paths.pai_key_pem])
        x509Translate('cert', paths.pai_cert_pem)
        x509Translate('key', paths.pai_key_pem)
    if generate_dac:
        # Generate DAC
        execute([ 'rm', '-f', paths.dac_cert_pem ])
        execute([ cert_tool, 'gen-att-cert', '-t', 'd', '-l', lifetime, '-c', '"Matter DAC"', '-V', vid, '-P', pid, '-C', paths.pai_cert_pem, '-K', paths.pai_key_pem, '-o', paths.dac_cert_pem, '-O', paths.dac_key_pem])
        x509Translate('cert', paths.dac_cert_pem)
        x509Translate('key', paths.dac_key_pem)
    if generate_paa or generate_pai or generate_dac:
        # Generate PKCS#12 file
        pass_arg = "pass:\"{}\"".format(args.attest.key_pass);
        execute([ 'openssl', 'pkcs12', '-export', '-inkey', paths.dac_key_pem, '-in', paths.dac_cert_pem, '-certfile', paths.pai_cert_pem, '-out', paths.att_certs, '-password', pass_arg ])
        x509Translate('cert', paths.dac_cert_pem)
        x509Translate('key', paths.dac_key_pem)


def x509Copy(type, in_path, out_dir, out_name):
    if in_path is None:
        return False
    (in_dir, in_base) = os.path.split(in_path)
    (in_root, in_ext) = os.path.splitext(in_base)
    out_path = "{}/{}{}".format(out_dir, out_name, in_ext)
    if not os.path.samefile(in_path, out_path):
        execute(['cp', in_path, out_path])
    return x509Translate(type, out_path)


def x509Translate(type, in_path):
    (in_base, in_ext) = os.path.splitext(in_path)
    in_ext = in_ext.strip('.')
    # Translate only PEM to DER
    if 'pem' == in_ext:
        out_ext = 'der'
        out_path = "{}.{}".format(in_base, out_ext)
        if 'cert' == type:
            execute(['openssl', 'x509', '-outform', out_ext, '-in', in_path, '-out', out_path])
        elif 'key' == type:
            execute(['openssl', 'ec', '-inform', in_ext, '-in', in_path, '-outform', out_ext, '-out', out_path])
        else:
            fail("Invalid X.509 type: {}".format(type))

def parsePKCSCerts(certs):
    BEGIN = '-----BEGIN CERTIFICATE-----'
    END = '-----END CERTIFICATE-----'
    offset = 0
    size = len(certs)
    cert_list = []
    while offset < size:
        b = certs.find(BEGIN, offset)
        if b < 0:
            break;
        e = certs.find(END, b)
        assert(e > 0)
        e += len(END)
        cert_list.append(certs[b:e])
        offset = e + 1
    return cert_list


def generateAttestation(serial, args, paths):

    # Generate CSR, DAC
    print("\n◆ Credentials: CSR\n")
    step = CsrCommand(args.attest.cn, args.vendor_id, args.product_id, args.attest.key_id)
    (key_id, csr) = step.execute(serial)
    # Write CSR to file

    # Generate DAC
    print("\n◆ Credentials: DAC\n")
    signer = SigningServer(paths.cpms)
    (pai_path, dac_path) = signer.sign(csr)
    x509Copy('cert', pai_path, paths.temp, 'pai_cert')
    x509Copy('cert', dac_path, paths.temp, 'dac_cert')
    return key_id

def importAttestation(serial, args, paths):
    print("\n◆ Credentials: Import\n")
    if args.attest.pkcs12 is not None:
        # Extract key from PKCS#12
        password_arg = "pass:{}".format(args.attest.key_pass)
        ps = subprocess.Popen(('openssl', 'pkcs12', '-nodes', '-nocerts', '-in', args.attest.pkcs12, '-passin', password_arg), stdout=subprocess.PIPE)
        subprocess.check_output(('openssl', 'ec', '-outform', 'der', '-out', paths.dac_key_der), stdin=ps.stdout)
        
        # Extract certificates from PKCS#12
        out = execute([ 'openssl', 'pkcs12', '-nodes', '-nokeys', '-in', args.attest.pkcs12, '-passin', password_arg ], True, True)
        # Parse certificates
        certs = parsePKCSCerts(out.decode("utf-8"))
        with open(paths.dac_cert_pem, 'w') as f:
            f.write(certs[0])
        with open(paths.pai_cert_pem, 'w') as f:
            f.write(certs[1])
        # Convert to DER
        x509Copy('cert', paths.pai_cert_pem, paths.temp, 'pai_cert')
        x509Copy('cert', paths.dac_cert_pem, paths.temp, 'dac_cert')

    step = ImportCommand(ImportCommand.KEY, args.attest.key_id, paths.dac_key_der)
    (key_id, key_offset, key_size) = step.execute(serial)
    return key_id

def writeAttestation(serial, paths, info, key_id):
    print("\n◆ Credentials: Write \n")
    step = ImportCommand(ImportCommand.DAC, key_id, paths.dac_cert_der)
    (key_id, dac_offset, dac_size) = step.execute(serial)
    step = ImportCommand(ImportCommand.PAI, key_id,  paths.pai_cert_der)
    (key_id, pai_offset, pai_size) = step.execute(serial)
    step = ImportCommand(ImportCommand.CD, key_id, paths.cd, True)
    (key_id, cd_offset, cd_size) = step.execute(serial)

    # Generate header (for backwards compatibility with silabs_examples/credentials/creds.py)
    print("\n◆ Credentials: silabs_creds.h (legacy)")
    with open(paths.template, 'r') as tf:
        header = tf.read()
        header = header.replace('{{key_id}}', hex(key_id))
        header = header.replace('{{dac_offset}}', hex(dac_offset))
        header = header.replace('{{dac_size}}', str(dac_size))
        header = header.replace('{{pai_offset}}', hex(pai_offset))
        header = header.replace('{{pai_size}}', str(pai_size))
        header = header.replace('{{cd_offset}}', hex(cd_offset))
        header = header.replace('{{cd_size}}', str(cd_size))
        # Write header
        with open(paths.header, 'w') as hf:
            hf.write(header)

#-------------------------------------------------------------------------------
# Main
#-------------------------------------------------------------------------------

def main(argv):
    # Process arguments
    args = Arguments()
    args.parse()

    # Gather device info
    cmmd = Commander(args.jtag)
    info = cmmd.info()
    paths = Paths(info, args)
    print("\n◆ Device Info:\n{}".format(info))

    print("\n◆ Prepare")
    # Collect/Generate certificates
    if (args.spake2p.verifier is None):
        generateSPAKE2pVerifier(args, paths)
    # Generate/Import Attestation Credentials
    collectCerts(args, paths)
    if args.generate:
        generateCerts(args, paths)
    # Export configuration to JSON
    args.exports(paths.config)
    if args.cpms:
        exit()

    print("\n◆ Loading Generator Firmware")
    serial = SerialComm(info.part, args.jtag)
    # Option 1: Load into RAM
    # with open(paths.gen_fw, 'rb') as f:
    #     generator_image = f.read()
    # serial.open()
    # serial.reset(True)
    # ram_addr = chip.ram_addr
    # stack_addr = ram_addr + chip.stacksize
    # serial.flash(ram_addr, stack_addr, generator_image)  
    # serial.close()
    # Option 2: Load into FLASH
    cmmd.flash(args.gen_fw or paths.gen_fw)

    # Initialize device
    step = InitCommand(info)
    step.execute(serial)

    # Write Attestation Credentials
    if args.csr:
        key_id = generateAttestation(serial, args, paths)
        writeAttestation(serial, paths, info, key_id)
    else:
        key_id = importAttestation(serial, args, paths)
        writeAttestation(serial, paths, info, key_id)

    # Write Factory Data
    print("\n◆ Write Factory Data\n")
    step = SetupCommand(args)
    step.execute(serial)

    # Flash Production Firmware
    if args.prod_fw:
        print("\n◆ Write app\n")
        cmmd.flash(args.prod_fw)

main(sys.argv[1:])
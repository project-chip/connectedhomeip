#! /usr/bin/python3
import os
import sys

if "STUDIO_PYTHONPATH" in os.environ.keys():
    [sys.path.append(aPath) for aPath in os.environ["STUDIO_PYTHONPATH"].split(os.pathsep)]

from modules.arguments import *
from modules.connection import *
from modules.commander import *
from modules.commands import *
from modules.exporter import *
from modules.util import *
from modules.signing_server import *
from ecdsa.curves import NIST256p
import subprocess
import base64
import hashlib
import struct
import shutil


def toOsPath(path):
    if path == "" or path == None:
        return ""
    return os.path.abspath(os.path.normpath(path))


def copyFile(src, dest):
    shutil.copy(src, dest)

class Paths:

    kDefaultToolPath = '../out/tools/chip-cert'

    def __init__(self, info, args):
        base = toOsPath(os.path.normpath(os.path.dirname(__file__)))
        self.root = toOsPath(base + '/..')
        self.debug = toOsPath(self.root + '/out/debug')
        self.temp = toOsPath(args.temp or (base + '/temp'))
        self.support = toOsPath("{}/support".format(base))
        self.out_default = toOsPath("{}/paa_cert.pem".format(base))
        self.att_certs = toOsPath(args.attest.paa_cert or "{}/certs.p12".format(self.temp))
        self.paa_cert_pem = toOsPath(args.attest.paa_cert or "{}/paa_cert.pem".format(self.temp))
        self.paa_cert_der = toOsPath("{}/paa_cert.der".format(self.temp))
        self.paa_key_pem = toOsPath(args.attest.paa_key or "{}/paa_key.pem".format(self.temp))
        self.paa_key_der = toOsPath("{}/paa_key.der".format(self.temp))
        self.pai_cert_pem = toOsPath("{}/pai_cert.pem".format(self.temp))
        self.pai_cert_der = toOsPath("{}/pai_cert.der".format(self.temp))
        self.pai_key_pem = toOsPath("{}/pai_key.pem".format(self.temp))
        self.pai_key_der = toOsPath("{}/pai_key.der".format(self.temp))
        self.dac_cert_pem = toOsPath("{}/dac_cert.pem".format(self.temp))
        self.dac_cert_der = toOsPath("{}/dac_cert.der".format(self.temp))
        self.dac_key_pem = toOsPath("{}/dac_key.pem".format(self.temp))
        self.dac_key_der = toOsPath("{}/dac_key.der".format(self.temp))
        self.cert_tool = toOsPath(os.path.normpath(Paths.kDefaultToolPath))
        self.config = toOsPath("{}/config/latest.json".format(base))
        self.cd = toOsPath("{}/cd.der".format(self.temp))
        self.csr_pem = toOsPath(self.temp + '/csr.pem')
        self.gen_fw = toOsPath("{}/images/{}".format(base, info.image))
        self.template = toOsPath("{}/silabs_creds.tmpl".format(base))
        self.header = toOsPath("{}/silabs_creds.h".format(self.temp))
        if not os.path.isdir(self.temp):
            os.makedirs(self.temp)
        # execute(["mkdir", "-p", self.temp ])


def generateSPAKE2pVerifier(args, paths):
    print("\n◆ SPAKE2+ Verifier")
    salt = base64.b64decode(args.spake2p.salt)
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
    args.spake2p.verifier = base64.b64encode(verifier).decode('utf-8')
    print("  ∙ pass: {}\n  ∙ salt: {}\n  ∙ iter: {}\n  ▪︎ {}"
        .format(args.spake2p.passcode, args.spake2p.salt, args.spake2p.iterations, args.spake2p.verifier))

def collectCerts(args, paths):
    # CD
    if args.attest.cd and os.path.exists(args.attest.cd) and (not os.path.exists(paths.cd) or not os.path.samefile(args.attest.cd, paths.cd)):
        copyFile(args.attest.cd, paths.cd)

    # PKCS#12
    if args.attest.pkcs12 is not None:
        copyFile(args.attest.pkcs12, paths.att_certs)

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
    else:
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
    out_path = toOsPath("{}/{}{}".format(out_dir, out_name, in_ext))
    if (not os.path.exists(out_path)) or (os.path.exists(in_path) and os.path.exists(out_path) and not os.path.samefile(in_path, out_path)):
        copyFile(in_path, out_path)
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


def generateAttestation(conn, args, paths):
    # Generate CSR, DAC
    print("\n◆ Credentials: CSR\n")
    step = CsrCommand(args.attest.cn, args.vendor_id, args.product_id, args.attest.key_id)
    (key_id, csr) = step.execute(conn)
    # Write CSR to file

    # Generate DAC
    print("\n◆ Credentials: DAC\n")
    signer = SigningServer(paths.stop)
    (pai_path, dac_path) = signer.sign(csr)
    x509Copy('cert', pai_path, paths.temp, 'pai_cert')
    x509Copy('cert', dac_path, paths.temp, 'dac_cert')
    return key_id

def importAttestation(conn, args, paths):
    print("\n◆ Credentials: Import\n")
    step = ImportCommand(ImportCommand.KEY, args.attest.key_id, paths.dac_key_der)
    (key_id, key_offset, key_size) = step.execute(conn)
    return key_id

def writeAttestation(conn, paths, info, key_id):
    print("\n◆ Credentials: Write \n")
    step = ImportCommand(ImportCommand.DAC, key_id, paths.dac_cert_der)
    (key_id, dac_offset, dac_size) = step.execute(conn)
    step = ImportCommand(ImportCommand.PAI, key_id,  paths.pai_cert_der)
    (key_id, pai_offset, pai_size) = step.execute(conn)
    step = ImportCommand(ImportCommand.CD, key_id, paths.cd, True)
    (key_id, cd_offset, cd_size) = step.execute(conn)

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
    args.load()

    # Gather device info
    cmmd = Commander(args.conn)
    info = cmmd.info()
    paths = Paths(info, args)
    if args.part_number is None:
        args.part_number = info.part
    print("\n◆ Device Info:\n{}".format(info))

    # Flash Production Firmware
    print("\n◆ Writing firmware\n")
    cmmd.flash(args.gen_fw or paths.gen_fw)

    print("\n◆ Preparing credentials")
    # Collect/Generate certificates
    if (args.spake2p.verifier is None):
        generateSPAKE2pVerifier(args, paths)

    # Generate/Import Attestation Credentials
    collectCerts(args, paths)
    if args.generate:
        generateCerts(args, paths)
    # Export configuration to JSON
    args.write(paths.config)
    # Export configuration to binary
    if args.binary:
        e = Exporter()
        e.export(args, paths)
    if args.stop or args.binary:
        exit()

    print("\n◆ Connecting to device")
    conn = Connection(args, paths, info.part)
    conn.open(args.conn)

    # Initialize device
    step = InitCommand(info)
    step.execute(conn)

    # Write Attestation Credentials
    if args.csr:
        key_id = generateAttestation(conn, args, paths)
        writeAttestation(conn, paths, info, key_id)
    else:
        key_id = importAttestation(conn, args, paths)
        writeAttestation(conn, paths, info, key_id)

    # Write Factory Data
    print("\n◆ Write Factory Data\n")
    step = SetupCommand(args)
    step.execute(conn)
    conn.close()

    # Flash Production Firmware
    if args.prod_fw:
        print("\n◆ Write app\n")
        cmmd.flash(args.prod_fw)

main(sys.argv[1:])
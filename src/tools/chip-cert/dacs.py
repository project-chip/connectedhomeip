#!/usr/bin/env python

import argparse
import os
import subprocess
import sys
import typing
from binascii import hexlify, unhexlify
from enum import Enum

copyrightNotice = """/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
"""


def bytes_from_hex(hex: str) -> bytes:
    """Converts any `hex` string representation including `01:ab:cd` to bytes

    Handles any whitespace including newlines, which are all stripped.
    """
    return unhexlify("".join(hex.replace(":", "").split()))


def make_c_array(byte_string: bytes, name: str, linelen: int) -> str:
    """Convert a large byte string to a named constant C/C++ uint8_t array. """
    def _extract_front(b: bytes, length: int) -> bytes:
        to_extract = min(length, len(b))
        span = b[0:to_extract]
        del b[0:to_extract]

        return span

    byte_string = bytearray(byte_string)
    output = "const uint8_t %s[%d] = {\n" % (name, len(byte_string))
    while len(byte_string) > 0:
        current_line_bytes = _extract_front(byte_string, linelen)
        output += "    %s,\n" % ", ".join(["0x%02x" %
                                          b for b in current_line_bytes])
    output += "};\n"

    return output


def make_array_header(byte_string: bytes, name: str) -> str:
    """Returns the header define for an array with the given name and size."""
    byte_string = bytearray(byte_string)
    return 'extern const uint8_t ' + name + '[{:d}];\n'.format(len(byte_string))


def open_outfile(filename=None) -> typing.BinaryIO:
    """Opens either a file or stdout for output."""
    if filename:
        fh = open(filename, 'wb')
    else:
        fh = sys.stdout

    try:
        yield fh
    finally:
        if fh is not sys.stdout:
            fh.close()


class CertType(Enum):
    PAA = 1
    PAI = 2
    DAC = 3


class Names:
    def __init__(self, cert_type: CertType, test_dir, dev_dir, pid):
        prefixes = {CertType.PAA: test_dir + '/Chip-Test-PAA-FFF1-',
                    CertType.PAI: dev_dir + '/Matter-Development-PAI-noPID-',
                    CertType.DAC: dev_dir + '/Matter-Development-DAC-{:X}-'.format(pid)}
        array_names_prefix = {CertType.PAA: 'kTestPAA_',
                              CertType.PAI: 'kDevelopmentPAI_',
                              CertType.DAC: 'kDevelopmentDAC_'}
        array_names_suffix = {CertType.PAA: '',
                              CertType.PAI: '_FFF1',
                              CertType.DAC: '_FFF1_{:X}'.format(pid)}
        generic_prefix = {CertType.PAA: 'kPaa',
                          CertType.PAI: 'kPai',
                          CertType.DAC: 'kDac'}

        prefix = prefixes[cert_type]
        array_name_prefix = array_names_prefix[cert_type]
        array_name_suffix = array_names_suffix[cert_type]

        self.cert_pem = prefix + 'Cert.pem'
        self.cert_der = prefix + 'Cert.der'
        self.key_pem = prefix + 'Key.pem'
        self.key_der = prefix + 'Key.der'
        self.public_key_array_name = array_name_prefix + 'PublicKey' + array_name_suffix
        self.private_key_array_name = array_name_prefix + 'PrivateKey' + array_name_suffix
        self.cert_array_name = array_name_prefix + 'Cert' + array_name_suffix
        self.generic_prefix = generic_prefix[cert_type]


class DevCertBuilder:
    def __init__(self, cert_type: CertType, dev_dir: str, test_dir: str, chip_cert_dir: str, pid: int):
        self.pid = pid
        self.cert_type = cert_type
        self.chipcert = chip_cert_dir + 'chip-cert'

        if not os.path.exists(self.chipcert):
            raise Exception('Path not found: %s' % self.chipcert)

        paa = Names(CertType.PAA, test_dir, dev_dir, pid)
        pai = Names(CertType.PAI, test_dir, dev_dir, pid)
        dac = Names(CertType.DAC, test_dir, dev_dir, pid)
        if cert_type == CertType.PAI:
            self.signer = paa
            self.own = pai
        if cert_type == CertType.DAC:
            self.signer = pai
            self.own = dac

    def make_certs_and_keys(self) -> None:
        """Creates the PEM and DER certs and keyfiles"""
        if self.cert_type == CertType.PAI:
            subject_name = 'Matter Dev PAI 0xFFF1 no PID'
            pid_flag = ''
            type_flag = '-t i'
            vidpid_fallback_encoding_flag = ''
        elif self.cert_type == CertType.DAC:
            subject_name = 'Matter Dev DAC 0xFFF1/0x{:X}'.format(self.pid)
            pid_flag = '-P 0x{:X}'.format(self.pid)
            type_flag = '-t d'
            # For a subset of DACs with PIDs in a range [0x8010, 0x8014]
            # use alternative (fallback) PID/VID encoding method.
            if self.pid >= 0x8010 and self.pid <= 0x8014:
                vidpid_fallback_encoding_flag = ' -a'
            else:
                vidpid_fallback_encoding_flag = ''
        else:
            return

        cmd = self.chipcert + ' gen-att-cert ' + type_flag + ' -c "' + subject_name + '" -C ' + self.signer.cert_pem + ' -K ' + \
            self.signer.key_pem + ' -V 0xFFF1 ' + pid_flag + vidpid_fallback_encoding_flag + \
            ' -l 4294967295 -o ' + self.own.cert_pem + ' -O ' + self.own.key_pem
        subprocess.run(cmd, shell=True)
        cmd = 'openssl x509 -inform pem -in ' + self.own.cert_pem + \
            ' -out ' + self.own.cert_der + ' -outform DER'
        subprocess.run(cmd, shell=True)
        cmd = 'openssl ec -inform pem -in ' + self.own.key_pem + \
            ' -out ' + self.own.key_der + ' -outform DER'
        subprocess.run(cmd, shell=True)

    def get_raw_keys(self) -> tuple[str, str]:
        """Extracts the raw key bytes from the PEM file"""
        cmd = 'openssl ec -inform pem -in ' + self.own.key_pem + ' -text'
        out = subprocess.run(
            cmd, shell=True, capture_output=True).stdout.decode('utf-8')
        priv = ''.join(out[out.find('priv:')+5:out.find('pub:')].split())
        pub = ''.join(out[out.find('pub:')+4:out.find('ASN1')].split())
        return [bytes_from_hex(pub), bytes_from_hex(priv)]

    def full_arrays(self) -> tuple[str, str]:
        """Returns the byte arrays and declarations"""
        with open(self.own.cert_der, "rb") as infile:
            b = infile.read()
            cert = make_c_array(b, self.own.cert_array_name, 21)
            header = make_array_header(b, self.own.cert_array_name)

        [pub, priv] = self.get_raw_keys()
        h_pub = 'extern const uint8_t ' + \
            self.own.public_key_array_name + '[65];\n'
        h_priv = 'extern const uint8_t ' + \
            self.own.private_key_array_name + '[32];\n'

        c = cert + make_c_array(pub, self.own.public_key_array_name, 17) + \
            make_c_array(priv, self.own.private_key_array_name, 16)
        h = header + h_pub + h_priv
        return [h, c]

    def generic_arrays(self) -> tuple[str, str]:
        """Returns the generic refernce for Certs and Keys of this type"""
        h_start = 'extern ByteSpan ' + self.own.generic_prefix
        h = h_start + 'Cert;\n' + h_start + 'PublicKey;\n' + h_start + 'PrivateKey;\n\n'
        c_start = 'ByteSpan ' + self.own.generic_prefix
        cert = c_start + \
            'Cert       = ByteSpan(' + self.own.cert_array_name + ');\n'
        priv = c_start + \
            'PrivateKey = ByteSpan(' + self.own.private_key_array_name + ');\n'
        publ = c_start + \
            'PublicKey  = ByteSpan(' + self.own.public_key_array_name + ');\n'
        c = cert + priv + publ
        return [h, c]

    def headers(self, filename) -> tuple[str, str]:
        """Returns a string with the headers for the output files"""
        includes = '#include <cstdint>\n\n#include <lib/support/Span.h>\n#include <platform/CHIPDeviceConfig.h>\n\n'
        namespaces = 'namespace chip {\nnamespace DevelopmentCerts {\n\n'
        h_top = copyrightNotice + '#pragma once\n' + includes + namespaces

        c_top = copyrightNotice + '#include "' + \
            filename + '.h"\n\n' + includes + namespaces
        return [h_top, c_top]

    def footer(self) -> str:
        """Returns the footer for the output files"""
        return '} // namespace DevelopmentCerts\n} // namespace chip\n'


def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument('-d', '--dev_dir', dest='certdir',
                           default='credentials/development/attestation',
                           help='output directory for PEM and DER files')
    argparser.add_argument('-t', '--test_dir', dest='testdir',
                           default='credentials/test/attestation/', help='directory holding test PAA')
    argparser.add_argument('-o', '--out_dir', dest='outdir',
                           default='src/credentials/examples/', help='Output directory for array files')
    argparser.add_argument('-c', '--chip-cert_dir', dest='chipcertdir',
                           default='out/debug/standalone/', help='Directory where chip-cert tool is located')
    argparser.add_argument('--paiout', dest='paiout', default='ExamplePAI',
                           help='output file name for PAI C arrays (no .h or .cpp extension)')
    argparser.add_argument('--dacout', dest='dacout', default='ExampleDACs',
                           help='output file name for PAI C arrays (no .h or .cpp extension)')

    args = argparser.parse_args()
    builder = DevCertBuilder(CertType.PAI, args.certdir,
                             args.testdir, args.chipcertdir, 0x8000)

    builder.make_certs_and_keys()
    [h_full, c_full] = builder.full_arrays()
    [h_generic, c_generic] = builder.generic_arrays()
    [h_top, c_top] = builder.headers(args.paiout)
    footer = builder.footer()
    with open(args.outdir + args.paiout + '.h', "w") as hfile:
        hfile.write(h_top)
        hfile.write(h_full)
        hfile.write(h_generic)
        hfile.write(footer)
    with open(args.outdir + args.paiout + '.cpp', "w") as cfile:
        cfile.write(c_top)
        cfile.write(c_full)
        cfile.write(c_generic)
        cfile.write(footer)

    with open(args.outdir + args.dacout + '.h', "w") as hfile:
        with open(args.outdir + args.dacout + '.cpp', "w") as cfile:
            builder = DevCertBuilder(
                CertType.DAC, args.certdir, args.testdir, args.chipcertdir, 0x8000)
            [h_top, c_top] = builder.headers(args.dacout)
            [h_generic, c_generic] = builder.generic_arrays()
            footer = builder.footer()

            hfile.write(h_top)
            hfile.write(h_generic)
            cfile.write(c_top)

            for i in range(0x8000, 0x8020):
                builder = DevCertBuilder(
                    CertType.DAC, args.certdir, args.testdir, args.chipcertdir, i)
                builder.make_certs_and_keys()
                [h, c] = builder.full_arrays()
                [h_generic, c_generic] = builder.generic_arrays()

                define = '#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x{:X}\n'.format(
                    i)
                end = '#endif\n'

                hfile.write(define)
                hfile.write(h)
                hfile.write(end)

                cfile.write(define)
                cfile.write(c)
                cfile.write(c_generic)
                cfile.write(end)

            hfile.write(footer)
            cfile.write(footer)


if __name__ == '__main__':
    sys.exit(main())

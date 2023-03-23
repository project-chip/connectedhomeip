#!/usr/bin/env python3

#
#    Copyright (c) 2022 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import argparse
import base64
from ecdsa.curves import NIST256p
import hashlib
import struct

# Forbidden passcodes as listed in the "5.1.7.1. Invalid Passcodes" section of the Matter spec
INVALID_PASSCODES = [00000000,
                     11111111,
                     22222222,
                     33333333,
                     44444444,
                     55555555,
                     66666666,
                     77777777,
                     88888888,
                     99999999,
                     12345678,
                     87654321, ]

# Length of `w0s` and `w1s` elements
WS_LENGTH = NIST256p.baselen + 8


def generate_verifier(passcode: int, salt: bytes, iterations: int) -> bytes:
    ws = hashlib.pbkdf2_hmac('sha256', struct.pack('<I', passcode), salt, iterations, WS_LENGTH * 2)
    w0 = int.from_bytes(ws[:WS_LENGTH], byteorder='big') % NIST256p.order
    w1 = int.from_bytes(ws[WS_LENGTH:], byteorder='big') % NIST256p.order
    L = NIST256p.generator * w1

    return w0.to_bytes(NIST256p.baselen, byteorder='big') + L.to_bytes('uncompressed')


def main():
    def passcode_arg(arg: str) -> int:
        passcode = int(arg)

        if not 0 <= passcode <= 99999999:
            raise argparse.ArgumentTypeError('passcode out of range')

        if passcode in INVALID_PASSCODES:
            raise argparse.ArgumentTypeError('invalid passcode')

        return passcode

    def salt_arg(arg: str) -> bytes:
        salt = base64.b64decode(arg)

        if not 16 <= len(salt) <= 32:
            raise argparse.ArgumentTypeError('invalid salt length')

        return salt

    def iterations_arg(arg: str) -> int:
        iterations = int(arg)

        if not 1000 <= iterations <= 100000:
            raise argparse.ArgumentTypeError('iteration count out of range')

        return iterations

    parser = argparse.ArgumentParser(description='SPAKE2+ Python Tool', fromfile_prefix_chars='@')
    commands = parser.add_subparsers(dest='command', metavar='subcommand'.ljust(16), required=True)

    gen_verifier = commands.add_parser('gen-verifier', help='Generate SPAKE2+ Verifier')
    gen_verifier.add_argument('-p', '--passcode', type=passcode_arg,
                              required=True, help='8-digit passcode')
    gen_verifier.add_argument('-s', '--salt', type=salt_arg,
                              required=True, help='Salt of length 16 to 32 octets encoded in Base64')
    gen_verifier.add_argument('-i', '--iteration-count', type=iterations_arg,
                              metavar='count', required=True, help='Iteration count between 1000 and 100000')

    args = parser.parse_args()

    if args.command == 'gen-verifier':
        verifier = generate_verifier(args.passcode, args.salt, args.iteration_count)
        print(base64.b64encode(verifier).decode('ascii'))


if __name__ == '__main__':
    main()

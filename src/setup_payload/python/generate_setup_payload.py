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
import enum
import sys

import Base38
from bitarray import bitarray
from stdnum.verhoeff import calc_check_digit

# See section 5.1.4.1 Manual Pairing Code in the Matter specification v1.0
MANUAL_DISCRIMINATOR_LEN = 4
PINCODE_LEN = 27

MANUAL_CHUNK1_DISCRIMINATOR_MSBITS_LEN = 2
MANUAL_CHUNK1_DISCRIMINATOR_MSBITS_POS = 0
MANUAL_CHUNK1_VID_PID_PRESENT_BIT_POS = MANUAL_CHUNK1_DISCRIMINATOR_MSBITS_POS + MANUAL_CHUNK1_DISCRIMINATOR_MSBITS_LEN
MANUAL_CHUNK1_LEN = 1

MANUAL_CHUNK2_DISCRIMINATOR_LSBITS_LEN = 2
MANUAL_CHUNK2_PINCODE_LSBITS_LEN = 14
MANUAL_CHUNK2_PINCODE_LSBITS_POS = 0
MANUAL_CHUNK2_DISCRIMINATOR_LSBITS_POS = MANUAL_CHUNK2_PINCODE_LSBITS_POS + MANUAL_CHUNK2_PINCODE_LSBITS_LEN
MANUAL_CHUNK2_LEN = 5

MANUAL_CHUNK3_PINCODE_MSBITS_LEN = 13
MANUAL_CHUNK3_PINCODE_MSBITS_POS = 0
MANUAL_CHUNK3_LEN = 4

MANUAL_VID_LEN = 5
MANUAL_PID_LEN = 5

# See section 5.1.3. QR Code in the Matter specification v1.0
QRCODE_VERSION_LEN = 3
QRCODE_DISCRIMINATOR_LEN = 12
QRCODE_VID_LEN = 16
QRCODE_PID_LEN = 16
QRCODE_COMMISSIONING_FLOW_LEN = 2
QRCODE_DISCOVERY_CAP_BITMASK_LEN = 8
QRCODE_PADDING_LEN = 4
QRCODE_VERSION = 0
QRCODE_PADDING = 0

INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444, 55555555,
                     66666666, 77777777, 88888888, 99999999, 12345678, 87654321]


class CommissioningFlow(enum.IntEnum):
    Standard = 0,
    UserIntent = 1,
    Custom = 2


class SetupPayload:
    def __init__(self, discriminator, pincode, rendezvous=4, flow=CommissioningFlow.Standard, vid=0, pid=0):
        self.long_discriminator = discriminator
        self.short_discriminator = discriminator >> 8
        self.pincode = pincode
        self.rendezvous = rendezvous
        self.flow = flow
        self.vid = vid
        self.pid = pid

    def manual_chunk1(self):
        discriminator_shift = (MANUAL_DISCRIMINATOR_LEN - MANUAL_CHUNK1_DISCRIMINATOR_MSBITS_LEN)
        discriminator_mask = (1 << MANUAL_CHUNK1_DISCRIMINATOR_MSBITS_LEN) - 1
        discriminator_chunk = (self.short_discriminator >> discriminator_shift) & discriminator_mask
        vid_pid_present_flag = 0 if self.flow == CommissioningFlow.Standard else 1
        return (discriminator_chunk << MANUAL_CHUNK1_DISCRIMINATOR_MSBITS_POS) | (vid_pid_present_flag << MANUAL_CHUNK1_VID_PID_PRESENT_BIT_POS)

    def manual_chunk2(self):
        discriminator_mask = (1 << MANUAL_CHUNK2_DISCRIMINATOR_LSBITS_LEN) - 1
        pincode_mask = (1 << MANUAL_CHUNK2_PINCODE_LSBITS_LEN) - 1
        discriminator_chunk = self.short_discriminator & discriminator_mask
        return ((self.pincode & pincode_mask) << MANUAL_CHUNK2_PINCODE_LSBITS_POS) | (discriminator_chunk << MANUAL_CHUNK2_DISCRIMINATOR_LSBITS_POS)

    def manual_chunk3(self):
        pincode_shift = PINCODE_LEN - MANUAL_CHUNK3_PINCODE_MSBITS_LEN
        pincode_mask = (1 << MANUAL_CHUNK3_PINCODE_MSBITS_LEN) - 1
        return ((self.pincode >> pincode_shift) & pincode_mask) << MANUAL_CHUNK3_PINCODE_MSBITS_POS

    def generate_manualcode(self):
        payload = str(self.manual_chunk1()).zfill(MANUAL_CHUNK1_LEN)
        payload += str(self.manual_chunk2()).zfill(MANUAL_CHUNK2_LEN)
        payload += str(self.manual_chunk3()).zfill(MANUAL_CHUNK3_LEN)

        if self.flow != CommissioningFlow.Standard:
            payload += str(self.vid).zfill(MANUAL_VID_LEN)
            payload += str(self.pid).zfill(MANUAL_PID_LEN)

        payload += calc_check_digit(payload)
        return payload

    def generate_qrcode(self):
        qrcode_bit_string = '{0:b}'.format(QRCODE_PADDING).zfill(QRCODE_PADDING_LEN)
        qrcode_bit_string += '{0:b}'.format(self.pincode).zfill(PINCODE_LEN)
        qrcode_bit_string += '{0:b}'.format(self.long_discriminator).zfill(QRCODE_DISCRIMINATOR_LEN)
        qrcode_bit_string += '{0:b}'.format(self.rendezvous).zfill(QRCODE_DISCOVERY_CAP_BITMASK_LEN)
        qrcode_bit_string += '{0:b}'.format(int(self.flow)).zfill(QRCODE_COMMISSIONING_FLOW_LEN)
        qrcode_bit_string += '{0:b}'.format(self.pid).zfill(QRCODE_PID_LEN)
        qrcode_bit_string += '{0:b}'.format(self.vid).zfill(QRCODE_VID_LEN)
        qrcode_bit_string += '{0:b}'.format(QRCODE_VERSION).zfill(QRCODE_VERSION_LEN)

        qrcode_bits = bitarray(qrcode_bit_string)
        bytes = list(qrcode_bits.tobytes())
        bytes.reverse()
        return 'MT:{}'.format(Base38.encode(bytes))


def validate_args(args):
    def check_int_range(value, min_value, max_value, name):
        if value and ((value < min_value) or (value > max_value)):
            print('{} is out of range, should be in range from {} to {}'.format(name, min_value, max_value))
            sys.exit(1)

    if args.passcode is not None:
        if ((args.passcode < 0x0000001 and args.passcode > 0x5F5E0FE) or (args.passcode in INVALID_PASSCODES)):
            print('Invalid passcode:' + str(args.passcode))
            sys.exit(1)

    check_int_range(args.discriminator, 0x0000, 0x0FFF, 'Discriminator')
    check_int_range(args.product_id, 0x0000, 0xFFFF, 'Product id')
    check_int_range(args.vendor_id, 0x0000, 0xFFFF, 'Vendor id')
    check_int_range(args.discovery_cap_bitmask, 0x0001, 0x0007, 'Discovery Capability Mask')


def main():
    def any_base_int(s): return int(s, 0)
    parser = argparse.ArgumentParser(description='Matter Manual and QRCode Setup Payload Generator Tool')
    parser.add_argument('-d', '--discriminator', type=any_base_int, required=True,
                              help='The discriminator for pairing, range: 0x00-0x0FFF')
    parser.add_argument('-p', '--passcode', type=any_base_int, required=True,
                              help='The setup passcode for pairing, range: 0x01-0x5F5E0FE')
    parser.add_argument('-vid', '--vendor-id', type=any_base_int, default=0, help='Vendor id')
    parser.add_argument('-pid', '--product-id', type=any_base_int, default=0, help='Product id')
    parser.add_argument('-cf', '--commissioning-flow', type=any_base_int, default=0,
                               help='Device commissioning flow, 0:Standard, 1:User-Intent, 2:Custom. \
                               Default is 0.', choices=[0, 1, 2])
    parser.add_argument('-dm', '--discovery-cap-bitmask', type=any_base_int, default=4,
                               help='Commissionable device discovery capability bitmask. \
                               0:SoftAP, 1:BLE, 2:OnNetwork. Default: OnNetwork')
    args = parser.parse_args()
    validate_args(args)

    payloads = SetupPayload(args.discriminator, args.passcode, args.discovery_cap_bitmask,
                            CommissioningFlow(args.commissioning_flow), args.vendor_id, args.product_id)
    manualcode = payloads.generate_manualcode()
    qrcode = payloads.generate_qrcode()

    print("Manualcode : {}".format(manualcode))
    print("QRCode     : {}".format(qrcode))


if __name__ == '__main__':
    main()

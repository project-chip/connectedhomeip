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
import enum

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

    @staticmethod
    def parse_qrcode(qrcode):
        bytes = Base38.decode(qrcode.replace('MT:', ''))
        bytes.reverse()
        qrcode_bit_string = ''.join(format(x, '08b') for x in bytes)
        i = 0
        padding = int(qrcode_bit_string[i:i+QRCODE_PADDING_LEN], 2)
        i += QRCODE_PADDING_LEN
        pincode = int(qrcode_bit_string[i:i+PINCODE_LEN], 2)
        i += PINCODE_LEN
        long_discriminator = int(qrcode_bit_string[i:i+QRCODE_DISCRIMINATOR_LEN], 2)
        i += QRCODE_DISCRIMINATOR_LEN
        rendezvous = int(qrcode_bit_string[i:i+QRCODE_DISCOVERY_CAP_BITMASK_LEN], 2)
        i += QRCODE_DISCOVERY_CAP_BITMASK_LEN
        flow = int(qrcode_bit_string[i:i+QRCODE_COMMISSIONING_FLOW_LEN], 2)
        i += QRCODE_COMMISSIONING_FLOW_LEN
        pid = int(qrcode_bit_string[i:i+QRCODE_PID_LEN], 2)
        i += QRCODE_PID_LEN
        vid = int(qrcode_bit_string[i:i+QRCODE_VID_LEN], 2)
        i += QRCODE_VID_LEN
        qrcode_version = int(qrcode_bit_string[i:i+QRCODE_VERSION_LEN], 2)
        i += QRCODE_VERSION_LEN
        obj = SetupPayload(long_discriminator, pincode, rendezvous, CommissioningFlow(flow), vid, pid)
        return obj

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

    def generate_bytes(self):
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
        return bytes

    def generate_qrcode(self):
        return 'MT:{}'.format(Base38.encode(self.generate_bytes()))

    def print(self):
        print('version    : ' + '{0:b}'.format(QRCODE_VERSION).zfill(QRCODE_VERSION_LEN))
        print('vendor id  : ' + '{0:b}'.format(self.vid).zfill(QRCODE_VID_LEN))
        print('product id : ' + '{0:b}'.format(self.pid).zfill(QRCODE_PID_LEN))
        print('comm flow  : ' + '{0:b}'.format(int(self.flow)).zfill(QRCODE_COMMISSIONING_FLOW_LEN))
        print('disc mask  : ' + '{0:b}'.format(self.rendezvous).zfill(QRCODE_DISCOVERY_CAP_BITMASK_LEN))
        print('discrimin  : ' + '{0:b}'.format(self.long_discriminator).zfill(QRCODE_DISCRIMINATOR_LEN))
        print('pin code   : ' + '{0:b}'.format(self.pincode).zfill(PINCODE_LEN))
        print('padding    : ' + '{0:b}'.format(QRCODE_PADDING).zfill(QRCODE_PADDING_LEN))
        bytes_string = ''.join(format(x, '02x') for x in self.generate_bytes())
        print('bytes      : [' + bytes_string + ']')

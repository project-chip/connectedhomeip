#!/usr/bin/env python3
#
# Copyright (c) 2022-2024 Project CHIP Authors
# All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import enum

from bitarray import bitarray
from bitarray.util import int2ba, zeros
from construct import BitsInteger, BitStruct, Enum
from stdnum.verhoeff import calc_check_digit

from .base38 import Base38

# Format for constructing manualcode
manualcode_format = BitStruct(
    'version' / BitsInteger(1),
    'vid_pid_present' / BitsInteger(1),
    'discriminator' / BitsInteger(4),
    'pincode_lsb' / BitsInteger(14),
    'pincode_msb' / BitsInteger(13),
    'vid' / BitsInteger(16),
    'pid' / BitsInteger(16),
    'padding' / BitsInteger(7),  # this is intentional as BitStruct only takes 8-bit aligned data
)

# Format for constructing qrcode
# qrcode bytes are packed as lsb....msb, hence the order is reversed
qrcode_format = BitStruct(
    'padding' / BitsInteger(4),
    'pincode' / BitsInteger(27),
    'discriminator' / BitsInteger(12),
    'discovery' / BitsInteger(8),
    'flow' / Enum(BitsInteger(2),
                  Standard=0, UserIntent=1, Custom=2),
    'pid' / BitsInteger(16),
    'vid' / BitsInteger(16),
    'version' / BitsInteger(3),
)


class CommissioningFlow(enum.IntEnum):
    Standard = 0,
    UserIntent = 1,
    Custom = 2


class SetupPayload:
    """
    Represents a Matter setup payload and provides methods for generating and parsing codes.

    Attributes:
        long_discriminator (int): The full 12-bit discriminator
        short_discriminator (int): The 4-bit short discriminator
        pincode (int): The setup PIN code
        discovery (int): Rendezvous capabilities
        flow (CommissioningFlow): The commissioning flow type
        vid (int): Vendor ID
        pid (int): Product ID

    Methods:
        p_print(): Print a formatted representation of the payload
        generate_qrcode(): Generate a QR code string
        generate_manualcode(): Generate a manual pairing code string
        parse(payload): Parse a QR code or manual pairing code string
    """

    def __init__(self, discriminator, pincode, rendezvous=4, flow=CommissioningFlow.Standard, vid=0, pid=0):
        self.long_discriminator = discriminator
        self.short_discriminator = discriminator >> 8
        self.pincode = pincode
        self.discovery = rendezvous
        self.flow = flow
        self.vid = vid
        self.pid = pid

    def p_print(self):
        """
        Print a formatted representation of the payload.
        """
        print(f'{"Flow":<24} :{self.flow}')
        print(f'{"Pincode":<24} :{self.pincode}')
        print(f'{"Short Discriminator":<24} :{self.short_discriminator}')
        print(f'{"Long Discriminator":<24} :{self.long_discriminator}')
        print(f'{"Discovery Capabilities":<24} :{self.discovery}')
        print(f'{"Vendor Id":<24} :{self.vid:<6} (0x{self.vid:04x})')
        print(f'{"Product Id":<24} :{self.pid:<6} (0x{self.pid:04x})')

    def qrcode_dict(self):
        return {
            'version': 0,
            'vid': self.vid,
            'pid': self.pid,
            'flow': int(self.flow),
            'discovery': self.discovery,
            'discriminator': self.long_discriminator,
            'pincode': self.pincode,
            'padding': 0,
        }

    def manualcode_dict(self):
        return {
            'version': 0,
            'vid_pid_present': 0 if self.flow == CommissioningFlow.Standard else 1,
            'discriminator': self.short_discriminator,
            'pincode_lsb': self.pincode & 0x3FFF,    # 14 ls-bits
            'pincode_msb': self.pincode >> 14,       # 13 ms-bits
            'vid': 0 if self.flow == CommissioningFlow.Standard else self.vid,
            'pid': 0 if self.flow == CommissioningFlow.Standard else self.pid,
            'padding': 0,
        }

    def generate_qrcode(self):
        """
        Generate a QR code string representation of the payload.

        Returns:
            str: The QR code string.
        """
        data = qrcode_format.build(self.qrcode_dict())
        b38_encoded = Base38.encode(data[::-1])  # reversing
        return 'MT:{}'.format(b38_encoded)

    def generate_manualcode(self):
        """
        Generate a manual pairing code string representation of the payload.

        Returns:
            str: The manual pairing code string.
        """
        CHUNK1_START = 0
        CHUNK1_LEN = 4
        CHUNK2_START = CHUNK1_START + CHUNK1_LEN
        CHUNK2_LEN = 16
        CHUNK3_START = CHUNK2_START + CHUNK2_LEN
        CHUNK3_LEN = 13

        bytes = manualcode_format.build(self.manualcode_dict())
        bits = bitarray()
        bits.frombytes(bytes)

        chunk1 = str(int(bits[CHUNK1_START:CHUNK1_START + CHUNK1_LEN].to01(), 2)).zfill(1)
        chunk2 = str(int(bits[CHUNK2_START:CHUNK2_START + CHUNK2_LEN].to01(), 2)).zfill(5)
        chunk3 = str(int(bits[CHUNK3_START:CHUNK3_START + CHUNK3_LEN].to01(), 2)).zfill(4)
        chunk4 = str(self.vid).zfill(5) if self.flow != CommissioningFlow.Standard else ''
        chunk5 = str(self.pid).zfill(5) if self.flow != CommissioningFlow.Standard else ''
        payload = '{}{}{}{}{}'.format(chunk1, chunk2, chunk3, chunk4, chunk5)
        return '{}{}'.format(payload, calc_check_digit(payload))

    @staticmethod
    def from_container(container, is_qrcode):
        """
        Create a SetupPayload instance from a parsed container.

        Args:
            container (dict): The parsed container with payload data.
            is_qrcode (bool): True if the container is from a QR code, False for manual code.

        Returns:
            SetupPayload: A new SetupPayload instance.
        """
        if is_qrcode:
            payload = SetupPayload(container['discriminator'], container['pincode'],
                                   container['discovery'], CommissioningFlow(container['flow'].__int__()),
                                   container['vid'], container['pid'])
        else:
            payload = SetupPayload(discriminator=container['discriminator'],
                                   pincode=(container['pincode_msb'] << 14) | container['pincode_lsb'],
                                   vid=container['vid'] if container['vid_pid_present'] else None,
                                   pid=container['pid'] if container['vid_pid_present'] else None)
            payload.short_discriminator = container['discriminator']
            payload.long_discriminator = None
            payload.discovery = None
            payload.flow = 2 if container['vid_pid_present'] else 0

        return payload

    @staticmethod
    def parse_qrcode(payload):
        """
        Parse a QR code string into a SetupPayload instance.

        Args:
            payload (str): The QR code string to parse.

        Returns:
            SetupPayload: A new SetupPayload instance.
        """
        payload = payload[3:]  # remove 'MT:'
        b38_decoded = Base38.decode(payload)[::-1]
        container = qrcode_format.parse(b38_decoded)
        return SetupPayload.from_container(container, is_qrcode=True)

    @staticmethod
    def parse_manualcode(payload):
        """
        Parse a manual pairing code string into a SetupPayload instance.

        Args:
            payload (str): The manual pairing code string to parse.

        Returns:
            SetupPayload: A new SetupPayload instance, or None if parsing fails.
        """
        payload_len = len(payload)
        if payload_len != 11 and payload_len != 21:
            print('Invalid length')
            return None

        # if first digit is greater than 7 the its not v1
        if int(str(payload)[0]) > 7:
            print('incorrect first digit')
            return None

        if calc_check_digit(payload[:-1]) != str(payload)[-1]:
            print('check digit mismatch')
            return None

        # vid_pid_present bit position
        is_long = int(str(payload)[0]) & (1 << 2)

        bits = int2ba(int(payload[0]), length=4)
        bits += int2ba(int(payload[1:6]), length=16)
        bits += int2ba(int(payload[6:10]), length=13)
        bits += int2ba(int(payload[10:15]), length=16) if is_long else zeros(16)
        bits += int2ba(int(payload[15:20]), length=16) if is_long else zeros(16)
        bits += zeros(7)  # padding

        container = manualcode_format.parse(bits.tobytes())
        return SetupPayload.from_container(container, is_qrcode=False)

    @staticmethod
    def parse(payload):
        """
        Parse either a QR code or manual pairing code string into a SetupPayload instance.

        Args:
            payload (str): The code string to parse.

        Returns:
            SetupPayload: A new SetupPayload instance.
        """
        if payload.startswith('MT:'):
            return SetupPayload.parse_qrcode(payload)
        else:
            return SetupPayload.parse_manualcode(payload)

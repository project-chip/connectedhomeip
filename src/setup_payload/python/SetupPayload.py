#!/usr/bin/env python3
#
# Copyright (c) 2024 Project CHIP Authors
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

import Base38
import click
from bitarray import bitarray
from bitarray.util import int2ba, zeros
from construct import BitsInteger, BitStruct, Enum
from stdnum.verhoeff import calc_check_digit

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
    def __init__(self, discriminator, pincode, rendezvous=4, flow=CommissioningFlow.Standard, vid=0, pid=0):
        self.long_discriminator = discriminator
        self.short_discriminator = discriminator >> 8
        self.pincode = pincode
        self.discovery = rendezvous
        self.flow = flow
        self.vid = vid
        self.pid = pid

    def p_print(self):
        print('{:<{}} :{}'.format('Flow', 24, self.flow))
        print('{:<{}} :{}'.format('Pincode', 24, self.pincode))
        print('{:<{}} :{}'.format('Short Discriminator', 24, self.short_discriminator))
        if self.long_discriminator:
            print('{:<{}} :{}'.format('Long Discriminator', 24, self.long_discriminator))
        if self.discovery:
            print('{:<{}} :{}'.format('Discovery Capabilities', 24, self.discovery))
        if self.vid is not None and self.pid is not None:
            print('{:<{}} :{:<{}} (0x{:04x})'.format('Vendor Id', 24, self.vid, 6, self.vid))
            print('{:<{}} :{:<{}} (0x{:04x})'.format('Product Id', 24, self.pid, 6, self.pid))

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
        data = qrcode_format.build(self.qrcode_dict())
        b38_encoded = Base38.encode(data[::-1])  # reversing
        return 'MT:{}'.format(b38_encoded)

    def generate_manualcode(self):
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
        payload = None
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
        payload = payload[3:]  # remove 'MT:'
        b38_decoded = Base38.decode(payload)[::-1]
        container = qrcode_format.parse(b38_decoded)
        return SetupPayload.from_container(container, is_qrcode=True)

    @staticmethod
    def parse_manualcode(payload):
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
        if payload.startswith('MT:'):
            return SetupPayload.parse_qrcode(payload)
        else:
            return SetupPayload.parse_manualcode(payload)


@click.group()
def cli():
    pass


@cli.command()
@click.argument('payload')
def parse(payload):
    click.echo(f'Parsing payload: {payload}')
    SetupPayload.parse(payload).p_print()


@cli.command()
@click.option('--discriminator', '-d', required=True, type=click.IntRange(0, 0xFFF), help='Discriminator')
@click.option('--passcode', '-p', required=True, type=click.IntRange(1, 0x5F5E0FE), help='setup pincode')
@click.option('--vendor-id', '-vid', type=click.IntRange(0, 0xFFFF), default=0, help='Vendor ID')
@click.option('--product-id', '-pid', type=click.IntRange(0, 0xFFFF), default=0, help='Product ID')
@click.option('--discovery-cap-bitmask', '-dm', type=click.IntRange(0, 7), default=4, help='Commissionable device discovery capability bitmask. 0:SoftAP, 1:BLE, 2:OnNetwork. Default: OnNetwork')
@click.option('--commissioning-flow', '-cf', type=click.IntRange(0, 2), default=0, help='Commissioning flow, 0:Standard, 1:User-Intent, 2:Custom')
def generate(passcode, discriminator, vendor_id, product_id, discovery_cap_bitmask, commissioning_flow):
    payload = SetupPayload(discriminator, passcode, discovery_cap_bitmask, commissioning_flow, vendor_id, product_id)
    print("Manualcode : {}".format(payload.generate_manualcode()))
    print("QRCode     : {}".format(payload.generate_qrcode()))


if __name__ == '__main__':
    cli()

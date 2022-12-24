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
import sys
import enum
import argparse
from bitarray import bitarray
from bitarray.util import ba2int
from stdnum.verhoeff import calc_check_digit
import Base38

# See section 5.1.4.1 Manual Pairing Code in the Matter specification v1.0
kManualDiscriminatorLength = 4
kPincodeLength = 27

kManualChunk1DiscriminatorMsbitsLength = 2
kManualChunk1DiscriminatorMsbitsPos = 0
kManualChunk1VidPidPresentBitPos = kManualChunk1DiscriminatorMsbitsPos + kManualChunk1DiscriminatorMsbitsLength
kManualChunk1Length = 1

kManualChunk2DiscriminatorLsbitsLength = 2
kManualChunk2PincodeLsbitsLength = 14
kManualChunk2PincodeLsbitsPos = 0
kManualChunk2DiscriminatorLsbitsPos = kManualChunk2PincodeLsbitsPos + kManualChunk2PincodeLsbitsLength
kManualChunk2Length = 5

kManualChunk3PincodeMsbitsLength = 13
kManualChunk3PincodeMsbitsPos = 0
kManualChunk3Length = 4

kManualVendorIdLength = 5
kManualProductIdLength = 5

# See section 5.1.3. QR Code in the Matter specification v1.0
kQRCodeVersionLength = 3
kQRCodeDiscriminatorLength = 12
kQRCodeVendorIdLength = 16
kQRCodeProductIdLength = 16
kQRCodeCommissioningFlowLength = 2
kQRCodeDiscoveryCapBitmaskLength = 8
kQRCodePaddingLength = 4
kQRCodeVersion = 0
kQRCodePadding = 0

INVALID_PASSCODES = [00000000, 11111111, 22222222, 33333333, 44444444, 55555555,
                     66666666, 77777777, 88888888, 99999999, 12345678, 87654321]


class CommissioningFlow(enum.IntEnum):
    Standard = 0,
    UserIntent = 1,
    Custom = 2


class SetupPayload:
    def __init__(self, discriminator, pincode, rendezvous=4, flow=CommissioningFlow.Standard, vid=0, pid=0):
        self.longDiscriminator = discriminator
        self.shortDiscriminator = self.GetShortDiscriminator(discriminator)
        self.pincode = pincode
        self.rendezvous = rendezvous
        self.flow = flow
        self.vid = vid
        self.pid = pid

    # 4 Most-significant bits of the 12-bits Discriminator
    def GetShortDiscriminator(self, discriminator):
        return (discriminator >> 8)

    def ManualChunk1(self):
        discriminatorShift = (kManualDiscriminatorLength - kManualChunk1DiscriminatorMsbitsLength)
        discriminatorMask = (1 << kManualChunk1DiscriminatorMsbitsLength) - 1
        discriminatorChunk = (self.shortDiscriminator >> discriminatorShift) & discriminatorMask
        vidPidPresentFlag = 0 if self.flow == CommissioningFlow.Standard else 1
        return (discriminatorChunk << kManualChunk1DiscriminatorMsbitsPos) | (vidPidPresentFlag << kManualChunk1VidPidPresentBitPos)

    def ManualChunk2(self):
        discriminatorMask = (1 << kManualChunk2DiscriminatorLsbitsLength) - 1
        pincodeMask = (1 << kManualChunk2PincodeLsbitsLength) - 1
        discriminatorChunk = self.shortDiscriminator & discriminatorMask
        return ((self.pincode & pincodeMask) << kManualChunk2PincodeLsbitsPos) | (discriminatorChunk << kManualChunk2DiscriminatorLsbitsPos)

    def ManualChunk3(self):
        pincodeShift = kPincodeLength - kManualChunk3PincodeMsbitsLength
        pincodeMask = (1 << kManualChunk3PincodeMsbitsLength) - 1
        return ((self.pincode >> pincodeShift) & pincodeMask) << kManualChunk3PincodeMsbitsPos

    def GenerateManualCode(self):
        payload = str(self.ManualChunk1()).zfill(kManualChunk1Length)
        payload += str(self.ManualChunk2()).zfill(kManualChunk2Length)
        payload += str(self.ManualChunk3()).zfill(kManualChunk3Length)

        if self.flow != CommissioningFlow.Standard:
            payload += str(self.vid).zfill(kManualVendorIdLength)
            payload += str(self.pid).zfill(kManualProductIdLength)

        payload += calc_check_digit(payload)
        return payload

    def GenerateQRCode(self):
        qrcodeBitString = '{0:b}'.format(kQRCodePadding).zfill(kQRCodePaddingLength)
        qrcodeBitString += '{0:b}'.format(self.pincode).zfill(kPincodeLength)
        qrcodeBitString += '{0:b}'.format(self.longDiscriminator).zfill(kQRCodeDiscriminatorLength)
        qrcodeBitString += '{0:b}'.format(self.rendezvous).zfill(kQRCodeDiscoveryCapBitmaskLength)
        qrcodeBitString += '{0:b}'.format(int(self.flow)).zfill(kQRCodeCommissioningFlowLength)
        qrcodeBitString += '{0:b}'.format(self.pid).zfill(kQRCodeProductIdLength)
        qrcodeBitString += '{0:b}'.format(self.vid).zfill(kQRCodeVendorIdLength)
        qrcodeBitString += '{0:b}'.format(kQRCodeVersion).zfill(kQRCodeVersionLength)

        qrcodeBits = bitarray(qrcodeBitString)
        bytes = list(qrcodeBits.tobytes())
        bytes.reverse()
        return 'MT:{}'.format(Base38.Encode(bytes))


def ValidateArgs(args):
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
    ValidateArgs(args)

    payloads = SetupPayload(args.discriminator, args.passcode, args.discovery_cap_bitmask,
                            CommissioningFlow(args.commissioning_flow), args.vendor_id, args.product_id)
    manualcode = payloads.GenerateManualCode()
    qrcode = payloads.GenerateQRCode()

    print("Manualcode : {}".format(manualcode))
    print("QRCode     : {}".format(qrcode))


if __name__ == '__main__':
    main()

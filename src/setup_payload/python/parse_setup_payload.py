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
import sys

from SetupPayload import SetupPayload


def validate_args(args):
    if args.qrcode is not None:
        if len(args.qrcode.replace('MT:', '')) != 19:
            print('Invalid qrcode:' + str(args.qrcode))
            sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description='Matter Manual and QRCode Setup Payload Generator Tool')
    parser.add_argument('-q', '--qrcode',
                              help='The qrcode string for parsing')
    # parser.add_argument('-m', '--manualcode',
    # help='The manual code string for parsing')
    args = parser.parse_args()
    validate_args(args)

    payload = SetupPayload.parse_qrcode(args.qrcode)
    payload.print()


if __name__ == '__main__':
    main()

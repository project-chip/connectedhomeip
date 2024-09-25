#!/usr/bin/env python3
#
#    Copyright (c) 2024 Project CHIP Authors
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

import click

from .setup_payload import SetupPayload


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

    manualcode = payload.generate_manualcode()
    qrcode = payload.generate_qrcode()
    qrcode_link = 'https://project-chip.github.io/connectedhomeip/qrcode.html?data={}'.format(qrcode)

    print("Manualcode : {}".format(manualcode))
    print("QRCode     : {}".format(qrcode))
    print("QRCode Link: {}".format(qrcode_link))


if __name__ == '__main__':
    cli()

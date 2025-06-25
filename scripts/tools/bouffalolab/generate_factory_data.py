#!/usr/bin/env python3
#
#    Copyright (c) 2022 Project CHIP Authors
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
import binascii
import logging as log
import os
import random
import secrets
import shlex
import subprocess
import sys
from datetime import datetime, timedelta
from pathlib import Path

from Crypto.Cipher import AES
from cryptography import x509
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.serialization import load_der_private_key
from cryptography.x509.oid import ObjectIdentifier

MATTER_ROOT = os.path.dirname(os.path.realpath(__file__))[:-len("/scripts/tools/bouffalolab")]

TEST_CD_CERT = MATTER_ROOT + "/credentials/test/certification-declaration/Chip-Test-CD-Signing-Cert.pem"
TEST_CD_KEY = MATTER_ROOT + "/credentials/test/certification-declaration/Chip-Test-CD-Signing-Key.pem"
TEST_PAA_CERT = MATTER_ROOT + "/credentials/test/attestation/Chip-Test-PAA-FFF1-Cert.pem"
TEST_PAA_KEY = MATTER_ROOT + "/credentials/test/attestation/Chip-Test-PAA-FFF1-Key.pem"
TEST_PAI_CERT = MATTER_ROOT + "/credentials/test/attestation/Chip-Test-PAI-FFF1-8000-Cert.pem"
TEST_PAI_KEY = MATTER_ROOT + "/credentials/test/attestation/Chip-Test-PAI-FFF1-8000-Key.pem"
TEST_CHIP_CERT = MATTER_ROOT + "/out/linux-x64-chip-cert/chip-cert"
TEST_CD_TYPE = 1  # 0 - development, 1 - provisional, 2 - official


def gen_test_passcode(passcode):

    INVALID_PASSCODES = [0, 11111111, 22222222, 33333333, 44444444,
                         55555555, 66666666, 77777777, 88888888, 99999999,
                         12345678, 87654321]

    def check_passcode(passcode):
        return 0 <= passcode <= 99999999 and passcode not in INVALID_PASSCODES

    if isinstance(passcode, int):
        if check_passcode(passcode):
            raise Exception("passcode is invalid value.")
        return passcode

    passcode = -1
    while not check_passcode(passcode):
        passcode = random.randint(0, 99999999)

    return passcode


def gen_test_discriminator(discriminator):

    if isinstance(discriminator, int):
        if discriminator > 0xfff:
            raise Exception("discriminator is invalid value.")

    discriminator = random.randint(0, 0xfff)

    return discriminator


def gen_test_unique_id(unique_id):

    if isinstance(unique_id, bytes):
        if len(unique_id) != 16:
            raise Exception("rotating unique id has invalid length.")
        return unique_id

    unique_id = secrets.token_bytes(16)

    return unique_id


def gen_test_spake2(passcode, spake2p_it, spake2p_salt, spake2p_verifier=None):

    sys.path.insert(0, os.path.join(MATTER_ROOT, 'scripts', 'tools', 'spake2p'))
    from spake2p import generate_verifier

    if isinstance(spake2p_it, int):
        if not 1000 <= spake2p_it <= 100000:
            raise Exception("SPAKE2+ iteration count out of range.")
    else:
        spake2p_it = random.randint(1000, 100000)

    if spake2p_salt is None:
        spake2p_salt_len = random.randint(16, 32)
        spake2p_salt = bytes(random.sample(range(0, 255), spake2p_salt_len))
    else:
        spake2p_salt = base64.b64decode(spake2p_salt)
        if not 16 <= len(spake2p_salt) <= 32:
            raise Exception("SPAKE2+ slat is invalid.")

    if spake2p_verifier is None:
        spake2p_verifier = generate_verifier(passcode, spake2p_salt, spake2p_it)
    else:
        if generate_verifier(passcode, spake2p_salt, spake2p_it) != spake2p_verifier:
            raise Exception("SPAKE2+ verifier is invalid.")

    return spake2p_it, spake2p_salt, spake2p_verifier


def gen_test_certs(chip_cert: str,
                   output: str,
                   vendor_id: int,
                   product_id: int,
                   device_name: str,
                   cd_cert: str = None,
                   cd_key: str = None,
                   cd: str = None,
                   paa_cert: str = None,
                   paa_key: str = None,
                   pai_cert: str = None,
                   pai_key: str = None,
                   dac_cert: str = None,
                   dac_key: str = None,
                   dac_product_id: int = None,
                   discriminator: int = None):

    def parse_cert_file(cert):

        def get_subject_attr(subject, oid):
            try:
                return subject.get_attributes_for_oid(oid)
            except Exception:
                return None

        if not os.path.isfile(cert):
            return None, None, None, None, None

        with open(cert, 'rb') as cert_file:
            cert = x509.load_pem_x509_certificate(cert_file.read(), default_backend())

        vendor_id = None
        product_id = None
        for attribute in cert.subject:
            if ObjectIdentifier('1.3.6.1.4.1.37244.2.1') == attribute.oid:
                vendor_id = int(attribute.value, 16)
            if ObjectIdentifier('1.3.6.1.4.1.37244.2.2') == attribute.oid:
                product_id = int(attribute.value, 16)

        issue_date = cert.not_valid_before
        expire_date = cert.not_valid_after

        return vendor_id, product_id, issue_date, expire_date

    def verify_certificates(chip_cert, paa_cert, pai_cert, dac_cert):

        if not os.path.isfile(pai_cert) or not os.path.isfile(dac_cert):
            raise Exception("PAI certificate or DAC certificate is not specified.")

        with open(pai_cert, 'rb') as cert_file:
            cert = x509.load_pem_x509_certificate(cert_file.read(), default_backend())
        pai_public_key = cert.public_key()

        with open(dac_cert, 'rb') as cert_file:
            cert = x509.load_pem_x509_certificate(cert_file.read(), default_backend())

        try:
            pai_public_key.verify(cert.signature, cert.tbs_certificate_bytes, ec.ECDSA(hashes.SHA256()))
        except Exception:
            raise Exception("Failed to verify DAC signature with PAI certificate.")

        if (pai_cert != TEST_PAI_CERT and paa_cert != TEST_PAA_CERT) or (pai_cert == TEST_PAI_CERT and paa_cert == TEST_PAA_CERT):
            if os.path.isfile(paa_cert):
                cmd = [chip_cert, "validate-att-cert",
                       "--dac", dac_cert,
                       "--pai", pai_cert,
                       "--paa", paa_cert,
                       ]
                log.info("Verify Certificate Chain: {}".format(shlex.join(cmd)))
                subprocess.run(cmd)

    def gen_dac_certificate(chip_cert, device_name, dac_vid, dac_pid, pai_cert, pai_key, dac_cert, dac_key, pai_issue_date, pai_expire_date, discriminator):
        def gen_valid_times(issue_date, expire_date):
            now = datetime.now() - timedelta(days=1)

            if not (issue_date <= now <= expire_date):
                raise Exception("Invalid time in test PAI to generate DAC.")

            return now.strftime('%Y%m%d%H%M%SZ'), (expire_date - now).days

        if not os.path.isfile(dac_cert) or not os.path.isfile(dac_key):
            if not os.path.isfile(pai_cert) or not os.path.isfile(pai_key):
                raise Exception("No test PAI certificate/key specified for test DAC generation.")

            valid_from, lifetime = gen_valid_times(pai_issue_date, pai_expire_date)
            cmd = [chip_cert, "gen-att-cert",
                   "--type", "d",  # device attestation certificate
                   "--subject-cn", device_name + " Test DAC 0",
                   "--subject-vid", hex(dac_vid),
                   "--subject-pid", hex(dac_pid),
                   "--ca-cert", pai_cert,
                   "--ca-key", pai_key,
                   "--out", dac_cert,
                   "--out-key", dac_key,
                   "--valid-from", valid_from,
                   "--lifetime", str(lifetime),
                   ]
            log.info("Generate DAC: {}".format(shlex.join(cmd)))
            subprocess.run(cmd)

    def convert_pem_to_der(chip_cert, action, pem):

        if not os.path.isfile(pem):
            raise Exception("File {} is not existed.".format(pem))

        der = Path(pem).with_suffix(".der")
        if not os.path.isfile(der):
            cmd = [chip_cert, action, pem, der, "--x509-der", ]
            subprocess.run(cmd)

        return der

    def gen_cd(chip_cert, paa_cert, dac_vendor_id, dac_product_id, vendor_id, product_id, cd_cert, cd_key, cd):

        if os.path.isfile(cd):
            return

        cmd = [chip_cert, "gen-cd",
               "--key", cd_key,
               "--cert", cd_cert,
               "--out", cd,
               "--format-version",  "1",
               "--vendor-id",  hex(vendor_id),
               "--product-id",  hex(product_id),
               "--device-type-id", "0x1234",
               "--certificate-id", "ZIG20141ZB330001-24",
               "--security-level",  "0",
               "--security-info",  "0",
               "--certification-type",  str(TEST_CD_TYPE),
               "--version-number", "9876",
               ]

        if dac_vendor_id != vendor_id or dac_product_id != product_id:
            cmd += ["--dac-origin-vendor-id", hex(dac_vendor_id),
                    "--dac-origin-product-id", hex(dac_product_id),
                    ]

        if paa_cert:
            cmd += ["--authorized-paa-cert", paa_cert]

        log.info("Generate CD: {}".format(shlex.join(cmd)))
        subprocess.run(cmd)

    pai_vendor_id, pai_product_id, pai_issue_date, pai_expire_date = parse_cert_file(pai_cert)

    dac_vendor_id = pai_vendor_id

    if dac_product_id is not None and pai_product_id is not None and dac_product_id != pai_product_id:
        raise Exception("Specified product id for DAC certificate is not same as product id in PAI certificate.")

    if pai_product_id is not None:
        dac_product_id = pai_product_id

    if dac_cert is None:
        dac_disc_vp = "{}_{}_{}".format(hex(dac_vendor_id).split("x")[-1], hex(dac_product_id).split("x")[-1], discriminator)
        dac_cert = os.path.join(output, "out_{}_dac_cert.pem".format(dac_disc_vp))
        dac_key = os.path.join(output, "out_{}_dac_key.pem".format(dac_disc_vp))

    gen_dac_certificate(chip_cert, device_name, dac_vendor_id, dac_product_id, pai_cert,
                        pai_key, dac_cert, dac_key, pai_issue_date, pai_expire_date, discriminator)

    dac_cert_der = convert_pem_to_der(chip_cert, "convert-cert", dac_cert)
    dac_key_der = convert_pem_to_der(chip_cert, "convert-key", dac_key)
    pai_cert_der = convert_pem_to_der(chip_cert, "convert-cert", pai_cert)

    dac_vendor_id, dac_product_id, *_ = parse_cert_file(dac_cert)
    paa_vendor_id, paa_product_id, *_ = parse_cert_file(paa_cert)

    verify_certificates(chip_cert, paa_cert, pai_cert, dac_cert)

    gen_cd(chip_cert, paa_cert, dac_vendor_id, dac_product_id, vendor_id, product_id, cd_cert, cd_key, cd)

    return cd, pai_cert_der, dac_cert_der, dac_key_der


def gen_mfd_partition(args, mfd_output):

    def int_to_2bytearray_l(intvalue):
        src = bytearray(2)
        src[1] = (intvalue >> 8) & 0xFF
        src[0] = (intvalue >> 0) & 0xFF
        return src

    def int_to_4bytearray_l(intvalue):
        src = bytearray(4)
        src[3] = (intvalue >> 24) & 0xFF
        src[2] = (intvalue >> 16) & 0xFF
        src[1] = (intvalue >> 8) & 0xFF
        src[0] = (intvalue >> 0) & 0xFF
        return src

    def gen_efuse_aes_iv():
        return bytes(random.sample(range(0, 0xff), 12) + [0] * 4)

    def read_file(rfile):
        with open(rfile, 'rb') as _f:
            return _f.read()

    def get_private_key(der):
        with open(der, 'rb') as file:
            keys = load_der_private_key(file.read(), password=None, backend=default_backend())
            private_key = keys.private_numbers().private_value.to_bytes(32, byteorder='big')

            return private_key

    def encrypt_data(data_bytearray, key_bytearray, iv_bytearray):
        data_bytearray += bytes([0] * (16 - (len(data_bytearray) % 16)))
        cryptor = AES.new(key_bytearray, AES.MODE_CBC, iv_bytearray)
        ciphertext = cryptor.encrypt(data_bytearray)
        return ciphertext

    def convert_to_bytes(data):
        if isinstance(data, bytes) or isinstance(data, str):
            if isinstance(data, str):
                return data.encode()
            else:
                return data
        elif isinstance(data, int):
            byte_len = int((data.bit_length() + 7) / 8)
            return data.to_bytes(byte_len, byteorder='little')
        elif data is None:
            return bytes([])
        else:
            raise Exception("Data is invalid type: {}".format(type(data)))

    def gen_tlvs(mfdDict, need_sec):
        MFD_ID_RAW_MASK = 0x8000

        sec_tlvs = bytes([])
        raw_tlvs = bytes([])

        for name in mfdDict.keys():
            d = mfdDict[name]
            if d["sec"] and need_sec:
                if d["len"] and d["len"] < len(d["data"]):
                    raise Exception("Data size of {} is over {}".format(name, d["len"]))
                sec_tlvs += int_to_2bytearray_l(d["id"])
                sec_tlvs += int_to_2bytearray_l(len(d["data"]))
                sec_tlvs += d["data"]

        for name in mfdDict.keys():
            d = mfdDict[name]
            if not d["sec"] or not need_sec:
                if d["len"] and d["len"] < len(d["data"]):
                    raise Exception("Data size of {} is over {}".format(name, d["len"]))
                raw_tlvs += int_to_2bytearray_l(d["id"] + MFD_ID_RAW_MASK)
                raw_tlvs += int_to_2bytearray_l(len(d["data"]))
                raw_tlvs += d["data"]

        return sec_tlvs, raw_tlvs

    mfdDict = {
        "aes_iv": {'sec': False, "id": 1, "len": 16, "data": gen_efuse_aes_iv() if args.key else bytes([0] * 16)},
        "dac_cert": {'sec': False, "id": 2, "len": None, "data": read_file(args.dac_cert)},
        "dac_key": {'sec': True, "id": 3, "len": None, "data": get_private_key(args.dac_key)},
        "passcode": {'sec': False, "id": 4, "len": 4, "data": convert_to_bytes(args.passcode)},
        "pai_cert": {'sec': False, "id": 5, "len": None, "data": read_file(args.pai_cert)},
        "cd": {'sec': False, "id": 6, "len": None, "data": read_file(args.cd)},
        "sn": {'sec': False, "id": 7, "len": 32, "data": convert_to_bytes(args.sn)},
        "discriminator": {'sec': False, "id": 8, "len": 2, "data": convert_to_bytes(args.discriminator)},
        "uid": {'sec': False, "id": 9, "len": 32, "data": convert_to_bytes(args.unique_id)},
        "spake2p_it": {'sec': False, "id": 10, "len": 4, "data": convert_to_bytes(args.spake2p_it)},
        "spake2p_salt": {'sec': False, "id": 11, "len": 32, "data": convert_to_bytes(args.spake2p_salt)},
        "spake2p_verifier": {'sec': False, "id": 12, "len": None, "data": convert_to_bytes(args.spake2p_verifier)},
        "vendor_name": {'sec': False, "id": 13, "len": 32, "data": convert_to_bytes(args.vendor_name)},
        "vendor_id": {'sec': False, "id": 14, "len": 2, "data": convert_to_bytes(args.vendor_id)},
        "product_name": {'sec': False, "id": 15, "len": 32, "data": convert_to_bytes(args.product_name)},
        "product_id": {'sec': False, "id": 16, "len": 2, "data": convert_to_bytes(args.product_id)},
        "product_part_no": {'sec': False, "id": 17, "len": 32, "data": convert_to_bytes(args.product_part_no)},
        "product_url": {'sec': False, "id": 18, "len": 256, "data": convert_to_bytes(args.product_url)},
        "product_label": {'sec': False, "id": 19, "len": 64, "data": convert_to_bytes(args.product_label)},
        "manufactoring_date": {'sec': False, "id": 20, "len": 16, "data": convert_to_bytes(args.manufactoring_date)},
        "hardware_ver": {'sec': False, "id": 21, "len": 4, "data": convert_to_bytes(args.hardware_version)},
        "hardware_ver_str": {'sec': False, "id": 22, "len": 64, "data": convert_to_bytes(args.hardware_version_string)},
    }

    sec_tlvs, raw_tlvs = gen_tlvs(mfdDict, args.key)

    output = bytes([])

    sec_tlvs = sec_tlvs and encrypt_data(sec_tlvs, args.key, mfdDict["aes_iv"]["data"])

    output = int_to_4bytearray_l(len(sec_tlvs))
    output += sec_tlvs
    output += int_to_4bytearray_l(binascii.crc32(sec_tlvs))
    output += int_to_4bytearray_l(len(raw_tlvs))
    output += raw_tlvs
    output += int_to_4bytearray_l(binascii.crc32(raw_tlvs))

    with open(mfd_output, "wb+") as fp:
        fp.write(output)


def gen_onboarding_data(args, onboard_txt, onboard_png, rendez=6):

    try:
        import qrcode
        from SetupPayload import CommissioningFlow, SetupPayload
    except ImportError:
        sys.path.append(os.path.join(MATTER_ROOT, "src/setup_payload/python"))
        try:
            import qrcode
            from SetupPayload import CommissioningFlow, SetupPayload
        except Exception:
            raise Exception("Please make sure qrcode has been installed.")
    else:
        raise Exception("Please make sure qrcode has been installed.")

    setup_payload = SetupPayload(discriminator=args.discriminator,
                                 pincode=args.passcode,
                                 rendezvous=rendez,
                                 flow=CommissioningFlow.Standard,
                                 vid=args.vendor_id,
                                 pid=args.product_id)
    with open(onboard_txt, "w") as manual_code_file:
        manual_code_file.write("Manualcode : " + setup_payload.generate_manualcode() + "\n")
        manual_code_file.write("QRCode : " + setup_payload.generate_qrcode())
    qr = qrcode.make(setup_payload.generate_qrcode())
    qr.save(onboard_png)

    return setup_payload.generate_manualcode(), setup_payload.generate_qrcode()


def main():

    def check_arg(args):

        if not isinstance(args.output, str) or not os.path.exists(args.output):
            raise Exception("output path is not specified or not existed.")
        log.info("output path: {}".format(args.output))

        if not isinstance(args.chip_cert, str) or not os.path.exists(args.chip_cert):
            raise Exception("chip-cert should be built before and is specified.")
        log.info("chip-cert path: {}".format(args.chip_cert))

    def to_bytes(input):
        if isinstance(input, str):
            return bytearray.fromhex(input)
        elif isinstance(input, bytes):
            return input
        else:
            return None

    def hex_to_int(hex_string):
        return int(hex_string, 16)

    parser = argparse.ArgumentParser(description="Bouffalo Lab Factory Data generator tool")

    parser.add_argument("--dac_cert", type=str, help="DAC certificate file.")
    parser.add_argument("--dac_key", type=str, help="DAC certificate privat key.")
    parser.add_argument("--dac_pid", type=hex_to_int, help="Product Identification, hex string, used in DAC certificate. ")
    parser.add_argument("--passcode", type=int, help="Setup pincode, optional.")
    parser.add_argument("--pai_cert", type=str, default=TEST_PAI_CERT, help="PAI certificate file.")
    parser.add_argument("--cd", type=str, help="Certificate Declaration file.")
    parser.add_argument("--sn", type=str, default="Test SN", help="Serial Number, optional.")
    parser.add_argument("--discriminator", type=int, help="Discriminator ID, optional.")
    parser.add_argument("--unique_id", type=base64.b64decode, help="Rotating Unique ID in hex string, optional.")
    parser.add_argument("--spake2p_it", type=int, default=None, help="Spake2+ iteration count, optional.")
    parser.add_argument("--spake2p_salt", type=base64.b64decode, help="Spake2+ salt in hex string, optional.")

    parser.add_argument("--vendor_id", type=hex_to_int, default=0x130D, help="Vendor Identification, hex string, mandatory.")
    parser.add_argument("--vendor_name", type=str, default="Bouffalo Lab", help="Vendor Name string, optional.")
    parser.add_argument("--product_id", type=hex_to_int, default=0x1001, help="Product Identification, hex string, mandatory.")
    parser.add_argument("--product_name", type=str, default="Test Product", help="Product Name string, optional.")

    parser.add_argument("--product_part_no", type=str, help="Product Part number, optional.")
    parser.add_argument("--product_url", type=str, help="Product Web URL, optional.")
    parser.add_argument("--product_label", type=str, help="Product Web URL, optional.")
    parser.add_argument("--manufactoring_date", type=str, help="Product Web URL, optional.")
    parser.add_argument("--hardware_version", type=int, help="Product Web URL, optional.")
    parser.add_argument("--hardware_version_string", type=str, help="Product Web URL, optional.")
    parser.add_argument("--rendezvous", type=int, default=6, help="Rendezvous Mode for QR code generation")

    parser.add_argument("--output", type=str, help="output path.")
    parser.add_argument("--key", type=str, help="Encrypt private part in mfd.")

    parser.add_argument("--cd_cert", type=str, default=TEST_CD_CERT, help="for test, to sign certificate declaration.")
    parser.add_argument("--cd_key", type=str, default=TEST_CD_KEY, help="for test, to sign certificate declaration.")
    parser.add_argument("--paa_cert", type=str, default=TEST_PAA_CERT, help="for test, to verify certificate chain.")
    parser.add_argument("--paa_key", type=str, default=TEST_PAA_KEY, help="for test, to sign pai certificate.")
    parser.add_argument("--pai_key", type=str, default=TEST_PAI_KEY, help="for test, to sign dac certificate.")
    parser.add_argument("--chip_cert", type=str, default=TEST_CHIP_CERT, help="for test, the tool to issue dac certificate.")

    args = parser.parse_args()

    log.basicConfig(format='[%(levelname)s] %(message)s', level=log.INFO)

    check_arg(args)

    passcode = gen_test_passcode(args.passcode)
    discriminator = gen_test_discriminator(args.discriminator)
    unique_id = gen_test_unique_id(args.unique_id)
    spake2p_it, spake2p_salt, spake2p_verifier = gen_test_spake2(passcode, args.spake2p_it, args.spake2p_salt)

    vp_info = "{}_{}".format(hex(args.vendor_id).split('x')[-1], hex(args.product_id).split('x')[-1])
    vp_disc_info = "{}_{}".format(vp_info, discriminator)

    if args.cd is None:
        args.cd = os.path.join(args.output, "out_{}_cd.der".format(vp_info))

    cd, pai_cert_der, dac_cert_der, dac_key_der = gen_test_certs(args.chip_cert,
                                                                 args.output,
                                                                 args.vendor_id,
                                                                 args.product_id,
                                                                 args.vendor_name,
                                                                 args.cd_cert,
                                                                 args.cd_key,
                                                                 args.cd,
                                                                 args.paa_cert,
                                                                 args.paa_key,
                                                                 args.pai_cert,
                                                                 args.pai_key,
                                                                 args.dac_cert,
                                                                 args.dac_key,
                                                                 args.dac_pid,
                                                                 discriminator)

    mfd_output = os.path.join(args.output, "out_{}_mfd.bin".format(vp_disc_info))
    args.dac_cert = dac_cert_der
    args.dac_key = dac_key_der
    args.passcode = passcode
    args.pai_cert = pai_cert_der
    args.cd = cd
    args.discriminator = discriminator
    args.unique_id = unique_id
    args.spake2p_it = spake2p_it
    args.spake2p_salt = spake2p_salt
    args.spake2p_verifier = spake2p_verifier
    args.key = to_bytes(args.key)
    gen_mfd_partition(args, mfd_output)

    onboard_txt = os.path.join(args.output, "out_{}_onboard.txt".format(vp_disc_info))
    onboard_png = os.path.join(args.output, "out_{}_onboard.png".format(vp_disc_info))
    manualcode, qrcode = gen_onboarding_data(args, onboard_txt, onboard_png, args.rendezvous)

    log.info("")
    log.info("Output as below: ")
    log.info("Passcode: {}".format(passcode))
    log.info("Discriminator ID: {}".format(discriminator))
    log.info("Rotating Unique ID: {}".format(unique_id.hex()))
    log.info("Rotating Unique ID base64 code: {}".format(base64.b64encode(unique_id).decode()))
    log.info("SPAKE2+ iteration: {}".format(spake2p_it))
    log.info("SPAKE2+ slat: {}".format(spake2p_salt.hex()))
    log.info("SPAKE2+ slat base code: {}".format(base64.b64encode(spake2p_salt).decode()))
    log.info("Manual code: {}".format(manualcode))
    log.info("QR code: {}".format(qrcode))

    log.info("")
    log.info("MFD partition file: {}".format(mfd_output))
    log.info("QR code PNG file: {}".format(onboard_png))


if __name__ == "__main__":
    main()

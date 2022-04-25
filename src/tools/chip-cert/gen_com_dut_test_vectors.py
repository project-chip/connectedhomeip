#!/usr/bin/env python

import argparse
import json
import os
import subprocess
import sys
import typing
import cryptography.x509
import os.path
import glob
from binascii import hexlify, unhexlify
from enum import Enum


class CertType(Enum):
    PAA = 1
    PAI = 2
    DAC = 3


CERT_STRUCT_TEST_CASES = [
    {
        "description": 'Valid certificate version field set to v3(2)',
        "test_folder": 'cert_version_v3',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'Invalid certificate version field set to v2(1)',
        "test_folder": 'cert_version_v2',
        "error_flag": 'cert-version',
        "is_success_case": 'false',
    },
    {
        "description": 'Valid certificate signature algorithm ECDSA_WITH_SHA256',
        "test_folder": 'sig_algo_ecdsa_with_sha256',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'Invalid certificate signature algorithm ECDSA_WITH_SHA1',
        "test_folder": 'sig_algo_ecdsa_with_sha1',
        "error_flag": 'sig-algo',
        "is_success_case": 'false',
    },
    {
        "description": "VID in Subject field doesn't match VID in Issuer field",
        "test_folder": 'subject_vid_mismatch',
        "error_flag": 'subject-vid-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": "PID in Subject field doesn't match PID in Issuer field",
        "test_folder": 'subject_pid_mismatch',
        "error_flag": 'subject-pid-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": "Valid certificate public key curve prime256v1",
        "test_folder": 'sig_curve_prime256v1',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": "Invalid certificate public key curve secp256k1",
        "test_folder": 'sig_curve_secp256k1',
        "error_flag": 'sig-curve',
        "is_success_case": 'false',
    },
    # TODO Cases:
    # 'issuer-vid'
    # 'issuer-pid'
    # 'subject-vid'
    # 'subject-pid'
    {
        "description": "Certificate doesn't include Basic Constraint extension",
        "test_folder": 'ext_basic_missing',
        "error_flag": 'ext-basic-missing',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Basic Constraint extension critical field is missing",
        "test_folder": 'ext_basic_critical_missing',
        "error_flag": 'ext-basic-critical-missing',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Basic Constraint extension critical field is set as 'non-critical'",
        "test_folder": 'ext_basic_critical_wrong',
        "error_flag": 'ext-basic-critical-wrong',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Basic Constraint extension CA field is missing",
        "test_folder": 'ext_basic_ca_missing',
        "error_flag": 'ext-basic-ca-missing',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Basic Constraint extension CA field is wrong (TRUE for DAC and FALSE for PAI)",
        "test_folder": 'ext_basic_ca_wrong',
        "error_flag": 'ext-basic-ca-wrong',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Basic Constraint extension PathLen field presence is wrong (present for DAC not present for PAI)",
        "test_folder": 'ext_basic_pathlen_presence_wrong',
        "error_flag": 'ext-basic-pathlen-presence-wrong',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Basic Constraint extension PathLen field set to 0",
        "test_folder": 'ext_basic_pathlen0',
        "error_flag": 'ext-basic-pathlen0',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Basic Constraint extension PathLen field set to 1",
        "test_folder": 'ext_basic_pathlen1',
        "error_flag": 'ext-basic-pathlen1',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Basic Constraint extension PathLen field set to 2",
        "test_folder": 'ext_basic_pathlen2',
        "error_flag": 'ext-basic-pathlen2',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate doesn't include Key Usage extension",
        "test_folder": 'ext_key_usage_missing',
        "error_flag": 'ext-key-usage-missing',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Key Usage extension critical field is missing",
        "test_folder": 'ext_key_usage_critical_missing',
        "error_flag": 'ext-key-usage-critical-missing',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Key Usage extension critical field is set as 'non-critical'",
        "test_folder": 'ext_key_usage_critical_wrong',
        "error_flag": 'ext-key-usage-critical-wrong',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Key Usage extension diginalSignature field is wrong (not present for DAC and present for PAI",
        "test_folder": 'ext_key_usage_dig_sig_wrong',
        "error_flag": 'ext-key-usage-dig-sig',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Key Usage extension keyCertSign field is wrong (present for DAC and not present for PAI",
        "test_folder": 'ext_key_usage_key_cert_sign_wrong',
        "error_flag": 'ext-key-usage-key-cert-sign',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Key Usage extension cRLSign field is wrong (present for DAC and not present for PAI",
        "test_folder": 'ext_key_usage_crl_sign_wrong',
        "error_flag": 'ext-key-usage-crl-sign',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate doesn't include Authority Key ID (AKID) extension",
        "test_folder": 'ext_akid_missing',
        "error_flag": 'ext-akid-missing',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate doesn't include Subject Key ID (SKID) extension",
        "test_folder": 'ext_skid_missing',
        "error_flag": 'ext-skid-missing',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate includes optional Extended Key Usage extension",
        "test_folder": 'ext_extended_key_usage_present',
        "error_flag": 'ext-extended-key-usage',
        "is_success_case": 'true',
    },
    {
        "description": "Certificate includes optional Authority Information Access extension",
        "test_folder": 'ext_authority_info_access_present',
        "error_flag": 'ext-authority-info-access',
        "is_success_case": 'true',
    },
    {
        "description": "Certificate includes optional Subject Alternative Name extension",
        "test_folder": 'ext_subject_alt_name_present',
        "error_flag": 'ext-subject-alt-name',
        "is_success_case": 'true',
    },
]


class Names:
    def __init__(self, cert_type: CertType, paa_path, test_case_out_dir):
        prefixes = {CertType.PAA: paa_path,
                    CertType.PAI: test_case_out_dir + '/pai-',
                    CertType.DAC: test_case_out_dir + '/dac-'}
        prefix = prefixes[cert_type]

        self.cert_pem = prefix + 'Cert.pem'
        self.cert_der = prefix + 'Cert.der'
        self.key_pem = prefix + 'Key.pem'
        self.key_der = prefix + 'Key.der'


class DevCertBuilder:
    def __init__(self, cert_type: CertType, error_type: str, paa_path: str, test_case_out_dir: str, chip_cert: str, vid: int, pid: int):
        self.vid = vid
        self.pid = pid
        self.cert_type = cert_type
        self.error_type = error_type
        self.chipcert = chip_cert

        if not os.path.exists(self.chipcert):
            raise Exception('Path not found: %s' % self.chipcert)

        if not os.path.exists(test_case_out_dir):
            os.mkdir(test_case_out_dir)

        paa = Names(CertType.PAA, paa_path, test_case_out_dir)
        pai = Names(CertType.PAI, paa_path, test_case_out_dir)
        dac = Names(CertType.DAC, paa_path, test_case_out_dir)
        if cert_type == CertType.PAI:
            self.signer = paa
            self.own = pai
        if cert_type == CertType.DAC:
            self.signer = pai
            self.own = dac

    def make_certs_and_keys(self) -> None:
        """Creates the PEM and DER certs and keyfiles"""
        error_type_flag = ' -i -e' + self.error_type

        if self.cert_type == CertType.PAI:
            subject_name = 'Matter Test PAI'
            vid_flag = ' -V 0x{:X}'.format(self.vid)
            pid_flag = ''
            type_flag = '-t i'
        elif self.cert_type == CertType.DAC:
            subject_name = 'Matter Test DAC'
            vid_flag = ' -V 0x{:X}'.format(self.vid)
            pid_flag = ' -P 0x{:X}'.format(self.pid)
            type_flag = '-t d'
        else:
            return

        cmd = self.chipcert + ' gen-att-cert ' + type_flag + error_type_flag + ' -c "' + subject_name + '" -C ' + self.signer.cert_pem + ' -K ' + \
            self.signer.key_pem + vid_flag + pid_flag + ' -l 4294967295 -o ' + self.own.cert_pem + ' -O ' + self.own.key_pem
        subprocess.run(cmd, shell=True)
        cmd = 'openssl x509 -inform pem -in ' + self.own.cert_pem + \
            ' -out ' + self.own.cert_der + ' -outform DER'
        subprocess.run(cmd, shell=True)
        cmd = 'openssl ec -inform pem -in ' + self.own.key_pem + \
            ' -out ' + self.own.key_der + ' -outform DER'
        subprocess.run(cmd, shell=True)


def add_raw_ec_keypair_to_dict_from_der(der_key_filename: str, json_dict: dict):
    with open(der_key_filename, 'rb') as infile:
        key_data_der = infile.read()

    key_der = cryptography.hazmat.primitives.serialization.load_der_private_key(key_data_der, None)
    json_dict["dac_private_key"] = hexlify(key_der.private_numbers().private_value.to_bytes(32, byteorder='big')).decode('utf-8')

    pk_x = key_der.public_key().public_numbers().x
    pk_y = key_der.public_key().public_numbers().y

    public_key_raw_bytes = bytearray([0x04])
    public_key_raw_bytes.extend(bytearray(pk_x.to_bytes(32, byteorder='big')))
    public_key_raw_bytes.extend(bytearray(pk_y.to_bytes(32, byteorder='big')))

    json_dict["dac_public_key"] = hexlify(bytes(public_key_raw_bytes)).decode('utf-8')


def add_files_to_json_config(files_mapping: dict, json_dict: dict):
    for output_key_name, filename in files_mapping.items():
        with open(filename, "rb") as infile:
            file_bytes = infile.read()
            json_dict[output_key_name] = hexlify(file_bytes).decode('utf-8')


def generate_test_case_vector_json(test_case_out_dir: str, test_cert: str, test_case):
    json_dict = {}
    files_in_path = glob.glob(os.path.join(test_case_out_dir, "*"))
    output_json_filename = test_case_out_dir + "/test_case_vector.json"

    files_to_add = {
        "dac_cert": "dac-Cert.der",
        "pai_cert": "pai-Cert.der",
        "firmware_information": "firmware-info.bin",
        "certification_declaration": "cd.der",
    }

    # Add description fields to JSON Config
    if "description" in test_case:
        json_dict["description"] = test_cert.upper() + " Test Vector: " + test_case["description"]
    if "is_success_case" in test_case:
        json_dict["is_success_case"] = test_case["is_success_case"]

    # Out of all files we could add, find the ones that were present in test case, and embed them in hex
    files_available = {os.path.basename(path) for path in files_in_path}
    files_to_add = {key: os.path.join(test_case_out_dir, filename)
                    for key, filename in files_to_add.items() if filename in files_available}

    add_files_to_json_config(files_to_add, json_dict)

    # Embed the DAC key if present
    if "dac-Key.der" in files_available:
        der_key_filename = os.path.join(test_case_out_dir, "dac-Key.der")
        add_raw_ec_keypair_to_dict_from_der(der_key_filename, json_dict)

    with open(output_json_filename, "wt+") as outfile:
        json.dump(json_dict, outfile, indent=2)


def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument('-o', '--out_dir', dest='outdir',
                           default='credentials/development/commissioner_dut',
                           help='output directory for all generated test vectors')
    argparser.add_argument('-p', '--paa', dest='paapath',
                           default='credentials/test/attestation/Chip-Test-PAA-FFF1-', help='PAA to use')
    argparser.add_argument('-d', '--cd', dest='cdpath',
                           default='credentials/test/certification-declaration/Chip-Test-CD-Signing-',
                           help='CD Signing Key/Cert to use')
    argparser.add_argument('-c', '--chip-cert_dir', dest='chipcertdir',
                           default='out/debug/linux_x64_clang/', help='Directory where chip-cert tool is located')

    args = argparser.parse_args()

    if not os.path.exists(args.outdir):
        os.mkdir(args.outdir)

    chipcert = args.chipcertdir + 'chip-cert'

    if not os.path.exists(chipcert):
        raise Exception('Path not found: %s' % chipcert)

    cd_cert = args.cdpath + 'Cert.pem'
    cd_key = args.cdpath + 'Key.pem'

    for test_cert in ['dac', 'pai']:
        for test_case in CERT_STRUCT_TEST_CASES:
            test_case_out_dir = args.outdir + '/struct_' + test_cert + '_' + test_case["test_folder"]
            if test_cert == 'dac':
                error_type_dac = test_case["error_flag"]
                error_type_pai = 'no-error'
            else:
                if test_case["error_flag"] == 'ext-skid-missing':
                    error_type_dac = 'ext-akid-missing'
                else:
                    error_type_dac = 'no-error'
                error_type_pai = test_case["error_flag"]

            vid = 0xFFF1
            pid = 0x8000

            # Generate PAI Cert/Key
            builder = DevCertBuilder(CertType.PAI, error_type_pai, args.paapath, test_case_out_dir,
                                     chipcert, vid, pid)
            builder.make_certs_and_keys()

            if test_cert == 'pai':
                if test_case["error_flag"] == 'subject-vid-mismatch':
                    vid += 1
                if test_case["error_flag"] == 'subject-pid-mismatch':
                    pid += 1

            # Generate DAC Cert/Key
            builder = DevCertBuilder(CertType.DAC, error_type_dac, args.paapath, test_case_out_dir,
                                     chipcert, vid, pid)
            builder.make_certs_and_keys()

            # Generate Certification Declaration (CD)
            vid_flag = ' -V 0x{:X}'.format(vid)
            pid_flag = ' -p 0x{:X}'.format(pid)
            cmd = chipcert + ' gen-cd -K ' + cd_key + ' -C ' + cd_cert + ' -O ' + test_case_out_dir + '/cd.der' + \
                ' -f 1 ' + vid_flag + pid_flag + ' -d 0x1234 -c "ZIG20141ZB330001-24" -l 0 -i 0 -n 9876 -t 0'
            subprocess.run(cmd, shell=True)

            # Generate Test Case Data Container in JSON Format
            generate_test_case_vector_json(test_case_out_dir, test_cert, test_case)


if __name__ == '__main__':
    sys.exit(main())

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

VID_NOT_PRESENT = 0xFFFF
PID_NOT_PRESENT = 0x0000

VALID_IN_PAST = "2020-06-28 14:23:43"
VALID_NOW = "2022-09-28 14:23:43"
VALID_IN_FUTURE = "2031-06-28 14:23:43"


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
    {
        "description": "Certificate validity period starts in the past",
        "test_folder": 'valid_in_past',
        "error_flag": 'no-error',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate validity period starts in the future",
        "test_folder": 'valid_in_future',
        "error_flag": 'no-error',
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
        "description": "Certificate Key Usage extension diginalSignature field is wrong (not present for DAC and present for PAI, which is OK as optional)",
        "test_folder": 'ext_key_usage_dig_sig_wrong',
        "error_flag": 'ext-key-usage-dig-sig',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Key Usage extension keyCertSign field is wrong (present for DAC and not present for PAI)",
        "test_folder": 'ext_key_usage_key_cert_sign_wrong',
        "error_flag": 'ext-key-usage-key-cert-sign',
        "is_success_case": 'false',
    },
    {
        "description": "Certificate Key Usage extension cRLSign field is wrong (present for DAC and not present for PAI)",
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

VIDPID_FALLBACK_ENCODING_TEST_CASES = [
    # Valid/Invalid encoding examples from the spec:
    {
        "description": 'Fallback VID and PID encoding example from spec: valid and recommended since easily human-readable',
        "common_name": 'ACME Matter Devel DAC 5CDA9899 Mvid:FFF1 Mpid:00B1',
        "test_folder": 'vidpid_fallback_encoding_01',
        "is_success_case": 'true',
    },
    {
        "description": 'Fallback VID and PID encoding example from spec: valid and recommended since easily human-readable',
        "common_name": 'ACME Matter Devel DAC 5CDA9899 Mpid:00B1 Mvid:FFF1',
        "test_folder": 'vidpid_fallback_encoding_02',
        "is_success_case": 'true',
    },
    {
        "description": 'Fallback VID and PID encoding example from spec: valid example showing that order or separators are not considered at all for the overall validity of the embedded fields',
        "common_name": 'Mpid:00B1,ACME Matter Devel DAC 5CDA9899,Mvid:FFF1',
        "test_folder": 'vidpid_fallback_encoding_03',
        "is_success_case": 'true',
    },
    {
        "description": 'Fallback VID and PID encoding example from spec: valid, but less readable',
        "common_name": 'ACME Matter Devel DAC 5CDA9899 Mvid:FFF1Mpid:00B1',
        "test_folder": 'vidpid_fallback_encoding_04',
        "is_success_case": 'true',
    },
    {
        "description": 'Fallback VID and PID encoding example from spec: valid, but highly discouraged, since embedding of substrings within other substrings may be confusing to human readers',
        "common_name": 'Mvid:FFF1ACME Matter Devel DAC 5CDAMpid:00B19899',
        "test_folder": 'vidpid_fallback_encoding_05',
        "is_success_case": 'true',
    },
    {
        "description": 'Fallback VID and PID encoding example from spec: invalid, since substring following Mvid: is not exactly 4 uppercase hexadecimal digits',
        "common_name": 'ACME Matter Devel DAC 5CDA9899 Mvid:FF1 Mpid:00B1',
        "test_folder": 'vidpid_fallback_encoding_06',
        "is_success_case": 'false',
    },
    {
        "description": 'Fallback VID and PID encoding example from spec: invalid, since substring following Mvid: is not exactly 4 uppercase hexadecimal digits',
        "common_name": 'ACME Matter Devel DAC 5CDA9899 Mvid:fff1 Mpid:00B1',
        "test_folder": 'vidpid_fallback_encoding_07',
        "is_success_case": 'false',
    },
    {
        "description": 'Fallback VID and PID encoding example from spec: invalid, since substring following Mpid: is not exactly 4 uppercase hexadecimal digits',
        "common_name": 'ACME Matter Devel DAC 5CDA9899 Mvid:FFF1 Mpid:B1',
        "test_folder": 'vidpid_fallback_encoding_08',
        "is_success_case": 'false',
    },
    {
        "description": 'Fallback VID and PID encoding example from spec: invalid, since substring following Mpid: is not exactly 4 uppercase hexadecimal digits',
        "common_name": 'ACME Matter Devel DAC 5CDA9899 Mpid: Mvid:FFF1',
        "test_folder": 'vidpid_fallback_encoding_09',
        "is_success_case": 'false',
    },
    # More valid/invalid fallback encoding examples:
    {
        "description": 'Fallback VID and PID encoding example: invalid VID encoding',
        "common_name": 'Mvid:FFF Mpid:00B10x',
        "test_folder": 'vidpid_fallback_encoding_10',
        "is_success_case": 'false',
    },
    {
        "description": 'Fallback VID and PID encoding example: valid, but less human-readable',
        "common_name": 'MpidMvid:FFF10 Matter Test Mpid:00B1',
        "test_folder": 'vidpid_fallback_encoding_11',
        "is_success_case": 'true',
    },
    {
        "description": 'Fallback VID and PID encoding example: invalid, PID not present and VID not upper case',
        "common_name": 'Matter Devel DAC Mpid:Mvid:Fff1',
        "test_folder": 'vidpid_fallback_encoding_12',
        "is_success_case": 'false',
    },
    {
        "description": 'Fallback VID and PID encoding example: invalid VID prefix',
        "common_name": 'Matter Devel DAC Mpid:00B1 MVID:FFF1',
        "test_folder": 'vidpid_fallback_encoding_13',
        "is_success_case": 'false',
    },
    {
        "description": 'Fallback VID and PID encoding example: invalid PID and VID prefixes',
        "common_name": 'Matter Devel DAC Mpid_00B1 Mvid_FFF1',
        "test_folder": 'vidpid_fallback_encoding_14',
        "is_success_case": 'false',
    },
    # Examples with both fallback encoding in the common name and using Matter specific OIDs
    {
        "description": 'Mix of Fallback and Matter OID encoding for VID and PID: valid, Matter OIDs are used and wrong values in the common-name are ignored',
        "common_name": 'ACME Matter Devel DAC 5CDA9899 Mvid:FFF2 Mpid:00B2',
        "vid": 0xFFF1,
        "pid": 0x00B1,
        "test_folder": 'vidpid_fallback_encoding_15',
        "is_success_case": 'true',
    },
    {
        "description": 'Mix of Fallback and Matter OID encoding for VID and PID: wrong, Correct values encoded in the common-name are ignored',
        "common_name": 'ACME Matter Devel DAC 5CDA9899 Mvid:FFF1 Mpid:00B1',
        "vid": 0xFFF2,
        "pid": 0x00B2,
        "test_folder": 'vidpid_fallback_encoding_16',
        "is_success_case": 'false',
    },
    {
        "description": 'Mix of Fallback and Matter OID encoding for VID and PID: invalid, PID is using Matter OID then VID must also use Matter OID',
        "common_name": 'Mvid:FFF1',
        "pid": 0x00B1,
        "test_folder": 'vidpid_fallback_encoding_17',
        "is_success_case": 'false',
    },
]

CD_STRUCT_TEST_CASES = [
    {
        "description": 'Valid format_version field set to 1.',
        "test_folder": 'format_version_1',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'The format_version field is missing.',
        "test_folder": 'format_version_missing',
        "error_flag": 'format-version-missing',
        "is_success_case": 'false',
    },
    {
        "description": 'Invalid format_version field set to 2.',
        "test_folder": 'format_version_2',
        "error_flag": 'format-version-wrong',
        "is_success_case": 'false',
    },
    {
        "description": 'The vendor_id field is missing.',
        "test_folder": 'vid_missing',
        "error_flag": 'vid-missing',
        "is_success_case": 'false',
    },
    {
        "description": "The vendor_id field doesn't match the VID in DAC.",
        "test_folder": 'vid_mismatch',
        "error_flag": 'vid-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": 'The product_id_array field is missing.',
        "test_folder": 'pid_array_missing',
        "error_flag": 'pid-array-missing',
        "is_success_case": 'false',
    },
    {
        "description": "The product_id_array field is empty TLV array.",
        "test_folder": 'pid_array_count0',
        "error_flag": 'pid-array-count0',
        "is_success_case": 'false',
    },
    {
        "description": "The product_id_array field has one PID value which matches the PID value in DAC.",
        "test_folder": 'pid_array_count01_valid',
        "error_flag": 'pid-array-count01-valid',
        "is_success_case": 'true',
    },
    {
        "description": "The product_id_array field has one PID value that doesn't match the PID value in DAC.",
        "test_folder": 'pid_array_count01_mismatch',
        "error_flag": 'pid-array-count01-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": "The product_id_array field has 10 PID values one of which matches the PID value in DAC.",
        "test_folder": 'pid_array_count10_valid',
        "error_flag": 'pid-array-count10-valid',
        "is_success_case": 'true',
    },
    {
        "description": "The product_id_array field has 10 PID values none of which matches the PID value in DAC.",
        "test_folder": 'pid_array_count10_mismatch',
        "error_flag": 'pid-array-count10-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": "The product_id_array field has 100 PID values one of which matches the PID value in DAC.",
        "test_folder": 'pid_array_count100_valid',
        "error_flag": 'pid-array-count100-valid',
        "is_success_case": 'true',
    },
    {
        "description": "The product_id_array field has 100 PID values none of which matches the PID value in DAC.",
        "test_folder": 'pid_array_count100_mismatch',
        "error_flag": 'pid-array-count100-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": "The device_type_id field is missing.",
        "test_folder": 'device_type_id_missing',
        "error_flag": 'device-type-id-missing',
        "is_success_case": 'false',
    },
    {
        "description": "The device_type_id field doesn't match the device_type_id value in the DCL entries associated with the VID and PID.",
        "test_folder": 'device_type_id_mismatch',
        "error_flag": 'device-type-id-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": "The certificate_id field is missing.",
        "test_folder": 'cert_id_missing',
        "error_flag": 'cert-id-missing',
        "is_success_case": 'false',
    },
    {
        "description": "The certificate_id field doesn't contain a globally unique serial number allocated by the CSA for this CD.",
        "test_folder": 'cert_id_mismatch',
        "error_flag": 'cert-id-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": 'The certificate_id field has wrong length.',
        "test_folder": 'cert_id_len_wrong',
        "error_flag": 'cert-id-len-wrong',
        "is_success_case": 'false',
    },
    {
        "description": 'The security_level field is missing.',
        "test_folder": 'security_level_missing',
        "error_flag": 'security-level-missing',
        "is_success_case": 'false',
    },
    {
        "description": 'The security_level field is set to invalid value (different from 0).',
        "test_folder": 'security_level_wrong',
        "error_flag": 'security-level-wrong',
        "is_success_case": 'false',
    },
    {
        "description": 'The security_information field is missing.',
        "test_folder": 'security_info_missing',
        "error_flag": 'security-info-missing',
        "is_success_case": 'false',
    },
    {
        "description": 'The security_information field is set to invalid value (different from 0).',
        "test_folder": 'security_info_wrong',
        "error_flag": 'security-info-wrong',
        "is_success_case": 'false',
    },
    {
        "description": 'The version_number field is missing.',
        "test_folder": 'version_number_missing',
        "error_flag": 'version-number-missing',
        "is_success_case": 'false',
    },
    {
        "description": 'The version_number field matches the VID and PID used in a DeviceSoftwareVersionModel entry in the DCL matching the certification record associated with the product presenting this CD.',
        "test_folder": 'version_number_match',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": "The version_number field doesn't match the VID and PID used in a DeviceSoftwareVersionModel entry in the DCL matching the certification record associated with the product presenting this CD.",
        "test_folder": 'version_number_wrong',
        "error_flag": 'version-number-wrong',
        "is_success_case": 'false',
    },
    {
        "description": 'The certification_type field is missing.',
        "test_folder": 'cert_type_missing',
        "error_flag": 'cert-type-missing',
        "is_success_case": 'false',
    },
    {
        "description": 'The certification_type field is set to invalid value.',
        "test_folder": 'cert_type_wrong',
        "error_flag": 'cert-type-wrong',
        "is_success_case": 'false',
    },
    {
        "description": 'The dac_origin_vendor_id and dac_origin_product_id fields are not present.',
        "test_folder": 'dac_origin_vid_pid_missing',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'The dac_origin_vendor_id field is present and dac_origin_product_id fields is not present.',
        "test_folder": 'dac_origin_vid_present_pid_missing',
        "error_flag": 'dac-origin-vid-present',
        "is_success_case": 'false',
    },
    {
        "description": 'The dac_origin_vendor_id field is not present and dac_origin_product_id is present.',
        "test_folder": 'dac_origin_vid_missing_pid_present',
        "error_flag": 'dac-origin-pid-present',
        "is_success_case": 'false',
    },
    {
        "description": 'The dac_origin_vendor_id and dac_origin_product_id fields present and contain the VID and PID values that match the VID and PID found in the DAC Subject DN.',
        "test_folder": 'dac_origin_vid_pid_present_match',
        "error_flag": 'dac-origin-vid-pid-present',
        "is_success_case": 'true',
    },
    {
        "description": "The dac_origin_vendor_id and dac_origin_product_id fields present and the VID value doesn't match the VID found in the DAC Subject DN.",
        "test_folder": 'dac_origin_vid_pid_present_vid_mismatch',
        "error_flag": 'dac-origin-vid-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": "The dac_origin_vendor_id and dac_origin_product_id fields present and the PID value doesn't match the PID found in the DAC Subject DN.",
        "test_folder": 'dac_origin_vid_pid_present_pid_mismatch',
        "error_flag": 'dac-origin-pid-mismatch',
        "is_success_case": 'false',
    },
    {
        "description": 'The optional authorized_paa_list field is not present.',
        "test_folder": 'authorized_paa_list_missing',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'The authorized_paa_list contains one valid PAA which is authorized to sign the PAI.',
        "test_folder": 'authorized_paa_list_count0',
        "error_flag": 'authorized-paa-list-count0',
        "is_success_case": 'false',
    },
    {
        "description": 'The authorized_paa_list contains one valid PAA which is authorized to sign the PAI.',
        "test_folder": 'authorized_paa_list_count1_valid',
        "error_flag": 'authorized-paa-list-count1-valid',
        "is_success_case": 'true',
    },
    {
        "description": 'The authorized_paa_list contains two PAAs one of which is valid PAA authorized to sign the PAI.',
        "test_folder": 'authorized_paa_list_count2_valid',
        "error_flag": 'authorized-paa-list-count2-valid',
        "is_success_case": 'true',
    },
    {
        "description": 'The authorized_paa_list contains three PAAs none of which is a valid PAA authorized to sign the PAI.',
        "test_folder": 'authorized_paa_list_count3_invalid',
        "error_flag": 'authorized-paa-list-count3-invalid',
        "is_success_case": 'false',
    },
    {
        "description": 'The authorized_paa_list contains ten PAAs one of which is valid PAA authorized to sign the PAI.',
        "test_folder": 'authorized_paa_list_count10_valid',
        "error_flag": 'authorized-paa-list-count10-valid',
        "is_success_case": 'true',
    },
    {
        "description": 'The authorized_paa_list contains ten PAAs none of which is a valid PAA authorized to sign the PAI.',
        "test_folder": 'authorized_paa_list_count10_invalid',
        "error_flag": 'authorized-paa-list-count10-invalid',
        "is_success_case": 'false',
    },
    {
        "description": 'Invalid Signer Info version set to v2.',
        "test_folder": 'signer_info_v2',
        "error_flag": 'signer-info-v2',
        "is_success_case": 'false',
    },
    {
        "description": 'Invalid Signer Info digest algorithm SHA1.',
        "test_folder": 'signer_info_digest_algo_sha1',
        "error_flag": 'signer-info-digest-algo',
        "is_success_case": 'false',
    },
    {
        "description": 'The subjectKeyIdentifier contains SKID of a well-known Zigbee Alliance certificate.',
        "test_folder": 'signer_info_skid_valid',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'The subjectKeyIdentifier contains invalid SKID of a certificate unknown by Zigbee Alliance.',
        "test_folder": 'signer_info_skid_invalid',
        "error_flag": 'signer-info-skid',
        "is_success_case": 'false',
    },
    {
        "description": 'Valid CMS version set to v3.',
        "test_folder": 'cms_v3',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'Invalid CMS version set to v2.',
        "test_folder": 'cms_v2',
        "error_flag": 'cms-v2',
        "is_success_case": 'false',
    },
    {
        "description": 'Valid CMS digest algorithm SHA256.',
        "test_folder": 'cms_digest_algo_sha256',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'Invalid CMS digest algorithm SHA1.',
        "test_folder": 'cms_digest_algo_sha1',
        "error_flag": 'cms-digest-algo',
        "is_success_case": 'false',
    },
    {
        "description": 'Valid CMS signature algorithm ECDSA_WITH_SHA256.',
        "test_folder": 'cms_sig_algo_ecdsa_with_sha256',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'Invalid CMS signature algorithm ECDSA_WITH_SHA1.',
        "test_folder": 'cms_sig_algo_ecdsa_with_sha1',
        "error_flag": 'cms-sig-algo',
        "is_success_case": 'false',
    },
    {
        "description": 'Valid CMS eContentType pkcs7-data.',
        "test_folder": 'cms_econtent_type_pkcs7_data',
        "error_flag": 'no-error',
        "is_success_case": 'true',
    },
    {
        "description": 'Invalid CMS eContentType is set to Microsoft Authenticode [MSAC] OID = { 1.3.6.1.4.1.311.2.1.4 }.',
        "test_folder": 'cms_econtent_type_msac',
        "error_flag": 'cms-econtent-type',
        "is_success_case": 'false',
    },
    {
        "description": 'Invalid CMS Signature.',
        "test_folder": 'cms_signature',
        "error_flag": 'cms-sig',
        "is_success_case": 'false',
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
    def __init__(self, cert_type: CertType, error_type: str, paa_path: str, test_case_out_dir: str, chip_cert: str, vid: int, pid: int, custom_cn_attribute: str, valid_from: str):
        self.vid = vid
        self.pid = pid
        self.cert_type = cert_type
        self.error_type = error_type
        self.chipcert = chip_cert
        self.custom_cn_attribute = custom_cn_attribute
        self.valid_from = valid_from

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
        error_type_flag = ' -I -E' + self.error_type
        subject_name = self.custom_cn_attribute
        vid_flag = ' -V 0x{:X}'.format(self.vid)
        pid_flag = ' -P 0x{:X}'.format(self.pid)
        if (len(self.valid_from) == 0):
            validity_flags = ' -l 4294967295 '
        else:
            validity_flags = ' -f "' + self.valid_from + '" -l 730 '

        if self.cert_type == CertType.PAI:
            if (len(subject_name) == 0):
                subject_name = 'Matter Test PAI'
            type_flag = '-t i'
        elif self.cert_type == CertType.DAC:
            if (len(subject_name) == 0):
                subject_name = 'Matter Test DAC'
            type_flag = '-t d'
        else:
            return

        cmd = self.chipcert + ' gen-att-cert ' + type_flag + error_type_flag + ' -c "' + subject_name + '" -C ' + self.signer.cert_pem + ' -K ' + \
            self.signer.key_pem + vid_flag + pid_flag + validity_flags + ' -o ' + self.own.cert_pem + ' -O ' + self.own.key_pem
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
        # These test cases are expected to fail when error injected in DAC but expected to pass when error injected in PAI
        if (test_cert == 'pai') and (test_case["test_folder"] in ['ext_basic_pathlen0', 'vidpid_fallback_encoding_08', 'vidpid_fallback_encoding_09', 'ext_key_usage_dig_sig_wrong']):
            json_dict["is_success_case"] = "true"
        else:
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

            if test_case["test_folder"] == 'valid_in_past':
                if test_cert == 'dac':
                    dac_valid_from = VALID_IN_PAST
                    pai_valid_from = VALID_NOW
                else:
                    dac_valid_from = VALID_NOW
                    pai_valid_from = VALID_IN_PAST
            elif test_case["test_folder"] == 'valid_in_future':
                if test_cert == 'dac':
                    dac_valid_from = VALID_IN_FUTURE
                    pai_valid_from = VALID_NOW
                else:
                    dac_valid_from = VALID_NOW
                    pai_valid_from = VALID_IN_FUTURE
            else:
                dac_valid_from = ''
                pai_valid_from = ''

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
                                     chipcert, vid, PID_NOT_PRESENT, '', pai_valid_from)
            builder.make_certs_and_keys()

            if test_cert == 'pai':
                if test_case["error_flag"] == 'subject-vid-mismatch':
                    vid += 1
                if test_case["error_flag"] == 'subject-pid-mismatch':
                    pid += 1

            # Generate DAC Cert/Key
            builder = DevCertBuilder(CertType.DAC, error_type_dac, args.paapath, test_case_out_dir,
                                     chipcert, vid, pid, '', dac_valid_from)
            builder.make_certs_and_keys()

            # Generate Certification Declaration (CD)
            vid_flag = ' -V 0x{:X}'.format(vid)
            pid_flag = ' -p 0x{:X}'.format(pid)
            cmd = chipcert + ' gen-cd -K ' + cd_key + ' -C ' + cd_cert + ' -O ' + test_case_out_dir + '/cd.der' + \
                ' -f 1 ' + vid_flag + pid_flag + ' -d 0x1234 -c "ZIG20141ZB330001-24" -l 0 -i 0 -n 9876 -t 0'
            subprocess.run(cmd, shell=True)

            # Generate Test Case Data Container in JSON Format
            generate_test_case_vector_json(test_case_out_dir, test_cert, test_case)

    for test_cert in ['dac', 'pai']:
        for test_case in VIDPID_FALLBACK_ENCODING_TEST_CASES:
            test_case_out_dir = args.outdir + '/struct_' + test_cert + '_' + test_case["test_folder"]
            if test_cert == 'dac':
                common_name_dac = test_case["common_name"]
                common_name_pai = ''
                if "vid" in test_case:
                    vid_dac = test_case["vid"]
                else:
                    vid_dac = VID_NOT_PRESENT
                if "pid" in test_case:
                    pid_dac = test_case["pid"]
                else:
                    pid_dac = PID_NOT_PRESENT
                vid_pai = 0xFFF1
                pid_pai = 0x00B1
            else:
                common_name_dac = ''
                common_name_pai = test_case["common_name"]
                common_name_pai = common_name_pai.replace('DAC', 'PAI')
                vid_dac = 0xFFF1
                pid_dac = 0x00B1
                if "vid" in test_case:
                    vid_pai = test_case["vid"]
                else:
                    vid_pai = VID_NOT_PRESENT
                if "pid" in test_case:
                    pid_pai = test_case["pid"]
                else:
                    pid_pai = PID_NOT_PRESENT

            # Generate PAI Cert/Key
            builder = DevCertBuilder(CertType.PAI, 'no-error', args.paapath, test_case_out_dir,
                                     chipcert, vid_pai, pid_pai, common_name_pai, '')
            builder.make_certs_and_keys()

            # Generate DAC Cert/Key
            builder = DevCertBuilder(CertType.DAC, 'no-error', args.paapath, test_case_out_dir,
                                     chipcert, vid_dac, pid_dac, common_name_dac, '')
            builder.make_certs_and_keys()

            # Generate Certification Declaration (CD)
            cmd = chipcert + ' gen-cd -K ' + cd_key + ' -C ' + cd_cert + ' -O ' + test_case_out_dir + '/cd.der' + \
                ' -f 1  -V 0xFFF1  -p 0x00B1 -d 0x1234 -c "ZIG20141ZB330001-24" -l 0 -i 0 -n 9876 -t 0'
            subprocess.run(cmd, shell=True)

            # Generate Test Case Data Container in JSON Format
            generate_test_case_vector_json(test_case_out_dir, test_cert, test_case)

    for test_case in CD_STRUCT_TEST_CASES:
        test_case_out_dir = args.outdir + '/struct_cd_' + test_case["test_folder"]
        vid = 0xFFF1
        pid = 0x8000

        # Generate PAI Cert/Key
        builder = DevCertBuilder(CertType.PAI, 'no-error', args.paapath, test_case_out_dir,
                                 chipcert, vid, pid, '', '')
        builder.make_certs_and_keys()

        # Generate DAC Cert/Key
        builder = DevCertBuilder(CertType.DAC, 'no-error', args.paapath, test_case_out_dir,
                                 chipcert, vid, pid, '', '')
        builder.make_certs_and_keys()

        # Generate Certification Declaration (CD)
        vid_flag = ' -V 0x{:X}'.format(vid)
        pid_flag = ' -p 0x{:X}'.format(pid)

        dac_origin_flag = ' '
        if test_case["error_flag"] == 'dac-origin-vid-present' or test_case["error_flag"] == 'dac-origin-vid-pid-present':
            dac_origin_flag += ' -o 0x{:X}'.format(vid)
        if test_case["error_flag"] == 'dac-origin-pid-present' or test_case["error_flag"] == 'dac-origin-vid-pid-present':
            dac_origin_flag += ' -r 0x{:X}'.format(pid)

        if test_case["error_flag"] == 'authorized-paa-list-count0' or test_case["error_flag"] == 'authorized-paa-list-count1-valid' or test_case["error_flag"] == 'authorized-paa-list-count2-valid' or test_case["error_flag"] == 'authorized-paa-list-count3-invalid' or test_case["error_flag"] == 'authorized-paa-list-count10-valid' or test_case["error_flag"] == 'authorized-paa-list-count10-invalid':
            authorized_paa_flag = ' -a ' + args.paapath + 'Cert.pem'
        else:
            authorized_paa_flag = ''

        cmd = chipcert + ' gen-cd -I -E ' + test_case["error_flag"] + ' -K ' + cd_key + ' -C ' + cd_cert + ' -O ' + test_case_out_dir + '/cd.der' + \
            ' -f 1 ' + vid_flag + pid_flag + dac_origin_flag + authorized_paa_flag + ' -d 0x1234 -c "ZIG20141ZB330001-24" -l 0 -i 0 -n 9876 -t 0'
        subprocess.run(cmd, shell=True)

        # Generate Test Case Data Container in JSON Format
        generate_test_case_vector_json(test_case_out_dir, 'cd', test_case)

    # Test case: Generate {DAC, PAI, PAA} chain with random (invalid) PAA
    test_case = {
        "description": 'Use Invalid PAA (Not Registered in the DCL).',
        "test_folder": 'invalid_paa',
        "error_flag": 'no-error',
        "is_success_case": 'false',
    }
    test_case_out_dir = args.outdir + '/' + test_case["test_folder"]
    paapath = test_case_out_dir + '/paa-'

    if not os.path.exists(test_case_out_dir):
        os.mkdir(test_case_out_dir)

    # Generate PAA Cert/Key
    cmd = chipcert + ' gen-att-cert -t a -c "Invalid (Not Registered in the DCL) Matter PAA" -f "' + VALID_IN_PAST + \
        '" -l 4294967295 -o ' + paapath + 'Cert.pem -O ' + paapath + 'Key.pem'
    subprocess.run(cmd, shell=True)

    vid = 0xFFF1
    pid = 0x8000

    # Generate PAI Cert/Key
    builder = DevCertBuilder(CertType.PAI, test_case["error_flag"], paapath, test_case_out_dir,
                             chipcert, vid, PID_NOT_PRESENT, '', VALID_IN_PAST)
    builder.make_certs_and_keys()

    # Generate DAC Cert/Key
    builder = DevCertBuilder(CertType.DAC, test_case["error_flag"], paapath, test_case_out_dir,
                             chipcert, vid, pid, '', VALID_IN_PAST)
    builder.make_certs_and_keys()

    # Generate Certification Declaration (CD)
    vid_flag = ' -V 0x{:X}'.format(vid)
    pid_flag = ' -p 0x{:X}'.format(pid)
    cmd = chipcert + ' gen-cd -K ' + cd_key + ' -C ' + cd_cert + ' -O ' + test_case_out_dir + '/cd.der' + \
        ' -f 1 ' + vid_flag + pid_flag + ' -d 0x1234 -c "ZIG20141ZB330001-24" -l 0 -i 0 -n 9876 -t 0'
    subprocess.run(cmd, shell=True)

    # Generate Test Case Data Container in JSON Format
    generate_test_case_vector_json(test_case_out_dir, 'paa', test_case)


if __name__ == '__main__':
    sys.exit(main())

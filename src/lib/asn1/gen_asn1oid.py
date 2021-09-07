#!/usr/bin/env python

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2019 Google LLC.
#    Copyright (c) 2013-2017 Nest Labs, Inc.
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

#
#    @file
#      This file implements a Python script to generate a C/C++ header
#      for individual ASN1 Object IDs (OIDs) that are used in CHIP
#      TLV encodings (notably the CHIP Certificate object).
#

from __future__ import absolute_import
from __future__ import print_function
import optparse
import sys


def identity(n):
    return n


# OID labels
ansi_X9_62 = identity
certicom = identity
characteristicTwo = identity
chip = identity
curve = identity
curves = identity
digest_algorithm = identity
dod = identity
ds = identity
enterprise = identity
organization = identity
internet = identity
iso = identity
itu_t = identity
joint_iso_ccitt = identity
keyType = identity
mechanisms = identity
member_body = identity
pkcs1 = identity
pkcs = identity
pkix = identity
prime = identity
private = identity
rsadsi = identity
schemes = identity
security = identity
signatures = identity
us = identity
zigbee = identity

# OID Categories
oidCategories = [
    ("PubKeyAlgo",         0x0100),
    ("SigAlgo",            0x0200),
    ("AttributeType",      0x0300),
    ("EllipticCurve",      0x0400),
    ("Extension",          0x0500),
    ("KeyPurpose",         0x0600)
]

# Table of well-known ASN.1 object IDs
#
oids = [

    # !!! WARNING !!!
    #
    # The enumerated values associated with individual object IDs are used in CHIP TLV encodings (notably the CHIP Certificate object).
    # Because of this, the Enum Values assigned to object IDs in this table MUST NOT BE CHANGED once in use.


    #                                              Enum
    # Category          Name                       Value    Object ID
    # ----------------- -------------------------- -------- ------------------------------------------------------------------------------------------------

    # Public Key Algorithms
    ("PubKeyAlgo",     "ECPublicKey",             1,       [
     iso(1), member_body(2), us(840), ansi_X9_62(10045), keyType(2), 1]),

    # Signature Algorithms
    # RFC 3279
    ("SigAlgo",        "ECDSAWithSHA256",         1,       [
     iso(1), member_body(2), us(840), ansi_X9_62(10045), signatures(4), 3, 2]),

    # X.509 Distinguished Name Attribute Types
    #   WARNING -- Assign no values higher than 127.
    ("AttributeType",  "CommonName",              1,
     [joint_iso_ccitt(2), ds(5), 4, 3]),
    ("AttributeType",  "Surname",                 2,
     [joint_iso_ccitt(2), ds(5), 4, 4]),
    ("AttributeType",  "SerialNumber",
     3,       [joint_iso_ccitt(2), ds(5), 4, 5]),
    ("AttributeType",  "CountryName",
     4,       [joint_iso_ccitt(2), ds(5), 4, 6]),
    ("AttributeType",  "LocalityName",
     5,       [joint_iso_ccitt(2), ds(5), 4, 7]),
    ("AttributeType",  "StateOrProvinceName",
     6,       [joint_iso_ccitt(2), ds(5), 4, 8]),
    ("AttributeType",  "OrganizationName",
     7,       [joint_iso_ccitt(2), ds(5), 4, 10]),
    ("AttributeType",  "OrganizationalUnitName",
     8,       [joint_iso_ccitt(2), ds(5), 4, 11]),
    ("AttributeType",  "Title",                   9,
     [joint_iso_ccitt(2), ds(5), 4, 12]),
    ("AttributeType",  "Name",                    10,
     [joint_iso_ccitt(2), ds(5), 4, 41]),
    ("AttributeType",  "GivenName",               11,
     [joint_iso_ccitt(2), ds(5), 4, 42]),
    ("AttributeType",  "Initials",                12,
     [joint_iso_ccitt(2), ds(5), 4, 43]),
    ("AttributeType",  "GenerationQualifier",
     13,      [joint_iso_ccitt(2), ds(5), 4, 44]),
    ("AttributeType",  "DNQualifier",             14,
     [joint_iso_ccitt(2), ds(5), 4, 46]),
    ("AttributeType",  "Pseudonym",               15,
     [joint_iso_ccitt(2), ds(5), 4, 65]),
    ("AttributeType",  "DomainComponent",         16,
     [itu_t(0), 9, 2342, 19200300, 100, 1, 25]),
    ("AttributeType",  "ChipNodeId",              17,      [iso(1), organization(
        3), dod(6), internet(1), private(4), enterprise(1), zigbee(37244), chip(1), 1]),
    ("AttributeType",  "ChipFirmwareSigningId",   18,      [iso(1), organization(
        3), dod(6), internet(1), private(4), enterprise(1), zigbee(37244), chip(1), 2]),
    ("AttributeType",  "ChipICAId",               19,      [iso(1), organization(
        3), dod(6), internet(1), private(4), enterprise(1), zigbee(37244), chip(1), 3]),
    ("AttributeType",  "ChipRootId",              20,      [iso(1), organization(
        3), dod(6), internet(1), private(4), enterprise(1), zigbee(37244), chip(1), 4]),
    ("AttributeType",  "ChipFabricId",            21,      [iso(1), organization(
        3), dod(6), internet(1), private(4), enterprise(1), zigbee(37244), chip(1), 5]),
    ("AttributeType",  "ChipAuthTag1",            22,      [iso(1), organization(
        3), dod(6), internet(1), private(4), enterprise(1), zigbee(37244), chip(1), 6]),
    ("AttributeType",  "ChipAuthTag2",            23,      [iso(1), organization(
        3), dod(6), internet(1), private(4), enterprise(1), zigbee(37244), chip(1), 7]),

    # Elliptic Curves
    ("EllipticCurve",  "prime256v1",              1,       [
     iso(1), member_body(2), us(840), ansi_X9_62(10045), curves(3), prime(1), 7]),

    # Certificate Extensions
    ("Extension",      "BasicConstraints",
     1,       [joint_iso_ccitt(2), ds(5), 29, 19]),
    ("Extension",      "KeyUsage",                2,
     [joint_iso_ccitt(2), ds(5), 29, 15]),
    ("Extension",      "ExtendedKeyUsage",
     3,       [joint_iso_ccitt(2), ds(5), 29, 37]),
    ("Extension",      "SubjectKeyIdentifier",
     4,       [joint_iso_ccitt(2), ds(5), 29, 14]),
    ("Extension",      "AuthorityKeyIdentifier",
     5,       [joint_iso_ccitt(2), ds(5), 29, 35]),

    # Key Purposes
    ("KeyPurpose",     "ServerAuth",              1,       [iso(1), organization(
        3), dod(6), internet(1), security(5), mechanisms(5), pkix(7), 3, 1]),
    ("KeyPurpose",     "ClientAuth",              2,       [iso(1), organization(
        3), dod(6), internet(1), security(5), mechanisms(5), pkix(7), 3, 2]),
    ("KeyPurpose",     "CodeSigning",             3,       [iso(1), organization(
        3), dod(6), internet(1), security(5), mechanisms(5), pkix(7), 3, 3]),
    ("KeyPurpose",     "EmailProtection",         4,       [iso(1), organization(
        3), dod(6), internet(1), security(5), mechanisms(5), pkix(7), 3, 4]),
    ("KeyPurpose",     "TimeStamping",            5,       [iso(1), organization(
        3), dod(6), internet(1), security(5), mechanisms(5), pkix(7), 3, 8]),
    ("KeyPurpose",     "OCSPSigning",             6,       [iso(1), organization(
        3), dod(6), internet(1), security(5), mechanisms(5), pkix(7), 3, 9]),
]


def encodeOID(oid):

    assert len(oid) >= 2

    oid = [(oid[0]*40 + oid[1])] + oid[2:]

    encodedOID = []
    for val in oid:
        val, byte = divmod(val, 128)
        seg = [byte]
        while val > 0:
            val, byte = divmod(val, 128)
            seg.insert(0, byte + 0x80)
        encodedOID += (seg)

    return encodedOID


TEMPLATE = '''/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the \"License\");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an \"AS IS\" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */

/**
 *    @file
 *      ASN.1 Object ID Definitions
 *
 *      !!! WARNING !!! WARNING !!! WARNING !!!
 *
 *      DO NOT EDIT THIS FILE! This file is generated by the
 *      gen-oid-table.py script.
 *
 *      To make changes, edit the script and re-run it to generate
 *      this file.
 *
 */

#pragma once

#include <cstdint>

namespace chip {
namespace ASN1 {

enum OIDCategory
%(oid_category_enums)s

typedef uint16_t OID;

enum
%(oid_enums)s

struct OIDTableEntry
{
    OID EnumVal;
    const uint8_t *EncodedOID;
    uint16_t EncodedOIDLen;
};

struct OIDNameTableEntry
{
    OID EnumVal;
    const char *Name;
};

extern const OIDTableEntry sOIDTable[];
extern const OIDNameTableEntry sOIDNameTable[];
extern const size_t sOIDTableSize;

#ifdef ASN1_DEFINE_OID_TABLE
%(oid_utf8_strings)s

const OIDTableEntry sOIDTable[] =
%(oid_table)s

const size_t sOIDTableSize = sizeof(sOIDTable) / sizeof(OIDTableEntry);

#endif // ASN1_DEFINE_OID_TABLE

#ifdef ASN1_DEFINE_OID_NAME_TABLE

const OIDNameTableEntry sOIDNameTable[] =
%(oid_name_table)s

#endif // ASN1_DEFINE_OID_NAME_TABLE

} // namespace ASN1
} // namespace chip
'''

oid_category_enums = "{\n"
for (catName, catEnum) in oidCategories:
    oid_category_enums += "    kOIDCategory_%s = 0x%04X,\n" % (
        catName, catEnum)
oid_category_enums += '''
    kOIDCategory_NotSpecified = 0,
    kOIDCategory_Unknown = 0x0F00,
    kOIDCategory_Mask = 0x0F00
};'''

oid_enums = "{\n"
for (catName, catEnum) in oidCategories:
    for (oidCatName, oidName, oidEnum, oid) in oids:
        if (oidCatName == catName):
            oid_enums += "    kOID_%s_%s = 0x%04X,\n" % (
                catName, oidName, catEnum + oidEnum)
    oid_enums += "\n"
oid_enums += '''    kOID_NotSpecified = 0,
    kOID_Unknown = 0xFFFF,
    kOID_EnumMask = 0x00FF
};'''

oid_utf8_strings = "\n"
for (catName, oidName, oidEnum, oid) in oids:
    oid_utf8_strings += "static const uint8_t sOID_%s_%s[] = { %s };\n" % (
        catName, oidName, ", ".join(["0x%02X" % (x) for x in encodeOID(oid)]))

oid_table = "{\n"
for (catName, oidName, oidEnum, oid) in oids:
    oid_table += "    { kOID_%s_%s, sOID_%s_%s, sizeof(sOID_%s_%s) },\n" % (
        catName, oidName, catName, oidName, catName, oidName)
oid_table += "    { kOID_NotSpecified, NULL, 0 }\n};"

oid_name_table = "{\n"
for (catName, oidName, oidEnum, oid) in oids:
    oid_name_table += "    { kOID_%s_%s, \"%s\" },\n" % (
        catName, oidName, oidName)
oid_name_table += "    { kOID_NotSpecified, NULL }\n};"


def main(argv):
    parser = optparse.OptionParser()

    parser.add_option('--output_file')

    options, _ = parser.parse_args(argv)

    template_args = {
        'oid_category_enums': oid_category_enums,
        'oid_enums': oid_enums,
        'oid_utf8_strings': oid_utf8_strings,
        'oid_table': oid_table,
        'oid_name_table': oid_name_table,
    }

    with open(options.output_file, 'w') as asn1oid_file:
        asn1oid_file.write(TEMPLATE % template_args)

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

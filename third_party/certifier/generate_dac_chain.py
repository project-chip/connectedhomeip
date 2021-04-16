#!/usr/bin/python

import sys
import subprocess

from OpenSSL import crypto

def bytes_to_c_arr(data, lowercase=True):
    return [format(b, '#04x' if lowercase else '#04X') for b in data]

#print('Number of arguments:', len(sys.argv), 'arguments.')
#print('Argument List:', str(sys.argv))

#certificate_chain_dump = subprocess.run(["openssl", "pkcs12",  "-info",  "-in", str(sys.argv[1]), "-nodes", "-password", "pass:changeit"], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)

#print(certificate_chain_dump.stdout)

p12 = crypto.load_pkcs12(open(str(sys.argv[1]), 'rb').read(), b"changeit")

with open("paa_certificate", 'wb') as f:
    paa_certificate = crypto.dump_certificate(crypto.FILETYPE_ASN1, p12.get_ca_certificates()[0])

with open("pai_certificate", 'wb') as f:
    pai_certificate = crypto.dump_certificate(crypto.FILETYPE_ASN1, p12.get_ca_certificates()[1])

with open ("device_manufacturer_certificate", 'wb') as f:
    device_manufacturer_certificate = crypto.dump_certificate(crypto.FILETYPE_ASN1, p12.get_certificate())

with open("device_private_key_tmp", 'wb') as f:
    private_key = crypto.dump_privatekey(crypto.FILETYPE_ASN1, p12.get_privatekey())

with open("device_public_key_tmp", 'wb') as f:
    public_key = crypto.dump_publickey(crypto.FILETYPE_ASN1, p12.get_certificate().get_pubkey())

with open(str(sys.argv[2]), 'w') as f:
    f.write("const uint8_t paa_certificate[] = {{{}}};".format(", ".join(bytes_to_c_arr(paa_certificate))))
    f.write('\n\n')
    f.write("const uint8_t pai_certificate[] = {{{}}};".format(", ".join(bytes_to_c_arr(pai_certificate))))
    f.write('\n\n')
    f.write("const uint8_t device_manufacturer_certificate[] = {{{}}};".format(", ".join(bytes_to_c_arr(device_manufacturer_certificate))))
    f.write('\n\n')
    f.write("const uint8_t device_private_key_tmp[] = {{{}}};".format(", ".join(bytes_to_c_arr(private_key[7:7+32]))))
    f.write('\n\n')
    f.write("const uint8_t device_public_key_tmp[] = {{{}}};".format(", ".join(bytes_to_c_arr(public_key[26:]))))

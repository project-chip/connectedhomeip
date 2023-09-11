#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# Script that was used to generate CHIP Development Protuct Attestation Authority (PAA)
# certificates.
# The script expects the path to the chip-cert tool binary as an input argument.
#
# Usage example when the script is run from the CHIP SDK root directory:
#     ./credentials/development/gen-development-paa-cert.sh ./out/debug/standalone/chip-cert
#
# The result will be stored in:
#     credentials/development/attestation
#

set -e

here=${0%/*}

dest_dir="$here/attestation"

mkdir -p "$dest_dir"

if [ $# == 1 ]; then
    chip_cert_tool=$1
else
    echo "Error: Please specify exactly one input argument; the path to the chip-cert tool binary"
    exit
fi

cert_valid_from="2021-06-28 14:23:43"
cert_lifetime=4294967295
paa_key_file="$dest_dir/Chip-Development-PAA-Key"
paa_cert_file="$dest_dir/Chip-Development-PAA-Cert"

"$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Development PAA" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --out-key "$paa_key_file".pem --out "$paa_cert_file".pem

"$chip_cert_tool" convert-key "$paa_key_file".pem "$paa_key_file".der --x509-der
"$chip_cert_tool" convert-cert "$paa_cert_file".pem "$paa_cert_file".der --x509-der

# Example of how Vendor (FFF1) PAI Certificates can be generate:
#
# vid=FFF1
# pai_key_file="$dest_dir/Chip-Development-PAI-$vid-Key"
# pai_cert_file="$dest_dir/Chip-Development-PAI-$vid-Cert"
#
# "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Development PAI" --subject-vid "$vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem
#
# "$chip_cert_tool" convert-key "$pai_key_file".pem "$pai_key_file".der --x509-der
# "$chip_cert_tool" convert-cert "$pai_cert_file".pem "$pai_cert_file".der --x509-der

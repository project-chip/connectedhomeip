#!/usr/bin/env bash

#
# Copyright (c) 2021 Project CHIP Authors
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
#

# Script that can be used to generate Certification Declaration (CD)
# for testing purposes.
# The script expects the path to the chip-cert tool binary as an input argument.
#
# Usage example when the script is run from the CHIP SDK root directory:
#     ./credentials/test/gen-test-cds.sh ./out/debug/standalone/chip-cert
#
# The result will be stored in:
#     credentials/test/certification-declaration
#
# If the intention is to re-generate a new set of CDs that replace the already
# present ones then it is recommended to clear the folder:
#     rm credentials/test/certification-declaration/*
#

set -e

here=${0%/*}

dest_dir="$here/certification-declaration"

mkdir -p "$dest_dir"

if [ $# == 1 ]; then
    chip_cert_tool=$1
else
    echo "Error: Please specify exactly one input argument; the path to the chip-cert tool binary"
    exit
fi

cert_valid_from="2021-06-28 14:23:43"
cert_lifetime=4294967295

format_version=1
vids=(FFF2 FFF3)
pid0=8001
pid1=8002
device_type_id=0x1234
certificate_id0="ZIG20141ZB330001-24"
certificate_id1="ZIG20142ZB330002-24"
security_level=0
security_info=0
version_num=9876
certification_type=0
dac_origin_vendor_id=0xFFF1
dac_origin_product_id=0x8000

cd_signing_key="$dest_dir/Chip-Test-CD-Signing-Key.pem"
cd_signing_cert="$dest_dir/Chip-Test-CD-Signing-Cert.pem"

# Using gen-att-cert command to generate CD Signing Cert/Key:
"$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Test CD Signing Authority" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --out-key "$cd_signing_key" --out "$cd_signing_cert"

for vid in "${vids[@]}"; do

    "$chip_cert_tool" gen-cd --key "$cd_signing_key" --cert "$cd_signing_cert" --out "$dest_dir/Chip-Test-CD-$vid-$pid0.der" --format-version "$format_version" --vendor-id "0x$vid" --product-id "0x$pid0" --device-type-id "$device_type_id" --certificate-id "$certificate_id0" --security-level "$security_level" --security-info "$security_info" --version-number "$version_num" --certification-type "$certification_type"

    "$chip_cert_tool" gen-cd --key "$cd_signing_key" --cert "$cd_signing_cert" --out "$dest_dir/Chip-Test-CD-$vid-$pid1.der" --format-version "$format_version" --vendor-id "0x$vid" --product-id "0x$pid1" --device-type-id "$device_type_id" --certificate-id "$certificate_id1" --security-level "$security_level" --security-info "$security_info" --version-number "$version_num" --certification-type "$certification_type"

    "$chip_cert_tool" gen-cd --key "$cd_signing_key" --cert "$cd_signing_cert" --out "$dest_dir/Chip-Test-CD-$vid-$pid1-WithDACOrigin.der" --format-version "$format_version" --vendor-id "0x$vid" --product-id "0x$pid1" --device-type-id "$device_type_id" --certificate-id "$certificate_id1" --security-level "$security_level" --security-info "$security_info" --version-number "$version_num" --certification-type "$certification_type" --dac-origin-vendor-id "$dac_origin_vendor_id" --dac-origin-product-id "$dac_origin_product_id"

    "$chip_cert_tool" gen-cd --key "$cd_signing_key" --cert "$cd_signing_cert" --out "$dest_dir/Chip-Test-CD-$vid-$pid0-$pid1.der" --format-version "$format_version" --vendor-id "0x$vid" --product-id "0x$pid0" --product-id "0x$pid1" --device-type-id "$device_type_id" --certificate-id "$certificate_id1" --security-level "$security_level" --security-info "$security_info" --version-number "$version_num" --certification-type "$certification_type"

    "$chip_cert_tool" gen-cd --key "$cd_signing_key" --cert "$cd_signing_cert" --out "$dest_dir/Chip-Test-CD-$vid-$pid0-$pid1-WithDACOrigin.der" --format-version "$format_version" --vendor-id "0x$vid" --product-id "0x$pid0" --product-id "0x$pid1" --device-type-id "$device_type_id" --certificate-id "$certificate_id1" --security-level "$security_level" --security-info "$security_info" --version-number "$version_num" --certification-type "$certification_type" --dac-origin-vendor-id "$dac_origin_vendor_id" --dac-origin-product-id "$dac_origin_product_id"

done

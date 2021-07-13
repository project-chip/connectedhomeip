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

# Script that can be used to generate CHIP device attestation certificates
# for testing purposes.
#
# Usage:
#     ./gen-test-attestation-certs.sh
# or from the root directory:
#     ./credentials/test/gen-test-attestation-certs.sh
#
# The result will be stored in:
#     credentials/test/attestation
#
# If the intention is to re-generate new set of attestation certificates that
# already present then it is recommended remove current certificates:
#     rm credentials/test/attestation/*
#

here=${0%/*}

set -e

CHIP_ROOT=$(cd "$here/../.." && pwd)

dest_dir="$CHIP_ROOT/credentials/test/attestation"

mkdir -p "$dest_dir"

chip_cert_tool="$CHIP_ROOT/out/debug/linux_x64_gcc/obj/src/tools/chip-cert/bin/chip-cert"

cert_valid_from="2021-06-28 14:23:43"
cert_lifetime=7305

paa_vids=(FFF1 FFF2)
pai_pids=(ABC0 ABC1)
dac_ids=(0001 0002 0003 0004 0005 0006 0007 0008 0009 000A 000B 000C 000D 000E 000F)

for vid in "${paa_vids[@]}"; do
    paa_key_file="$dest_dir/Chip-Test-PAA-$vid-Key"
    paa_cert_file="$dest_dir/Chip-Test-PAA-$vid-Cert"

    "$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Test PAA $vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --out-key "$paa_key_file".pem --out "$paa_cert_file".pem

    "$chip_cert_tool" convert-key "$paa_key_file".pem "$paa_key_file".der --x509-der
    "$chip_cert_tool" convert-cert "$paa_cert_file".pem "$paa_cert_file".der --x509-der

    for pid in "${pai_pids[@]}"; do
        pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Key"
        pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Cert"

        "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

        "$chip_cert_tool" convert-key "$pai_key_file".pem "$pai_key_file".der --x509-der
        "$chip_cert_tool" convert-cert "$pai_cert_file".pem "$pai_cert_file".der --x509-der

        for dac in "${dac_ids[@]}"; do
            dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Key"
            dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Cert"

            "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

            "$chip_cert_tool" convert-key "$dac_key_file".pem "$dac_key_file".der --x509-der
            "$chip_cert_tool" convert-cert "$dac_cert_file".pem "$dac_cert_file".der --x509-der
        done
    done
done

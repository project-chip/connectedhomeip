#!/usr/bin/env bash

#
# Copyright (c) 2021-2023 Project CHIP Authors
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
# The script expects the path to the chip-cert tool binary as an input argument.
#
# Usage example when the script is run from the CHIP SDK root directory:
#     ./credentials/test/gen-test-attestation-certs.sh ./out/debug/standalone/chip-cert
#
# The result will be stored in:
#     credentials/test/attestation
# If the intention is to re-generate a new set of attestation certificates that
# replace the already present ones then it is recommended to remove current certificates:
#     rm credentials/test/attestation/*
#
# Second example also generates C-Style file with those certificates/keys to be use by the SDK tests:
#     ./credentials/test/gen-test-attestation-certs.sh ./out/debug/standalone/chip-cert src/credentials/tests/CHIPAttCert_test_vectors
#
# In addition to the DER/PEM files this command also generates the following C-Style files:
#     src/credentials/tests/CHIPAttCert_test_vectors.cpp
#     src/credentials/tests/CHIPAttCert_test_vectors.h
#

set -e

here=${0%/*}

dest_dir="$here/attestation"

rm -rf "$dest_dir"
mkdir -p "$dest_dir"

if [ $# == 1 ]; then
    chip_cert_tool=$1
elif [ $# == 2 ]; then
    chip_cert_tool=$1
    output_cstyle_file=$2
else
    echo "Error: Please specify one or two input arguments, where first argument is the path to the chip-cert tool binary"
    exit
fi

cert_valid_from="2021-06-28 14:23:43"
cert_lifetime=4294967295

# Set #1:
#   - PAA Subject includes VID
#   - PAI Subject includes PID
{
    vid=FFF1
    pid=8000
    dac_ids=(0000 0001 0002 0003 0004 0005 0006 0007)

    paa_key_file="$dest_dir/Chip-Test-PAA-$vid-Key"
    paa_cert_file="$dest_dir/Chip-Test-PAA-$vid-Cert"

    "$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Test PAA" --subject-vid "$vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --out-key "$paa_key_file".pem --out "$paa_cert_file".pem

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    for dac in "${dac_ids[@]}"; do
        dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Key"
        dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Cert"

        "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem
    done
}

# Set #2:
#   - PAA Subject doesn't include VID
#   - PAI Subject includes PID
{
    vid=FFF2
    pid=8001
    dac_ids=(0008 0009 000A 000B 000C 000D 000E 000F)

    paa_key_file="$dest_dir/Chip-Test-PAA-NoVID-Key"
    paa_cert_file="$dest_dir/Chip-Test-PAA-NoVID-Cert"

    "$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Test PAA" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --out-key "$paa_key_file".pem --out "$paa_cert_file".pem

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    for dac in "${dac_ids[@]}"; do
        dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Key"
        dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Cert"

        "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem
    done
}

# Set #3:
#   - PAA Subject doesn't include VID (the same PAA from Set #2 is used)
#   - PAI Subject doesn't include PID
{
    pid=8002
    dac_ids=(0010 0011 0012 0013 0014 0015 0016 0017)

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    for dac in "${dac_ids[@]}"; do
        dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Key"
        dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Cert"

        "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem
    done
}

# Set #4:
#   - PAA Subject doesn't include VID (the same PAA from Set #2 is used)
#   - PAI Subject doesn't include VID and PID encoded using fallback method
#   - DAC Subject VID and PID are encoded using fallback method
{
    pid=8003
    dac_ids=(0018 0019 001A 001B)

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-FB-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-FB-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --vid-pid-as-cn --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    for dac in "${dac_ids[@]}"; do
        dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-FB-Key"
        dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-FB-Cert"

        "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --vid-pid-as-cn --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem
    done
}

# Set #5:
#   - PAA Subject doesn't include VID (the same PAA from Set #2 is used)
#   - PAI Subject includes PID and both VID and PID are encoded using fallback method
#   - DAC Subject VID and PID are encoded using fallback method
{
    pid=8004
    dac_ids=(001C 001D 001E 001F)

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-FB-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-FB-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --vid-pid-as-cn --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    for dac in "${dac_ids[@]}"; do
        dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-FB-Key"
        dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-FB-Cert"

        "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --vid-pid-as-cn --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem
    done
}

# Set #6:
#   Generating certificates chains { PAA, PAI, DAC } with one of the certificates
#   in the chain either valid in the past or in the future.
#   Note that all these chains should pass validation according to Matter specifications
{
    cert_valid_from_in_future="2031-06-28 14:23:43"
    cert_valid_from_1sec_before_in_future="2031-06-28 14:23:42"
    cert_valid_from_1sec_before="2021-06-28 14:23:42"
    cert_lifetime_1year=365

    # Generating DAC with validity in the past with PAA from Set #2 and PAI from Set #5
    dac=0020

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInPast-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInPast-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime_1year" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Error Case: Generating DAC with validity 1 sec before validity of PAI/PAA with PAA from Set #2 and PAI from Set #5
    dac=0030

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Val1SecBefore-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Val1SecBefore-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_1sec_before" --lifetime "$cert_lifetime_1year" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Generating DAC with validity in the future with PAA from Set #2 and PAI from Set #5
    dac=0021

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInFuture-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInFuture-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_in_future" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Generating PAI and DAC with validity in the past with PAA from Set #2
    pid=8005
    dac=0022

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ValInPast-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ValInPast-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime_1year" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInPast-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInPast-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime_1year" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Error Case: Generating PAI and DAC with validity 1 sec before validity of PAA with PAA from Set #2
    dac=0032

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Val1SecBefore-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Val1SecBefore-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_1sec_before" --lifetime "$cert_lifetime_1year" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Val1SecBefore-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Val1SecBefore-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_1sec_before" --lifetime "$cert_lifetime_1year" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Generating PAI and DAC with validity in the future with PAA from Set #2
    dac=0023

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ValInFuture-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ValInFuture-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_in_future" --lifetime "$cert_lifetime_1year" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInFuture-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInFuture-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_in_future" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Error Case: same PAA and PAI and generating DAC with validity 1 sec before validity of PAI with PAA
    dac=0033

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Val1SecBefore-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Val1SecBefore-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_1sec_before_in_future" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Generating PAA and PAI with validity in the past with DAC valid in the past but doesn't expire
    pid=8006
    dac=0024

    paa_key_file="$dest_dir/Chip-Test-PAA-$vid-ValInPast-Key"
    paa_cert_file="$dest_dir/Chip-Test-PAA-$vid-ValInPast-Cert"

    "$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Test PAA" --subject-vid "$vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime_1year" --out-key "$paa_key_file".pem --out "$paa_cert_file".pem

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ValInPast-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ValInPast-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime_1year" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInPast-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInPast-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Error Case: Same PAA and PAI valid only for one year in the past with DAC valid in the future
    dac=0034

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInFuture-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInFuture-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_in_future" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Generating PAA and PAI and DAC with validity in the future; the DAC doesn't expire
    dac=0025

    paa_key_file="$dest_dir/Chip-Test-PAA-$vid-ValInFuture-Key"
    paa_cert_file="$dest_dir/Chip-Test-PAA-$vid-ValInFuture-Cert"

    "$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Test PAA" --subject-vid "$vid" --valid-from "$cert_valid_from_in_future" --lifetime "$cert_lifetime_1year" --out-key "$paa_key_file".pem --out "$paa_cert_file".pem

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ValInFuture-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ValInFuture-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_in_future" --lifetime "$cert_lifetime_1year" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInFuture-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-ValInFuture-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_in_future" --lifetime "$cert_lifetime" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    # Error Case: Same PAA and PAI valid in future with DAC valid 1 sec before validity of PAI with PAA
    dac=0035

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Val1SecBefore-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-Val1SecBefore-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from_1sec_before_in_future" --lifetime "$cert_lifetime_1year" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem
}

# Set #7:
#   - Generate new PAA to resign already generated PAIs
{
    vid=FFF2
    pid=8001

    paa_key_file="$dest_dir/Chip-Test-PAA-NoVID-Key"
    paa_cert_file="$dest_dir/Chip-Test-PAA-NoVID-ToResignPAIs-Cert"

    "$chip_cert_tool" gen-att-cert --type a --subject-cn "Matter Test PAA To Resign PAIs" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --key "$paa_key_file".pem --out "$paa_cert_file".pem

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Resigned-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --key "$pai_key_file".pem --out "$pai_cert_file".pem

    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ResignedSubjectDiff-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI Resigned" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --key "$pai_key_file".pem --out "$pai_cert_file".pem

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ResignedSKIDDiff-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-ResignedSKIDDiff-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-NoPID-Resigned-Cert"

    "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Test PAI" --subject-vid "$vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --key "$pai_key_file".pem --out "$pai_cert_file".pem
}

# Set #8:
#   - Generate DACs with CRL Distribution Point (CDP) Extensions (Valid and Invalid cases)
{
    vid=FFF1
    pid=8000
    dac=0000

    pai_key_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Key"
    pai_cert_file="$dest_dir/Chip-Test-PAI-$vid-$pid-Cert"

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Cert"

    cdp_uri="http://example.com/crl.pem"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP (HTTP)" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-HTTPS-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-HTTPS-Cert"

    cdp_uri="https://example.com/crl.pem"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP (HTTPS)" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-2URIs-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-2URIs-Cert"

    cdp_error_inject="ext-cdp-uri-duplicate"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP (Two URIs)" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" -I -E "$cdp_error_inject" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-2DPs-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-2DPs-Cert"

    cdp_error_inject="ext-cdp-dist-point-duplicate"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac Two CDP (Two DPs)" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" -I -E "$cdp_error_inject" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-2CDPs-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-2CDPs-Cert"

    cdp_error_inject="ext-cdp-add"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac Two CDPs" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" -I -E "$cdp_error_inject" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Long-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Long-Cert"

    cdp_uri="https://example.com/this-is-an-example-of-crl-distribution-point-extension-which-is-101-chars/crl.pem"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Long" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Wrong-Prefix-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Wrong-Prefix-Cert"

    cdp_uri="www.example.com/crl.pem"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP Wrong Prefix" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Issuer-PAA-FFF1-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Issuer-PAA-FFF1-Cert"
    issuer_cert_file="$dest_dir/Chip-Test-PAA-$vid-Cert"

    cdp_uri="https://example.com/crl.pem"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP Issuer PAA FFF1" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem --crl-issuer-cert "$issuer_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-2CRLIssuers-PAA-FFF1-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-2CRLIssuers-PAA-FFF1-Cert"
    cdp_error_inject="ext-cdp-crl-issuer-duplicate"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP 2 Issuers PAA FFF1" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" -I -E "$cdp_error_inject" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem --crl-issuer-cert "$issuer_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-CRL-Issuer-PAA-FFF1-2DPs-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-CRL-Issuer-PAA-FFF1-2DPs-Cert"
    cdp_error_inject="ext-cdp-dist-point-duplicate"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP Issuer PAA FFF1 (Two DPs)" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" -I -E "$cdp_error_inject" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem --crl-issuer-cert "$issuer_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-2CDPs-Issuer-PAA-FFF1-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-2CDPs-Issuer-PAA-FFF1-Cert"
    cdp_error_inject="ext-cdp-add"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac 2CDPs Issuer PAA FFF1" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" -I -E "$cdp_error_inject" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem --crl-issuer-cert "$issuer_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Issuer-PAA-NoVID-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Issuer-PAA-NoVID-Cert"
    issuer_cert_file="$dest_dir/Chip-Test-PAA-NoVID-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP Issuer PAA NoVID" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem --crl-issuer-cert "$issuer_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Issuer-PAI-FFF2-8004-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Issuer-PAI-FFF2-8004-Cert"
    issuer_cert_file="$dest_dir/Chip-Test-PAI-FFF2-8004-FB-Cert"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP Issuer PAI FFF2 8004" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem --crl-issuer-cert "$issuer_cert_file".pem

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-2CDPs-Issuer-PAI-FFF2-8004-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-2CDPs-Issuer-PAI-FFF2-8004-Cert"
    cdp_error_inject="ext-cdp-dist-point-duplicate"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Matter Test DAC $dac CDP Issuer PAI FFF2 8004 Two CDPs" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem --crl-issuer-cert "$issuer_cert_file".pem -I -E "$cdp_error_inject"

    dac_key_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Issuer-PAI-FFF2-8004-Long-Key"
    dac_cert_file="$dest_dir/Chip-Test-DAC-$vid-$pid-$dac-CDP-Issuer-PAI-FFF2-8004-Long-Cert"
    cdp_uri="https://example.com/this-is-an-example-of-crl-distribution-point-extension-which-is-101-chars/crl.pem"

    "$chip_cert_tool" gen-att-cert --type d --subject-cn "Long" --subject-vid "$vid" --subject-pid "$pid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --cdp-uri "$cdp_uri" --ca-key "$pai_key_file".pem --ca-cert "$pai_cert_file".pem --out-key "$dac_key_file".pem --out "$dac_cert_file".pem --crl-issuer-cert "$issuer_cert_file".pem
}

# In addition to PEM format also create certificates in DER form.
for cert_file_pem in "$dest_dir"/*Cert.pem; do
    cert_file_der="${cert_file_pem/.pem/.der}"
    "$chip_cert_tool" convert-cert "$cert_file_pem" "$cert_file_der" --x509-der
done

# In addition to PEM format also create private key in DER form.
for key_file_pem in "$dest_dir"/*Key.pem; do
    key_file_der="${key_file_pem/.pem/.der}"
    "$chip_cert_tool" convert-key "$key_file_pem" "$key_file_der" --x509-der
done

# Print generated certificate, keys, and parameters in C-Style to use in the SDK if the output file is provided.
if [ ! -z "$output_cstyle_file" ]; then

    copyright_note='/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
'

    cpp_includes='
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
'
    header_includes='
#pragma once

#include <lib/support/Span.h>
'

    namespaces_open='
namespace chip {
namespace TestCerts {
'
    namespaces_close='
} // namespace TestCerts
} // namespace chip
'

    printf "$copyright_note" >"$output_cstyle_file".cpp
    printf "$copyright_note" >"$output_cstyle_file".h
    printf "$cpp_includes" >>"$output_cstyle_file".cpp
    printf "$header_includes" >>"$output_cstyle_file".h
    printf "$namespaces_open\n" >>"$output_cstyle_file".cpp
    printf "$namespaces_open\n" >>"$output_cstyle_file".h
    for cert_file_pem in "$dest_dir"/*Cert.pem; do
        params_prefix="${cert_file_pem/*Chip-Test/sTestCert}"
        params_prefix="${params_prefix//-/_}"
        params_prefix="${params_prefix/_Cert.pem/}"

        cert_file_der="${cert_file_pem/.pem/.der}"
        key_file_pem="${cert_file_pem/Cert.pem/Key.pem}"

        {
            printf "// \${chip_root}/$cert_file_pem\n\n"

            printf "constexpr uint8_t ${params_prefix}_Cert_Array[] = {\n"
            less -f "$cert_file_der" | od -t x1 -An | sed 's/\</0x/g' | sed 's/\>/,/g' | sed 's/^/   /g'
            printf "};\n\n"
            printf "extern const ByteSpan ${params_prefix}_Cert = ByteSpan(${params_prefix}_Cert_Array);\n\n"

            printf "constexpr uint8_t ${params_prefix}_SKID_Array[] = {\n"
            openssl x509 -text -noout -in "$cert_file_pem" | sed '0,/X509v3 Subject Key Identifier:/d' | sed '2,$d' | sed 's/:/ /g' | sed 's/\</0x/g' | sed 's/\>/,/g' | sed "s/^[ \t]*/    /"
            printf "};\n\n"
            printf "extern const ByteSpan ${params_prefix}_SKID = ByteSpan(${params_prefix}_SKID_Array);\n\n"

            # Print key data if present
            if test -f "$key_file_pem"; then
                printf "// \${chip_root}/$key_file_pem\n\n"

                printf "constexpr uint8_t ${params_prefix}_PublicKey_Array[] = {\n"
                openssl ec -text -noout -in "$key_file_pem" | sed '0,/pub:$/d' | sed '/ASN1 OID:/,$d' | sed 's/:/ /g' | sed 's/\</0x/g' | sed 's/\>/,/g' | sed "s/^[ \t]*/    /" | sed 's/ *$//'
                printf "};\n\n"
                printf "extern const ByteSpan ${params_prefix}_PublicKey = ByteSpan(${params_prefix}_PublicKey_Array);\n\n"

                printf "constexpr uint8_t ${params_prefix}_PrivateKey_Array[] = {\n"
                openssl ec -text -noout -in "$key_file_pem" | sed '0,/priv:$/d' | sed '/pub:/,$d' | sed 's/:/ /g' | sed 's/\</0x/g' | sed 's/\>/,/g' | sed "s/^[ \t]*/    /" | sed 's/ *$//'
                printf "};\n\n"
                printf "extern const ByteSpan ${params_prefix}_PrivateKey = ByteSpan(${params_prefix}_PrivateKey_Array);\n\n"
            fi
        } >>"$output_cstyle_file".cpp

        {
            printf "extern const ByteSpan ${params_prefix}_Cert;\n"
            printf "extern const ByteSpan ${params_prefix}_SKID;\n"
            # Print key data if present
            if test -f "$key_file_pem"; then
                printf "extern const ByteSpan ${params_prefix}_PublicKey;\n"
                printf "extern const ByteSpan ${params_prefix}_PrivateKey;\n"
            fi
            printf "\n"
        } >>"$output_cstyle_file".h

    done
    printf "$namespaces_close" >>"$output_cstyle_file".cpp
    printf "$namespaces_close" >>"$output_cstyle_file".h
fi

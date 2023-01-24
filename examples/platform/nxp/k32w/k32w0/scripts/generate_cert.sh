#!/usr/bin/env bash
#
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

#
#    Description:
#      This is a utility script that creates PAA, PAI and DAC certificates for NXP factory data.
#

if [ -z "$1" ]; then
    echo "Usage: ./generate_cert.sh chip-cert-path"
    exit 1
fi

CHIP_CERT_TOOL="$1"

function exit_err() {
    echo "${1}"
    exit 1
}

DATE="2022-06-21 12:35:00"
LIFETIME="7305"

PAA_DATE="$DATE"
PAA_LIFETIME="$LIFETIME"
PAA_CERT="Chip-PAA-NXP-Cert.pem"
PAA_KEY="Chip-PAA-NXP-Key.pem"

PAI_DATE="$DATE"
PAI_LIFETIME="$LIFETIME"
PAI_VID="1037"
PAI_PID="A220"
PAI_CERT="Chip-PAI-NXP-"$PAI_VID"-"$PAI_PID"-Cert.pem"
PAI_CERT_DER="Chip-PAI-NXP-"$PAI_VID"-"$PAI_PID"-Cert.der"
PAI_KEY="Chip-PAI-NXP-"$PAI_VID"-"$PAI_PID"-Key.pem"

DAC_DATE="$DATE"
DAC_LIFETIME="$LIFETIME"
DAC_VID="$PAI_VID"
DAC_PID="$PAI_PID"
DAC_CERT="Chip-DAC-NXP-"$DAC_VID"-"$DAC_PID"-Cert.pem"
DAC_CERT_DER="Chip-DAC-NXP-"$DAC_VID"-"$DAC_PID"-Cert.der"
DAC_KEY="Chip-DAC-NXP-"$DAC_VID"-"$DAC_PID"-Key.pem"
DAC_KEY_DER="Chip-DAC-NXP-"$DAC_VID"-"$DAC_PID"-Key.der"

# Remove certificates if present
rm -rf "$PAA_CERT" "$PAA_KEY" "$PAI_CERT" "$PAI_KEY" "$DAC_CERT" "$DAC_KEY" "$PAI_CERT_DER" "$DAC_CERT_DER" "$DAC_KEY_DER" >/dev/null 2>&1

# Generate certificates
echo "Generate certificates"

# PAA (root authoritity)
"$CHIP_CERT_TOOL" gen-att-cert --type a --subject-cn "Matter Development PAA NXP" --valid-from "$PAA_DATE" --lifetime "$PAA_LIFETIME" --out-key "$PAA_KEY" --out "$PAA_CERT" && echo "Generated PAA" || exit_err "Failed to generate PAA"

# PAI (vendor)
"$CHIP_CERT_TOOL" gen-att-cert --type i --subject-cn "Matter Development PAI NXP" --subject-vid "$PAI_VID" --valid-from "$PAI_DATE" --lifetime "$PAI_LIFETIME" --ca-key "$PAA_KEY" --ca-cert "$PAA_CERT" --out-key "$PAI_KEY" --out "$PAI_CERT" && echo "Generated PAI" || exit_err "Failed to generate PAI"

# DAC (product)
"$CHIP_CERT_TOOL" gen-att-cert --type d --subject-cn "Matter Development DAC NXP" --subject-vid "$DAC_VID" --subject-pid "$DAC_PID" --valid-from "$DAC_DATE" --lifetime "$DAC_LIFETIME" --ca-key "$PAI_KEY" --ca-cert "$PAI_CERT" --out-key "$DAC_KEY" --out "$DAC_CERT" && echo "Generated DAC" || exit_err "Failed to generate DAC"

# Convert certificates and keys to der format (binary x509)
echo "Convert certificates and keys to DER format"

# PAI
"$CHIP_CERT_TOOL" convert-cert -d "$PAI_CERT" "$PAI_CERT_DER" && echo "Converted PAI" || exit_err "Failed to convert PAI"

# DAC Cer
"$CHIP_CERT_TOOL" convert-cert -d "$DAC_CERT" "$DAC_CERT_DER" && echo "Converted DAC Cert" || exit_err "Failed to convert DAC Cert"

# DAC Key
"$CHIP_CERT_TOOL" convert-key -d "$DAC_KEY" "$DAC_KEY_DER" && echo "Converted DAC Key" || exit_err "Failed to convert DAC Key"

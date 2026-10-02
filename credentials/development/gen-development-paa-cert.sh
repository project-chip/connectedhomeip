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

# Generate development-only PAA roots.
# Usage: ./credentials/development/gen-development-paa-cert.sh [--pqc] [--overwrite] /path/to/chip-cert
# By default, generate the legacy ECDSA root in attestation/.
# With --pqc, generate ML-DSA-44 and ML-DSA-65 roots using an OpenSSL 3.5+ chip-cert build.
# PQC keys stay in attestation/; public certificate pairs go in paa-root-certs/.
# To regenerate checked-in roots, back up their certificates and keys, then use --overwrite.

set -euo pipefail
umask 077

overwrite=false
pqc=false
while [[ $# -gt 0 ]]; do
    case "$1" in
        --overwrite) overwrite=true ;;
        --pqc) pqc=true ;;
        *) break ;;
    esac
    shift
done
if [[ $# != 1 || ${1:-} == -* ]]; then
    echo "Usage: $0 [--pqc] [--overwrite] /path/to/chip-cert" >&2
    exit 1
fi

chip_cert_tool=$1
here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
key_dir="$here/attestation"
# Preserve the original legacy filenames and output directory for existing users.
cert_dir="$key_dir"
names=(Chip-Development-PAA)
if [[ $pqc == true ]]; then
    cert_dir="$here/paa-root-certs"
    names=(Chip-Development-PAA-ML-DSA-44 Chip-Development-PAA-ML-DSA-65)
fi

if [[ $overwrite == true ]]; then
    cat >&2 <<'EOF'
WARNING: --overwrite will replace the selected development PAA certificates and private keys.
Back up these certificates and keys before running with --overwrite so you can restore the existing chains.
Existing PAIs and DACs that chain to the current PAAs will not validate against the new PAA certificates.
You will need to generate new PAIs and DACs chaining to the new PAA certificates.
EOF
fi

# Check all outputs before generating anything; replacement requires explicit opt-in.
for name in "${names[@]}"; do
    for output in "$key_dir/$name-Key.pem" "$key_dir/$name-Key.der" \
        "$cert_dir/$name-Cert.pem" "$cert_dir/$name-Cert.der"; do
        if [[ -d "$output" ]]; then
            echo "Output path is a directory: $output" >&2
            exit 1
        fi
        if [[ $overwrite == false && (-e "$output" || -L "$output") ]]; then
            echo "Refusing to overwrite: $output (back up existing roots, then use --overwrite)" >&2
            exit 1
        fi
    done
done

if ! command -v "$chip_cert_tool" >/dev/null 2>&1; then
    echo "Cannot execute chip-cert: $chip_cert_tool" >&2
    exit 1
fi

# Let chip-cert validate ML-DSA support using its actual runtime libraries.
# Generate and convert all roots in staging before replacing existing files.
staging_dir=$(mktemp -d "$here/.paa.XXXXXX")
trap 'rm -rf -- "$staging_dir"' EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

for name in "${names[@]}"; do
    key="$staging_dir/$name-Key"
    cert="$staging_dir/$name-Cert"

    subject_cn="Matter Development PAA"
    valid_from="2021-06-28 14:23:43"
    key_options=()
    if [[ $pqc == true ]]; then
        variant=${name##*-}
        subject_cn="Matter Development PAA ML-DSA-$variant"
        key_options=(--subject-vid FFF1 --key-type "ml-dsa-$variant")
    fi

    "$chip_cert_tool" gen-att-cert \
        --type a \
        --subject-cn "$subject_cn" \
        "${key_options[@]}" \
        --valid-from "$valid_from" \
        --lifetime 4294967295 \
        --out-key "$key.pem" \
        --out "$cert.pem"

    "$chip_cert_tool" convert-key "$key.pem" "$key.der" --x509-der
    "$chip_cert_tool" convert-cert "$cert.pem" "$cert.der" --x509-der
    chmod 644 "$cert.pem" "$cert.der"
done

# Publish only after generation and conversion have succeeded for all selected roots.
mkdir -p "$key_dir" "$cert_dir"
for name in "${names[@]}"; do
    for format in pem der; do
        mv -f -- "$staging_dir/$name-Key.$format" "$key_dir/$name-Key.$format"
        mv -f -- "$staging_dir/$name-Cert.$format" "$cert_dir/$name-Cert.$format"
    done
    echo "Generated $cert_dir/$name-Cert.pem and $cert_dir/$name-Cert.der"
done

# Example of how Vendor (FFF1) PAI Certificates can be generated from the legacy root:
#
# dest_dir="$here/attestation"
# paa_key_file="$dest_dir/Chip-Development-PAA-Key"
# paa_cert_file="$dest_dir/Chip-Development-PAA-Cert"
# cert_valid_from="2021-06-28 14:23:43"
# cert_lifetime=4294967295
# vid=FFF1
# pai_key_file="$dest_dir/Chip-Development-PAI-$vid-Key"
# pai_cert_file="$dest_dir/Chip-Development-PAI-$vid-Cert"
#
# "$chip_cert_tool" gen-att-cert --type i --subject-cn "Matter Development PAI" --subject-vid "$vid" --valid-from "$cert_valid_from" --lifetime "$cert_lifetime" --ca-key "$paa_key_file".pem --ca-cert "$paa_cert_file".pem --out-key "$pai_key_file".pem --out "$pai_cert_file".pem
#
# "$chip_cert_tool" convert-key "$pai_key_file".pem "$pai_key_file".der --x509-der
# "$chip_cert_tool" convert-cert "$pai_cert_file".pem "$pai_cert_file".der --x509-der

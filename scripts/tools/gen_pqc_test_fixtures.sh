#!/usr/bin/env bash
#
# Copyright (c) 2026 Project CHIP Authors
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
# Generate the PQC attestation fixtures used by TC-DA-1.11 and TC-DA-1.13:
# an ML-DSA-44 and an ML-DSA-65 PAA/PAI chain, each ending in a P-256 DAC that
# re-certifies the development DAC key (FFF1/8001), so the app's legacy chain
# and the PQC chains share one attestation signing key.
#
# Usage: gen_pqc_test_fixtures.sh <chip-cert binary> <output directory>
#
# The chip-cert build must support --key-type ml-dsa-44/ml-dsa-65 and needs an
# OpenSSL 3.5+ backend.

set -euo pipefail

CHIP_CERT="${1:?usage: $0 <chip-cert binary> <output directory>}"
OUT_DIR="${2:?usage: $0 <chip-cert binary> <output directory>}"
REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DEV_DAC_KEY="${REPO_ROOT}/credentials/development/attestation/Matter-Development-DAC-FFF1-8001-Key.pem"
VALID_FROM="2025-01-01 00:00:00"
LIFETIME=3650

mkdir -p "$OUT_DIR"

hex_of_der() {
    xxd -p "$1" | tr -d '\n'
}

for suffix in 44 65; do
    alg="ml-dsa-${suffix}"
    paa="${OUT_DIR}/pqc-paa-${suffix}"
    pai="${OUT_DIR}/pqc-pai-${suffix}"
    dac="${OUT_DIR}/pqc-dac-${suffix}"

    "$CHIP_CERT" gen-att-cert --type a \
        --subject-cn "Matter Test PQC PAA ${alg}" --subject-vid FFF1 \
        --key-type "$alg" --valid-from "$VALID_FROM" --lifetime "$LIFETIME" \
        --out-key "${paa}-key.pem" --out "${paa}.pem"

    "$CHIP_CERT" gen-att-cert --type i \
        --subject-cn "Matter Test PQC PAI ${alg}" --subject-vid FFF1 --subject-pid 8001 \
        --key-type "$alg" --valid-from "$VALID_FROM" --lifetime "$LIFETIME" \
        --ca-key "${paa}-key.pem" --ca-cert "${paa}.pem" \
        --out-key "${pai}-key.pem" --out "${pai}.pem"

    # Re-certify the development DAC key so SignWithDeviceAttestationKey stays
    # valid for both the default legacy chain and this PQC chain.
    "$CHIP_CERT" gen-att-cert --type d \
        --subject-cn "Matter Test PQC DAC ${suffix}" --subject-vid FFF1 --subject-pid 8001 \
        --key "$DEV_DAC_KEY" --valid-from "$VALID_FROM" --lifetime "$LIFETIME" \
        --ca-key "${pai}-key.pem" --ca-cert "${pai}.pem" \
        --out "${dac}.pem"

    # Validate with OpenSSL rather than chip-cert: it is an independent check, and
    # chip-cert builds predating the PQC crypto work reject ML-DSA PAAs by format.
    openssl verify -CAfile "${paa}.pem" -untrusted "${pai}.pem" "${dac}.pem"

    for pem in "$paa" "$pai" "$dac"; do
        openssl x509 -in "${pem}.pem" -outform der -out "${pem}.der"
    done

    # A PQC-capable commissioner validates the chain up to these roots, so publish them
    # in a ready-to-use trust-store directory (pass it as --paa-trust-store-path).
    # --paa-trust-store-path replaces the commissioner's default store, so the
    # development legacy PAAs must sit alongside the ML-DSA roots or the legacy and
    # fallback rows cannot validate the compiled-in legacy chain.
    mkdir -p "${OUT_DIR}/paa-root-certs"
    cp "${paa}.der" "${OUT_DIR}/paa-root-certs/"
    cp "${REPO_ROOT}/credentials/development/paa-root-certs/Chip-Test-PAA-FFF1-Cert.der" \
        "${REPO_ROOT}/credentials/development/paa-root-certs/Chip-Test-PAA-NoVID-Cert.der" \
        "${OUT_DIR}/paa-root-certs/"

    cat >"${OUT_DIR}/pqc_dac_provider_ml_dsa_${suffix}.json" <<EOF
{
    "description": "PQC ML-DSA-${suffix} attestation chain for TC-DA-1.11/1.13; legacy chain stays the compiled-in development chain (FFF1/8001)",
    "pai_cert_ml_dsa_${suffix}": "$(hex_of_der "${pai}.der")",
    "dac_cert_ml_dsa_${suffix}": "$(hex_of_der "${dac}.der")"
}
EOF
    echo "Generated ${OUT_DIR}/pqc_dac_provider_ml_dsa_${suffix}.json"
done

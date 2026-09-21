#!/usr/bin/env bash

# Generate development-only ML-DSA PAA roots using an OpenSSL 3.5+ chip-cert build.
# Usage: ./credentials/development/gen-development-ml-dsa-paa-certs.sh /path/to/chip-cert
# Keys stay in attestation/; public certificate pairs go in paa-root-certs/.

set -euo pipefail
umask 077

# Select OpenSSL through pkg-config, then use that installation at runtime too.
# Override discovery with PKG_CONFIG_PATH=/path/to/openssl/lib/pkgconfig.
if [[ -z ${PKG_CONFIG_PATH:-} && -f /opt/lib64/pkgconfig/openssl.pc ]]; then
    export PKG_CONFIG_PATH=/opt/lib64/pkgconfig
fi
if ! pkg-config --atleast-version=3.5.0 openssl; then
    echo 'OpenSSL 3.5+ required; set PKG_CONFIG_PATH to its pkgconfig directory.' >&2
    exit 1
fi
openssl_libdir=$(pkg-config --variable=libdir libcrypto)
openssl_prefix=$(pkg-config --variable=prefix openssl)
[[ -d "$openssl_libdir" && -x "$openssl_prefix/bin/openssl" ]] || {
    echo 'The pkg-config OpenSSL installation must contain libraries and bin/openssl.' >&2
    exit 1
}
export LD_LIBRARY_PATH="$openssl_libdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export PATH="$openssl_prefix/bin:$PATH"

if [[ $# != 1 ]]; then
    echo "Usage: $0 /path/to/chip-cert" >&2
    exit 1
fi

chip_cert_tool=$1
here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
key_dir="$here/attestation"
cert_dir="$here/paa-root-certs"

if ! command -v "$chip_cert_tool" >/dev/null 2>&1; then
    echo "Cannot execute chip-cert: $chip_cert_tool" >&2
    exit 1
fi

# Check both algorithms before generating anything; never replace an existing root.
for variant in 44 65; do
    name="Chip-Development-PAA-ML-DSA-$variant"
    for output in "$key_dir/$name-Key.pem" "$key_dir/$name-Key.der" \
        "$cert_dir/$name-Cert.pem" "$cert_dir/$name-Cert.der"; do
        if [[ -e "$output" || -L "$output" ]]; then
            echo "Refusing to overwrite: $output" >&2
            exit 1
        fi
    done
done

mkdir -p "$key_dir" "$cert_dir"

for variant in 44 65; do
    name="Chip-Development-PAA-ML-DSA-$variant"
    key="$key_dir/$name-Key"
    cert="$cert_dir/$name-Cert"

    "$chip_cert_tool" gen-att-cert \
        --type a \
        --subject-cn "Matter Development PAA ML-DSA-$variant" \
        --subject-vid FFF1 \
        --key-type "ml-dsa-$variant" \
        --valid-from "2026-06-28 14:23:43" \
        --lifetime 4294967295 \
        --out-key "$key.pem" \
        --out "$cert.pem"

    "$chip_cert_tool" convert-key "$key.pem" "$key.der" --x509-der
    "$chip_cert_tool" convert-cert "$cert.pem" "$cert.der" --x509-der
    chmod 644 "$cert.pem" "$cert.der"
    echo "Generated $cert.pem and $cert.der"
done

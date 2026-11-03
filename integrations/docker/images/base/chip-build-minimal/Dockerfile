# This minimal build image is intentionally not based on chip-build
FROM ubuntu:24.04
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get upgrade -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    generate-ninja \
    git pkg-config \
    ninja-build \
    python3-venv \
    && git config --global advice.detachedHead false

# CHIP build dependencies
RUN set -x \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libssl-dev \
    libglib2.0-dev \
    wget

# Build and install OpenSSL 3.5 LTS (post-quantum ML-DSA / ML-KEM support)
# Build once per image; CI jobs reuse these static and shared libraries.
ARG OPENSSL_VERSION=3.5.0
ARG OPENSSL_SHA256=344d0a79f1a9b08029b0744e2cc401a43f9c90acd1044d09a530b4885a8e9fc0
RUN set -x \
    && cd /tmp \
    && wget -q https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/openssl-${OPENSSL_VERSION}.tar.gz \
    && echo "${OPENSSL_SHA256}  openssl-${OPENSSL_VERSION}.tar.gz" | sha256sum -c - \
    && tar xzf openssl-${OPENSSL_VERSION}.tar.gz \
    && cd openssl-${OPENSSL_VERSION} \
    && ./Configure --prefix=/opt/matter/openssl --openssldir=/etc/ssl --libdir=lib \
    && make -j$(nproc) \
    && make install_sw \
    && test "$(PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR=/opt/matter/openssl/lib/pkgconfig pkg-config --modversion openssl)" = "$OPENSSL_VERSION" \
    && test -f /opt/matter/openssl/lib/libssl.a \
    && test -f /opt/matter/openssl/lib/libcrypto.a \
    && test -f /opt/matter/openssl/lib/libssl.so \
    && test -f /opt/matter/openssl/lib/libcrypto.so \
    && cd / && rm -rf /tmp/openssl-${OPENSSL_VERSION}* \
    && : # last line

# Unset uses host OpenSSL. Explicit true/false selects private OpenSSL with
# static/dynamic linkage. Only the SDK's OpenSSL GN configuration consumes these.
ARG OPENSSL_STATIC=
RUN case "$OPENSSL_STATIC" in \
        ''|true|false) ;; \
        *) echo "OPENSSL_STATIC must be empty (host), true (static), or false (dynamic)" >&2; exit 1 ;; \
    esac
ENV CHIP_OPENSSL_ROOT=${OPENSSL_STATIC:+/opt/matter/openssl}
ENV CHIP_OPENSSL_STATIC=${OPENSSL_STATIC}

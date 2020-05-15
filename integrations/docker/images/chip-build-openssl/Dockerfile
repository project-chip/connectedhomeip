ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# Install specific release of openssl
RUN set -x \
    && cd /tmp && wget https://github.com/openssl/openssl/archive/OpenSSL_1_1_1f.zip \
    && mkdir -p /tmp/openssl && cd /tmp/openssl && unzip ../OpenSSL_1_1_1f.zip \
    && cd /tmp/openssl/openssl-OpenSSL_1_1_1f && ./config && make && make install \
    && rm -rf /tmp/OpenSSL_1_1_1f.zip \
    && : # last line

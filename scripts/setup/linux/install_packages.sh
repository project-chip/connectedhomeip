#!/bin/bash

set -x
apt-get update
apt-get install -fy \
    git \
    curl \
    jq \
    make \
    autoconf \
    automake \
    libtool \
    pkg-config \
    g++ \
    clang-9 \
    clang-format-9 \
    clang-tidy-9 \
    lcov \
    shellcheck \
    libssl-dev \
    unzip \
    wget \
    libmbedtls-dev

if [[ ! -f 'build/downloads/open_ssl_1.1.1f_installed' ]]; then
    mkdir -p build/downloads
    cd build/downloads
    wget https://github.com/openssl/openssl/archive/OpenSSL_1_1_1f.zip
    mkdir openssl
    cd openssl
    unzip ../OpenSSL_1_1_1f.zip
    cd openssl-OpenSSL_1_1_1f
    ./config
    make

    rm -rf ../OpenSSL_1_1_1f.zip

    cd ~/project
    touch build/downloads/open_ssl_1.1.1f_installed
    chown -R circleci:circleci build
fi

cd ~/project/build/downloads/openssl/openssl-OpenSSL_1_1_1f
make install_sw install_ssldirs

#!/usr/bin/env bash

#
#    Copyright (c) 2020 Project CHIP Authors
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

set -x
env

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
    libdbus-1-dev \
    libmbedtls-dev \
    python3.8-dev \
    python3.8-venv

if [[ ! -f 'ci-cache-persistent/openssl/open_ssl_1.1.1f_installed' ]]; then
    mkdir -p ci-cache-persistent/openssl
    cd ci-cache-persistent/openssl || exit
    wget https://github.com/openssl/openssl/archive/OpenSSL_1_1_1f.zip
    mkdir openssl
    cd openssl || exit
    unzip ../OpenSSL_1_1_1f.zip
    cd openssl-OpenSSL_1_1_1f || exit
    ./config
    make

    rm -rf ../OpenSSL_1_1_1f.zip

    cd ~/project || exit
    touch ci-cache-persistent/openssl/open_ssl_1.1.1f_installed
    chown -R circleci:circleci build
fi

cd ~/project/ci-cache-persistent/openssl/openssl/openssl-OpenSSL_1_1_1f || exit
make install_sw install_ssldirs

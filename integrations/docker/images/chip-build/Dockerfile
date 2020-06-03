# start with Ubuntu 18.04LTS
FROM ubuntu:bionic

VOLUME "/var/source"

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && apt-get install -fy \
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
    libmbedtls-dev \
    python-pip \
    python3 \
    python3-pip \
    libusb-1.0 \
    libncurses5-dev \
    libncursesw5-dev \
    flex \
    bison \
    gperf \
    zlib1g-dev \
    libglib2.0-dev \
    libpixman-1-dev \
    && rm -rf /var/lib/apt/lists/ \
    && pip3 install circleci attrs coloredlogs \
    && : # last line

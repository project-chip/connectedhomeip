# start with Ubuntu 18.04LTS
FROM ubuntu:bionic

VOLUME "/var/source"

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && apt-get install -fy \
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
    libcurl4-openssl-dev \
    libmbedtls-dev \
    python-pip \
    python3 \
    python3-pip \
    libdbus-1-dev \
    libusb-1.0 \
    libncurses5-dev \
    libncursesw5-dev \
    libnspr4-dev \
    libxml2-dev \
    gettext \
    flex \
    bison \
    gperf \
    zlib1g-dev \
    libglib2.0-dev \
    libpixman-1-dev \
    ninja-build \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

RUN set -x \
    && pip3 install circleci attrs coloredlogs PyGithub pygit \
    && : # last line

# Install newer cmake toolchain
RUN set -x \
    && wget https://cmake.org/files/v3.17/cmake-3.17.2.tar.gz \
    && tar xzf cmake-3.17.2.tar.gz \
    && cd cmake-3.17.2 \
    && ./bootstrap \
    && make \
    && make install \
    && cd .. \
    && rm -rf cmake-3.17.2 \
    && rm cmake-3.17.2.tar.gz \
    && : # last line

# Install newer git version, to allow for github workflow checkout
RUN set -x \
    && wget https://mirrors.edge.kernel.org/pub/software/scm/git/git-2.27.0.tar.gz \
    && tar xzf git-2.27.0.tar.gz \
    && cd git-2.27.0 \
    && ./configure --prefix=/usr \
    && make -j \
    && make install \
    && cd .. \
    && rm -rf git-2.27.0 \
    && rm git-2.27.0.tar.gz \
    && : # last line

# Install bloat comparison tools
RUN set -x \
    && git clone https://github.com/google/bloaty.git \
    && mkdir -p bloaty/build \
    && cd bloaty/build \
    && cmake ../ \
    && make -j \
    && make install \
    && cd ../.. \
    && rm -rf bloaty \
    && : # last line


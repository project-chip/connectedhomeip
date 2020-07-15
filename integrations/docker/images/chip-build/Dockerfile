# start with Ubuntu 20.04LTS
FROM ubuntu:focal

VOLUME "/var/source"

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    git \
    curl \
    jq \
    make \
    autoconf \
    automake \
    libtool \
    pkg-config \
    g++ \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    lcov \
    shellcheck \
    libssl-dev \
    unzip \
    wget \
    libmbedtls-dev \
    python3 \
    python3-pip \
    libusb-1.0-0 \
    libncurses5-dev \
    libncursesw5-dev \
    libnspr4-dev \
    libxml2-dev \
    flex \
    bison \
    gperf \
    zlib1g-dev \
    libglib2.0-dev \
    libpixman-1-dev \
    libdbus-1-dev \
    libdbus-glib-1-dev \
    libical-dev \
    libudev-dev \
    libusb-dev \
    libreadline-dev \
    systemd \
    udev \
    ninja-build \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Python 2 and PIP
RUN set -x \
   && apt-get update \
   && DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common \
   && add-apt-repository universe \
   && DEBIAN_FRONTEND=noninteractive apt-get install -y python python2 \
   && curl https://bootstrap.pypa.io/get-pip.py --output get-pip.py \
   && python2 get-pip.py \
   && rm -rf /var/lib/apt/lists/ \
   && : # last line

RUN set -x \
    && pip3 install circleci attrs coloredlogs PyGithub pygit \
    && : # last line

# build and install gn
RUN set -x \
    && git clone https://gn.googlesource.com/gn \
    && cd gn \
    && python3 build/gen.py \
    && ninja -C out \
    && cp out/gn /usr/local/bin \
    && cd .. \
    && rm -rf gn \
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

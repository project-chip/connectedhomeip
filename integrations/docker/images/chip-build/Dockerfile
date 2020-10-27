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
    bridge-utils \
    libtool \
    pkg-config \
    g++ \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    lcov \
    net-tools \
    shellcheck \
    libssl-dev \
    unzip \
    wget \
    iproute2 \
    libmbedtls-dev \
    openjdk-11-jdk \
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
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
    strace \
    libavahi-common-dev \
    libavahi-client-dev \
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
    && pip3 install circleci attrs coloredlogs PyGithub pygit future \
    portpicker mobly                                                 \
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
    && make -j8 \
    && make install \
    && cd ../.. \
    && rm -rf bloaty \
    && : # last line

# NodeJS: install a newer version than what apt-get would read
# This installs the latest LTS version of nodejs
RUN set -x \
    && mkdir node_js \
    && cd node_js \
    && wget https://nodejs.org/dist/v12.19.0/node-v12.19.0-linux-x64.tar.xz \
    && tar xfvJ node-v12.19.0-linux-x64.tar.xz \
    && mv node-v12.19.0-linux-x64 /opt/ \
    && ln -s /opt/node-v12.19.0-linux-x64 /opt/node \
    && ln -s /opt/node/bin/* /usr/bin \
    && cd .. \
    && rm -rf node_js \
    && : # last line

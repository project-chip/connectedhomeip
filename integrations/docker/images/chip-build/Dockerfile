# start with Ubuntu 20.04LTS
FROM ubuntu:focal

VOLUME "/var/source"

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy \
    autoconf \
    automake \
    bison \
    bridge-utils \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    curl \
    flex \
    g++ \
    git \
    gperf \
    iproute2 \
    jq \
    lcov \
    libavahi-client-dev \
    libavahi-common-dev \
    libcairo2-dev \
    libdbus-1-dev \
    libdbus-glib-1-dev \
    libgif-dev \
    libglib2.0-dev \
    libical-dev \
    libjpeg-dev \
    libmbedtls-dev \
    libncurses5-dev \
    libncursesw5-dev \
    libnspr4-dev \
    libpango1.0-dev \
    libpixman-1-dev \
    libreadline-dev \
    libssl-dev \
    libtool \
    libudev-dev \
    libusb-1.0-0 \
    libusb-dev \
    libxml2-dev \
    make \
    net-tools \
    ninja-build \
    openjdk-8-jdk \
    pkg-config \
    python3 \
    python3-dev \
    python3-pip \
    python3-venv \
    shellcheck \
    strace \
    systemd \
    udev \
    unzip \
    wget \
    zlib1g-dev \
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

# start with Ubuntu 20.04LTS
FROM ubuntu:focal

VOLUME "/var/source"

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --no-install-recommends \
    autoconf=2.69-11.1 \
    automake=1:1.16.1-4ubuntu6 \
    bison=2:3.5.1+dfsg-1 \
    bridge-utils=1.6-2ubuntu1 \
    clang=1:10.0-50~exp1 \
    clang-format=1:10.0-50~exp1 \
    clang-tidy=1:10.0-50~exp1 \
    curl=7.68.0-1ubuntu2.5 \
    flex=2.6.4-6.2 \
    g++=4:9.3.0-1ubuntu2 \
    git=1:2.25.1-1ubuntu3.1 \
    gperf=3.1-1build1 \
    iproute2=5.5.0-1ubuntu1 \
    jq=1.6-1ubuntu0.20.04.1 \
    lcov=1.14-2 \
    libavahi-client-dev=0.7-4ubuntu7 \
    libavahi-common-dev=0.7-4ubuntu7 \
    libcairo2-dev=1.16.0-4ubuntu1 \
    libdbus-1-dev=1.12.16-2ubuntu2.1 \
    libdbus-glib-1-dev=0.110-5fakssync1 \
    libgif-dev=5.1.9-1 \
    libglib2.0-dev=2.64.6-1~ubuntu20.04.3 \
    libical-dev=3.0.8-1 \
    libjpeg-dev=8c-2ubuntu8 \
    libdmalloc-dev=5.5.2-14build1 \
    libmbedtls-dev=2.16.4-1ubuntu2 \
    libncurses5-dev=6.2-0ubuntu2 \
    libncursesw5-dev=6.2-0ubuntu2 \
    libnspr4-dev=2:4.25-1 \
    libpango1.0-dev=1.44.7-2ubuntu4 \
    libpixman-1-dev=0.38.4-0ubuntu1 \
    libreadline-dev=8.0-4 \
    libssl-dev=1.1.1f-1ubuntu2.3 \
    libtool=2.4.6-14 \
    libudev-dev=245.4-4ubuntu3.6 \
    libusb-1.0-0=2:1.0.23-2build1 \
    libusb-dev=2:0.1.12-32 \
    libxml2-dev=2.9.10+dfsg-5 \
    make=4.2.1-1.2 \
    net-tools=1.60+git20180626.aebd88e-1ubuntu1 \
    ninja-build=1.10.0-1build1 \
    openjdk-8-jdk=8u292-b10-0ubuntu1~20.04 \
    pkg-config=0.29.1-0ubuntu4 \
    python3=3.8.2-0ubuntu2 \
    python3-dev=3.8.2-0ubuntu2 \
    python3-pip=20.0.2-5ubuntu1.3 \
    python3-venv=3.8.2-0ubuntu2 \
    shellcheck=0.7.0-2build2 \
    strace=5.5-3ubuntu1 \
    systemd=245.4-4ubuntu3.6 \
    udev=245.4-4ubuntu3.6 \
    unzip=6.0-25ubuntu1 \
    wget=1.20.3-1ubuntu1 \
    xz-utils=5.2.4-1ubuntu1 \
    zlib1g-dev=1:1.2.11.dfsg-2ubuntu1.2 \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Cmake (Mbed OS requires >=3.19.0-rc3 version which is not available in Ubuntu 20.04 repository)
RUN set -x \
    && (cd /tmp \
    && wget --progress=dot:giga https://github.com/Kitware/CMake/releases/download/v3.19.3/cmake-3.19.3-Linux-x86_64.sh \
    && sh cmake-3.19.3-Linux-x86_64.sh --exclude-subdir --prefix=/usr/local \
    && rm -rf cmake-3.19.3-Linux-x86_64.sh) \
    && exec bash \
    && : # last line

# Python 2 and PIP
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends software-properties-common=0.98.9.4 \
    && add-apt-repository universe \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends python2=2.7.17-2ubuntu4 \
    && curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py \
    && python2 get-pip.py \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

RUN set -x \
    && pip3 install --no-cache-dir \
    circleci==1.2.2 \
    attrs==20.3.0 \
    coloredlogs==15.0 \
    PyGithub==1.55 \
    pygit==0.1 \
    future==0.18.2 \
    portpicker==1.3.1 \
    mobly==1.10.1 \
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
    && git clone --depth=1 https://github.com/google/bloaty.git \
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
    && curl -o node.tgz -s https://nodejs.org/dist/v12.19.0/node-v12.19.0-linux-x64.tar.xz \
    && mkdir -p /opt/node \
    && tar xfJ node.tgz --strip-components=1 -C /opt/node \
    && ln -s /opt/node/bin/* /usr/bin \
    && rm -f node.tgz \
    && : # last line

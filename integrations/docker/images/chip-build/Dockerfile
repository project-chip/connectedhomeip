# start with Ubuntu 20.04LTS
FROM ubuntu:focal

VOLUME "/var/source"

# base build and check tools and libraries layer
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --fix-missing \
    autoconf \
    automake \
    bison \
    bridge-utils \
    ccache \
    clang \
    clang-format \
    clang-tidy \
    curl \
    flex \
    g++ \
    git \
    git-lfs \
    gperf \
    iproute2 \
    jq \
    lcov \
    libavahi-client-dev \
    libavahi-common-dev \
    libcairo-dev \
    libcairo2-dev \
    libdbus-1-dev \
    libdbus-glib-1-dev \
    libdmalloc-dev \
    libgif-dev \
    libglib2.0-dev \
    libical-dev \
    libjpeg-dev \
    libmbedtls-dev \
    libncurses5-dev \
    libncursesw5-dev \
    libnl-3-dev \
    libnl-route-3-dev \
    libnspr4-dev \
    libpango1.0-dev \
    libpixman-1-dev \
    libreadline-dev \
    libsdl-pango-dev \
    libssl-dev \
    libtool \
    libudev-dev \
    libusb-1.0-0 \
    libusb-dev \
    libxml2-dev \
    make \
    meson \
    net-tools \
    ninja-build \
    openjdk-8-jdk \
    pkg-config \
    python-is-python3 \
    python3.9 \
    python3.9-dev \
    python3.9-venv \
    rsync \
    shellcheck \
    strace \
    systemd \
    udev \
    unzip \
    wget \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/ \
    && git lfs install \
    && : # last line

# Cmake v3.23.1
RUN set -x \
    && (cd /tmp \
    && wget --progress=dot:giga https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1-Linux-x86_64.sh \
    && sh cmake-3.23.1-Linux-x86_64.sh --exclude-subdir --prefix=/usr/local \
    && rm -rf cmake-3.23.1-Linux-x86_64.sh) \
    && exec bash \
    && : # last line

# Python 3.9 and PIP
RUN set -x \
    && DEBIAN_FRONTEND=noninteractive  apt-get update \
    && DEBIAN_FRONTEND=noninteractive  apt-get install -y libgirepository1.0-dev \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y software-properties-common \
    && add-apt-repository universe \
    && curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py \
    && python3.9 get-pip.py \
    && update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 1 \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

RUN set -x \
    && pip3 install \
    attrs \
    click \
    coloredlogs \
    cxxfilt \
    flake8 \
    future \
    ghapi \
    mobly \
    pandas \
    portpicker \
    pygit \
    PyGithub \
    tabulate \
    # Cleanup
    && pip3 cache purge \
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

# Need newer version of include-what-you-use
RUN set -x \
    && apt-get update \
    # Install build and runtime requirements for IWYU
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --fix-missing clang-12 libclang-12-dev \
    # Build and install IWYU
    && git clone --depth=1 --branch=clang_12 https://github.com/include-what-you-use/include-what-you-use.git \
    && mkdir -p include-what-you-use/build \
    && cd include-what-you-use/build \
    && cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_PREFIX_PATH=/usr/lib/llvm-12 -DIWYU_LINK_CLANG_DYLIB=OFF .. \
    && make -j8 \
    && make install \
    # Save clang-12 files, so we can restore them after build dependencies cleanup
    && tar -cf clang-12-files.tar $(dpkg -L libclang-common-12-dev |grep /include) /usr/lib/llvm-12/lib/libLLVM-12.so.1 \
    # Cleanup build dependencies
    && apt autoremove -fy clang-12 libclang-12-dev \
    && rm -rf /var/lib/apt/lists/ \
    # Restore clang-12 files
    && tar -xf clang-12-files.tar -C / \
    # Cleanup
    && cd ../.. \
    && rm -rf include-what-you-use \
    && : # last line

# Build glib-2.0 from source with enabled thread sanitizer. This is needed for
# running CHIP tests with TSAN enabled. When running applications with TSAN
# all shared libraries should be built with TSAN enabled, otherwise TSAN might
# report false positives. This case is most prominent with glib-2.0, which has
# a lot of threads-related APIs.
ENV LD_LIBRARY_PATH_TSAN=/usr/lib/x86_64-linux-gnu-tsan
RUN set -x \
    && mkdir -p $LD_LIBRARY_PATH_TSAN \
    && GLIB_VERSION=$(pkg-config --modversion glib-2.0) \
    && git clone --depth=1 --branch=$GLIB_VERSION https://github.com/GNOME/glib.git \
    && CFLAGS="-O2 -g -fsanitize=thread" meson glib/build glib \
    && DESTDIR=../build-image ninja -C glib/build install \
    && mv glib/build-image/usr/local/lib/x86_64-linux-gnu/lib* $LD_LIBRARY_PATH_TSAN \
    && rm -rf glib \
    && : # last line

# Install a known ZAP release
# Only keep the cli version, since `zap` is 143MB and not usable (UI)
#
# NOTE: zap version occurs in several files (CI and docker)
#       for a uniform update, a separate update script exists:
#
#       scripts/tools/zap/version_update.py
ENV ZAP_VERSION=v2023.01.18-nightly
RUN set -x \
    && mkdir -p /opt/zap-${ZAP_VERSION} \
    && cd /opt/zap-${ZAP_VERSION} \
    && wget https://github.com/project-chip/zap/releases/download/${ZAP_VERSION}/zap-linux.zip \
    && unzip zap-linux.zip \
    && rm zap-linux.zip \
    && rm zap \
    && ln -s /opt/zap-${ZAP_VERSION}/zap-cli /usr/bin/ \
    && : # last line

# NodeJS: install a newer version than what apt-get would read
# This installs the latest LTS version of nodejs
#
# NodeJS is required by github actions, we use Wandalen/wretry.action@v1.0.36
# and that seems to use the built-in node installation in the image
#
# This is not a CHIP dependency directly, but used by CI
ENV CHIP_NODE_VERSION=v16.13.2
RUN set -x \
    && mkdir node_js \
    && cd node_js \
    && wget https://nodejs.org/dist/$CHIP_NODE_VERSION/node-$CHIP_NODE_VERSION-linux-x64.tar.xz \
    && tar xfvJ node-$CHIP_NODE_VERSION-linux-x64.tar.xz \
    && mv node-$CHIP_NODE_VERSION-linux-x64 /opt/ \
    && ln -s /opt/node-$CHIP_NODE_VERSION-linux-x64 /opt/node \
    && ln -s /opt/node/bin/* /usr/bin \
    && cd .. \
    && rm -rf node_js \
    && : # last line

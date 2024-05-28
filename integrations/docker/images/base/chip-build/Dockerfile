FROM ubuntu:24.04
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

VOLUME "/var/source"

ARG TARGETPLATFORM

# Ensure TARGETPLATFORM is set
RUN case ${TARGETPLATFORM} in \
    "linux/amd64") \
    echo "Building for linux/amd64" \
    ;; \
    "linux/arm64") \
    echo "Building for linux/arm64" \
    ;; \
    *) \
    if [ -z "$TARGETPLATFORM" ] ;\
    then \
    echo "TARGETPLATFORM not defined! Please run from buildkit (buildx)." \
    && return 1 ;\
    else \
    echo "Unsupported platform ${TARGETPLATFORM}." \
    && return 1 ;\
    fi \
    ;; \
    esac


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
    cmake \
    curl \
    flex \
    g++-10 \
    generate-ninja \
    git \
    git-lfs \
    gperf \
    iproute2 \
    jq \
    lcov \
    libavahi-client-dev \
    libavahi-common-dev \
    libcairo2-dev \
    libcairo-dev \
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
    libsdl2-dev \
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
    nodejs \
    pkg-config \
    python3 \
    python3-click \
    python3-coloredlogs \
    python3-dev \
    python3-future \
    python3-pandas \
    python3-pip \
    python3-tabulate \
    python3-venv \
    python-is-python3 \
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

# Set gcc 10 as a default compiler to work with TSAN
RUN set -x \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 10 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 10 \
    && update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 30 \
    && update-alternatives --set cc /usr/bin/gcc \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 30 \
    && update-alternatives --set c++ /usr/bin/g++ \
    && : # last line

# NOTE: this install is NOT ideal as we are installing over system
#       we should migrate to venvs for packages we do not directly have
#       provided by the system
RUN set -x \
    && pip3 install --break-system-packages --no-cache-dir --break-system-packages \
    attrs \
    cxxfilt \
    ghapi \
    mobly \
    portpicker \
    pygit \
    PyGithub \
    ruff \
    && : # last line

# Install bloat comparison tools
RUN set -x \
    && git clone https://github.com/google/bloaty.git \
    && mkdir -p bloaty/build \
    && cd bloaty/build \
    && cmake -DCMAKE_BUILD_TYPE=MinSizeRel ../ \
    && make -j8 \
    && strip bloaty \
    && make install \
    && cd ../.. \
    && rm -rf bloaty \
    && : # last line

# Need newer version of include-what-you-use
RUN set -x \
    && apt-get update \
    # Install build and runtime requirements for IWYU
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --fix-missing clang-18 libclang-18-dev \
    # Build and install IWYU
    && git clone --depth=1 --branch=clang_18 https://github.com/include-what-you-use/include-what-you-use.git \
    && mkdir -p include-what-you-use/build \
    && cd include-what-you-use/build \
    && cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_PREFIX_PATH=/usr/lib/llvm-18 -DIWYU_LINK_CLANG_DYLIB=OFF .. \
    && make -j8 \
    && strip bin/include-what-you-use \
    && make install \
    # Save clang-18 files, so we can restore them after build dependencies cleanup
    # AWK logic converts 'libLLVM.so.18.1 => /usr/lib/llvm-18/lib/libLLVM.so.18.1 (0x00007f781ede6000)' to '/usr/lib/llvm-18/lib/libLLVM.so.18.1'
    && tar -cf clang-18-files.tar $(dpkg -L libclang-common-18-dev |grep /include) $(ldd bin/include-what-you-use | grep /usr/lib/llvm | awk '{print $3}') /usr/lib/llvm-18/lib/libLLVM.so.1 \
    # Cleanup build dependencies
    && apt autopurge -fy clang-18 libclang-18-dev \
    && rm -rf /var/lib/apt/lists/ \
    # Restore clang-18 files
    && tar -xf clang-18-files.tar -C / \
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
RUN case ${TARGETPLATFORM} in \
    "linux/amd64") \
    set -x \
    && mkdir -p $LD_LIBRARY_PATH_TSAN \
    && export CCACHE_DISABLE=1 PYTHONDONTWRITEBYTECODE=1 \
    && GLIB_VERSION=$(pkg-config --modversion glib-2.0) \
    && git clone --depth=1 --branch=$GLIB_VERSION https://github.com/GNOME/glib.git \
    && CFLAGS="-O2 -g -fsanitize=thread" meson glib/build glib -Dtests=false \
    && DESTDIR=../build-image ninja -C glib/build install \
    && mv glib/build-image/usr/local/lib/x86_64-linux-gnu/lib* $LD_LIBRARY_PATH_TSAN \
    && rm -rf glib \
    ;; \
    "linux/arm64") \
    echo "ARM64 unsupported with TSAN" \
    ;; \
    *) \
    echo "Unsupported platform ${TARGETPLATFORM}" \
    && return 1 ;\
    ;; \
    esac

# Some things that save space
# Protoc goes from 108M to 4.6M
RUN strip /usr/local/bin/protoc*

# Cleanup unused bits
RUN set -x \
    && DEBIAN_FRONTEND=noninteractive apt-get remove -fy --fix-missing \
    python3-scipy \
    && DEBIAN_FRONTEND=noninteractive apt-get autoremove -fy --fix-missing \
    && : # last line

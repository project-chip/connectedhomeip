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
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --fix-missing --no-install-recommends \
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
    dbus-daemon \
    ffmpeg \
    flex \
    g++ \
    generate-ninja \
    git \
    git-lfs \
    gperf \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-tools \
    iproute2 \
    iwyu \
    jq \
    lcov \
    libavahi-client-dev \
    libavahi-common-dev \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev \
    libcairo2-dev \
    libcairo-dev \
    libcurl4-openssl-dev \
    libdbus-1-dev \
    libdbus-glib-1-dev \
    libdmalloc-dev \
    libevent-dev \
    libgif-dev \
    libgirepository-1.0-1 \
    libglib2.0-dev \
    libgstreamer1.0-0 \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libical-dev \
    libjpeg-dev \
    libmbedtls-dev \
    libncurses5-dev \
    libncursesw5-dev \
    libnl-3-dev \
    libnl-route-3-dev \
    libnspr4-dev \
    libpango1.0-dev \
    libpcsclite-dev \
    libpixman-1-dev \
    libprotobuf-dev \
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
    patch \
    pkg-config \
    protobuf-compiler \
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
    tcpdump \
    udev \
    unzip \
    wget \
    zlib1g-dev \
    zstd \
    && rm -rf /var/lib/apt/lists/ \
    && git lfs install \
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
    gcovr==8.3 \
    && : # last line

# Install bloat comparison tools
RUN set -x \
    && git clone https://github.com/google/bloaty.git \
    && mkdir -p bloaty/build \
    && cd bloaty/build \
    && cmake -DCMAKE_BUILD_TYPE=MinSizeRel ../ \
    && make -j$(nproc) \
    && strip bloaty \
    && make install \
    && cd ../.. \
    && rm -rf bloaty \
    && : # last line

# Code built with thread sanitizer can sometimes fail to run with an error message
# similar to the following:
# FATAL: ThreadSanitizer: unexpected memory mapping 0x7763c5472000-0x7763c5900000
#
# This happens for example when the build is executed on a Ubuntu 24.04 host. It
# is reported to be linked with kernel versions 6.6.6 and higher. In order for the
# build to succeed ASLR entropy needs to be reduced. This can be done with the
# sysctl command:
# sudo sysctl vm.mmap_rnd_bits=28
#
# This command is not persistent across reboots. This is desired as reducing
# ASLR entropy has negative impact on exploit mitigation.
# Reference: https://github.com/google/sanitizers/issues/1716

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
    && (CFLAGS="-O2 -g -fsanitize=thread" meson glib/build glib -Dtests=false || \
    { cat glib/build/meson-logs/meson-log.txt; exit 1; }) \
    && DESTDIR=../build-image ninja -C glib/build install \
    && mv glib/build-image/usr/local/lib64/lib* $LD_LIBRARY_PATH_TSAN \
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

# Sanitizer compilation fails if this is installed before
# glib recompile.
RUN set -x \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -fy --fix-missing --no-install-recommends \
    libgirepository1.0-dev \
    && rm -rf /var/lib/apt/lists/ \
    && : # last line

# Install OpenSSL 3.5 privately for SDK targets (including ML-DSA / ML-KEM).
# Keep system headers, pkg-config metadata, CLI and loader defaults unchanged
# for tools built in this image or later stages such as chip-build-cirque.
# Build once per image; CI jobs reuse these static and shared libraries.
ARG OPENSSL_VERSION=3.5.0
ARG OPENSSL_SHA256=344d0a79f1a9b08029b0744e2cc401a43f9c90acd1044d09a530b4885a8e9fc0
RUN set -x \
    && cd /tmp \
    && wget -q https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VERSION}/openssl-${OPENSSL_VERSION}.tar.gz \
    && echo "${OPENSSL_SHA256}  openssl-${OPENSSL_VERSION}.tar.gz" | sha256sum -c - \
    && tar xzf openssl-${OPENSSL_VERSION}.tar.gz \
    && cd openssl-${OPENSSL_VERSION} \
    && ./Configure --prefix=/opt/matter/openssl --openssldir=/etc/ssl --libdir=lib \
    && make -j$(nproc) \
    && make install_sw \
    && test "$(PKG_CONFIG_PATH= PKG_CONFIG_LIBDIR=/opt/matter/openssl/lib/pkgconfig pkg-config --modversion openssl)" = "$OPENSSL_VERSION" \
    && test -f /opt/matter/openssl/lib/libssl.a \
    && test -f /opt/matter/openssl/lib/libcrypto.a \
    && test -f /opt/matter/openssl/lib/libssl.so \
    && test -f /opt/matter/openssl/lib/libcrypto.so \
    && cd / && rm -rf /tmp/openssl-${OPENSSL_VERSION}* \
    && : # last line

# Unset uses host OpenSSL. Explicit true/false selects private OpenSSL with
# static/dynamic linkage. Only the SDK's OpenSSL GN configuration consumes these.
ARG OPENSSL_STATIC=
RUN case "$OPENSSL_STATIC" in \
        ''|true|false) ;; \
        *) echo "OPENSSL_STATIC must be empty (host), true (static), or false (dynamic)" >&2; exit 1 ;; \
    esac
ENV CHIP_OPENSSL_ROOT=${OPENSSL_STATIC:+/opt/matter/openssl}
ENV CHIP_OPENSSL_STATIC=${OPENSSL_STATIC}

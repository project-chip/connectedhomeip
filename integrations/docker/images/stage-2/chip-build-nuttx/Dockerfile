ARG VERSION=1
FROM ghcr.io/project-chip/chip-build:${VERSION}
LABEL org.opencontainers.image.source https://github.com/project-chip/connectedhomeip

# ------------------------------------------------------------------------------
# Install prerequisites
RUN set -x \
    && dpkg --add-architecture i386 \
    && apt update -y \
    && apt install -y genromfs xxd libgmp-dev libmpfr-dev libmpc-dev \
    && : # last line

# ------------------------------------------------------------------------------
# Download and build g++-13
RUN set -x \
    && ! test -d /opt/nuttx/gcc-13 \
    && wget -P gcc_build https://mirrorservice.org/sites/sourceware.org/pub/gcc/releases/gcc-13.1.0/gcc-13.1.0.tar.gz \
    && cd gcc_build \
    && tar xzf gcc-13.1.0.tar.gz \
    && cd gcc-13.1.0 \
    && ./configure --prefix=/opt/nuttx/gcc-13 --disable-multilib \
    && make -j$(nproc) \
    && make install-strip \
    && rm -rf ../../gcc_build \
    && : # last line

    RUN set -x \
    && ! test -f /opt/nuttx/gcc-13/bin/cc \
    && ln -s /opt/nuttx/gcc-13/bin/gcc /opt/nuttx/gcc-13/bin/cc \
    && : # last line

# ------------------------------------------------------------------------------
# Download NuttX SDK
RUN set -x \
    && cd /opt/nuttx \
    && rm -rf nuttx \
    && wget -q -O nuttx.zip https://codeload.github.com/apache/nuttx/zip/6e941aed \
    && unzip -q nuttx.zip \
    && mv nuttx-6e941aed nuttx \
    && rm nuttx.zip \
    && : # last line

RUN set -x \
    && cd /opt/nuttx \
    && rm -rf apps \
    && wget -q -O nuttx_app.zip https://codeload.github.com/apache/nuttx-apps/zip/c6051e4c \
    && unzip -q nuttx_app.zip \
    && mv nuttx-apps-c6051e4c apps \
    && rm nuttx_app.zip \
    && : # last line

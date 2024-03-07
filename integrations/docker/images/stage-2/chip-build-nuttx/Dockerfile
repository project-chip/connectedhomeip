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
    && wget -P gcc_build https://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-13.1.0/gcc-13.1.0.tar.gz \
    && cd gcc_build \
    && tar xzf gcc-13.1.0.tar.gz \
    && cd gcc-13.1.0 \
    && ./configure --prefix=/opt/nuttx/gcc-13 --disable-multilib \
    && make -j8 \
    && make install-strip \
    && rm -rf ../../gcc_build \
    && : # last line

# ------------------------------------------------------------------------------
# Download NuttX SDK
RUN set -x \
    && cd /opt/nuttx \
    && wget -q -O nuttx.zip https://codeload.github.com/apache/nuttx/zip/b8e0423 \
    && unzip -q nuttx.zip \
    && mv nuttx-b8e0423 nuttx \
    && rm nuttx.zip

RUN set -x \
    && cd /opt/nuttx \
    && wget -q -O nuttx_app.zip https://codeload.github.com/apache/nuttx-apps/zip/9c48a8d \
    && unzip -q nuttx_app.zip \
    && mv nuttx-apps-9c48a8d apps \
    && rm nuttx_app.zip

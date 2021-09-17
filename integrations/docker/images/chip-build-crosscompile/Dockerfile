ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

COPY ubuntu-21.04-aarch64-sysroot.tar.xz /opt/ubuntu-21.04-aarch64-sysroot.tar.xz

RUN set -x \
    && cd /opt \
    && tar xfvJ ubuntu-21.04-aarch64-sysroot.tar.xz \
    && rm ubuntu-21.04-aarch64-sysroot.tar.xz \
    && : # last line


ENV SYSROOT_AARCH64=/opt/ubuntu-21.04-aarch64-sysroot


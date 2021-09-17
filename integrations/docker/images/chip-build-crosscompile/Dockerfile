ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

COPY ubuntu-21.04-aarch64-sysroot.tar.xz /opt/ubuntu-21.04-aarch64-sysroot.tar.xz

# Unpack the sysroot, while also removing some rather large items in it that
# are generally not required for compilation
RUN set -x \
    && cd /opt \
    && tar xfvJ ubuntu-21.04-aarch64-sysroot.tar.xz \
    && rm ubuntu-21.04-aarch64-sysroot.tar.xz \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/usr/lib/firmware \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/usr/lib/git-core \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/usr/lib/modules \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/lib/firmware \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/lib/git-core \
    && rm -rf /opt/ubuntu-21.04-aarch64-sysroot/lib/modules \
    && : # last line


ENV SYSROOT_AARCH64=/opt/ubuntu-21.04-aarch64-sysroot

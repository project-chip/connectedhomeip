ARG VERSION=latest
FROM connectedhomeip/chip-build:${VERSION}

# ==================================================
# Needed tooling
# ==================================================

# GNU ARM Embedded toolchain, cross compiler for various platform builds
RUN set -x \
    && (mkdir -p /opt/ARM-software && cd /opt/ARM-software \
    && curl https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 \
    | tar jxvf -) \
    && : # last line

ENV ARM_GCC_INSTALL_ROOT=/opt/ARM-software/gcc-arm-none-eabi-9-2019-q4-major/bin/
ENV PATH=$PATH:$ARM_GCC_INSTALL_ROOT

# ==================================================
# SDK files - submodule under /third_party/qpg_sdk
# ==================================================

# Nothing to do currently
